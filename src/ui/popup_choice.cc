#include "popup_choice.hh"
#include "root.hh"

#include <algorithm>

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

namespace {

auto contains(const Rectangle &area, const Vector &position) -> bool {
  return bool{area & Rectangle{position, position + Vector{1, 1}}};
}

} // namespace

PopupChoice::PopupChoice(Container &parent, const Rectangle &area, const Char &background):
Widget{area, background, parent}
{
  onKey([this](const ui::Element &, const Event &event) {return handleKey(event);});
  onMouseButton([this](const ui::Element &, const Event &event) {return handleMouseButton(event);});
  onMouseMove([this](const ui::Element &, const Event &event) {return handleMouseMove(event);});
}

PopupChoice::PopupChoice(Container &parent, const Char &background):
PopupChoice{parent, RectangleDefault, background}
{}

void PopupChoice::caption(const Text &caption) {
  caption_ = caption;
  render();
  layoutChanged();
}

void PopupChoice::open() {
  if (popup_) {
    return;
  }
  auto source{popupItems()};
  const auto count{source.size()};
  if (count <= 0) {
    return;
  }
  const auto self{bounds()};
  auto width{self.width()};
  const auto &glyphs{uiStyle().popup};
  const auto padding{toDim(Text{glyphs.left}.width() + Text{glyphs.right}.width())};
  const auto scan{std::min(static_cast<std::size_t>(count), scanLimit_)};
  for (std::size_t position = 0; position < scan; ++position) {
    width = std::max(width, toDim(source[static_cast<ListIndex>(position)].width() + padding));
  }
  const auto rows{count > static_cast<ListIndex>(maxRows_)? maxRows_: static_cast<Dim>(count)};
  auto *root{this->root()};
  if (uiStyle().popupPlacement == PopupPlacement::inside) {
    // The popup is a widget inside the same window, clipped to the content area; it flips above the control when
    // there is not enough room below
    const auto content{root->content()};
    const auto below{toDim(content.y2() - self.y2())}; // rows available below the control
    const auto above{toDim(self.y1() - content.y1())}; // rows available above the control
    const auto popupRows{std::min(rows, std::max(below, above))};
    const auto flip{below < popupRows and above > below};
    const auto py{flip? toDim(self.y1() - popupRows): self.y2()};
    popupArea_ = Rectangle{self.x1(), py, toDim(self.x1() + width), toDim(py + popupRows)};
    popup_.emplace(
      *root,
      popupArea_,
      std::move(source),
      std::max(popupSelected(), ListIndex{0}),
      *this
    );
  } else {
    // The popup opens in a transient window of its own, above the owner window
    auto &terminal{*root->host().terminal()};
    const auto ownerDisplay{root->displayOrigin()};
    const auto x{toDim(ownerDisplay.x() + self.x1())};
    const auto y{toDim(ownerDisplay.y() + self.y2())};
    popupArea_ = Rectangle{Vector{self.x1(), self.y2()}, Vector{toDim(self.x1() + width), toDim(self.y2() + rows)}};
    popupWindow_.emplace(terminal, Rectangle{Vector{x, y}, Vector{toDim(x + width), toDim(y + rows)}}, background(), root->host(),
                         &root->colors(), &root->style());
    popup_.emplace(
      *popupWindow_,
      Rectangle{Vector{0, 0}, Vector{width, rows}},
      std::move(source),
      std::max(popupSelected(), ListIndex{0}),
      *this
    );
  }
  popup_->itemEnabled([this](ListIndex index) {return popupItemEnabled(index);});
  if (not popupItemEnabled(popup_->highlighted())) {
    if (const auto first{firstEnabled()}; first >= 0) {
      popup_->highlight(first);
    }
  }
  render();
}

void PopupChoice::close() {
  const auto wasOpen{popup_.has_value()};
  popup_.reset();       // the popup widget is gone while the window it is in is still there
  popupWindow_.reset(); // the window takes its root with it, after all of its widgets are gone
  dragging_ = false;
  if (wasOpen) {
    render();
  }
}

void PopupChoice::maxRows(Dim rows) {
  maxRows_ = std::max(Dim{1}, rows);
}

void PopupChoice::move(const Rectangle &area) {
  close();
  Widget::move(area);
}

void PopupChoice::focusChanged(bool focused) {
  if (not focused) {
    close();
  }
  Widget::focusChanged(focused);
}

void PopupChoice::pressedElsewhere() {
  close();
}

auto PopupChoice::preferredSize() -> Vector {
  const auto &glyphs{uiStyle().dropDown[static_cast<size_t>(WidgetState::normal)]};
  const auto padding{toDim(Text{glyphs.left}.width() + Text{glyphs.right}.width())};
  auto width{caption_.width()};
  auto source{popupItems()};
  const auto count{source.size()};
  const auto scan{std::min(static_cast<std::size_t>(std::max(count, ListIndex{0})), scanLimit_)};
  for (std::size_t position = 0; position < scan; ++position) {
    width = std::max(width, source[static_cast<ListIndex>(position)].width());
  }
  return Vector{toDim(width + padding), 1};
}

auto PopupChoice::firstEnabled() const -> ListIndex {
  for (ListIndex index{0}; index < popup_->size(); ++index) {
    if (popupItemEnabled(index)) {
      return index;
    }
  }
  return static_cast<ListIndex>(-1);
}

auto PopupChoice::lastEnabled() const -> ListIndex {
  for (auto index{static_cast<ListIndex>(popup_->size() - 1)};; --index) {
    if (popupItemEnabled(index)) {
      return index;
    }
    if (index == 0) {
      break;
    }
  }
  return static_cast<ListIndex>(-1);
}

auto PopupChoice::step(ListIndex from, i8 direction) const -> ListIndex {
  auto index{from};
  for (;;) {
    const auto candidate{static_cast<ListIndex>(index + direction)};
    if (candidate < 0 or candidate >= popup_->size()) {
      return from;
    }
    index = candidate;
    if (popupItemEnabled(index)) {
      return index;
    }
  }
}

auto PopupChoice::near(ListIndex target, i8 direction) const -> ListIndex {
  target = std::clamp(target, ListIndex{0}, static_cast<ListIndex>(popup_->size() - 1));
  if (popupItemEnabled(target)) {
    return target;
  }
  auto index{target};
  for (;;) {
    const auto candidate{static_cast<ListIndex>(index + direction)};
    if (candidate < 0 or candidate >= popup_->size()) {
      return static_cast<ListIndex>(-1);
    }
    index = candidate;
    if (popupItemEnabled(index)) {
      return index;
    }
  }
}

auto PopupChoice::handleKey(const Event &event) -> bool {
  const auto key{event.as<KeyEvent>().key()};
  if (not popup_) {
    if (key == ' ' or key == Key::Enter or key == Key::Down) {
      open();
      return true;
    }
    return false;
  }
  const auto current{popup_->highlighted()};
  switch (key) {
    case Key::Up:
    popup_->highlight(step(current, -1));
    break;

    case Key::Down:
    popup_->highlight(step(current, 1));
    break;

    case Key::Home:
    if (const auto first{firstEnabled()}; first >= 0) {
      popup_->highlight(first);
    }
    break;

    case Key::End:
    if (const auto last{lastEnabled()}; last >= 0) {
      popup_->highlight(last);
    }
    break;

    case Key::PageUp:
    if (const auto target{near(static_cast<ListIndex>(current - popup_->rows()), -1)}; target >= 0) {
      popup_->highlight(target);
    }
    break;

    case Key::PageDown:
    if (const auto target{near(static_cast<ListIndex>(current + popup_->rows()), 1)}; target >= 0) {
      popup_->highlight(target);
    }
    break;

    case Key::Enter:
    case ' ':
    popupActivate(current, event);
    return true;

    case Key::Escape:
    close();
    break;

    default:
    return false;
  }
  return true;
}

auto PopupChoice::handleMouseButton(const Event &event) -> bool {
  const auto &mouse{event.as<MouseButtonEvent>()};
  const auto position{mouse.position()};
  if (mouse.modifiers().mod5) { // the wheel: button 1 is up, button 2 is down
    if (popup_ and mouse.action() == MouseAction::Down) {
      popup_->highlight(step(popup_->highlighted(), mouse.button() == MouseButton::Button1? -1: 1));
      return true;
    }
    return false;
  }
  if (mouse.button() != MouseButton::Button1) {
    return false;
  }
  const auto inside{contains(bounds(), position)};
  if (mouse.action() == MouseAction::Down) {
    if (popup_) {
      if (const auto item{popup_->itemAt(popupPosition(position))}; item >= 0) {
        if (popupItemEnabled(item)) {
          popupActivate(item, event);
        }
        return true;
      }
      if (inside) {
        close();
        return true;
      }
      close();
      return false;
    }
    if (inside) {
      open();
      dragging_ = popup_.has_value();
      return true;
    }
    return false;
  }
  // The button was released: if it opened the popup, letting go over an item chooses it, letting
  // go over the control leaves the popup open, and anywhere else closes it again
  if (not dragging_) {
    return false;
  }
  dragging_ = false;
  if (popup_) {
    if (const auto item{popup_->itemAt(popupPosition(position))}; item >= 0) {
      if (popupItemEnabled(item)) {
        popupActivate(item, event);
      }
    } else if (not inside) {
      close();
    }
  }
  return true;
}

auto PopupChoice::handleMouseMove(const Event &event) -> bool {
  if (not popup_) {
    return false;
  }
  const auto item{popup_->itemAt(popupPosition(event.as<MouseMoveEvent>().position()))};
  if (item < 0 or item == popup_->highlighted() or not popupItemEnabled(item)) {
    return false;
  }
  popup_->highlight(item);
  return true;
}

void PopupChoice::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0) {
    return;
  }
  const auto state{static_cast<size_t>(widgetState(popup_.has_value()))};
  const auto &glyphs{uiStyle().dropDown[state]};
  const Text left{glyphs.left}, right{glyphs.right};
  auto visible{canvas(dropDownRoles[state])};
  const auto position{contentPosition(Vector{size.x(), 1})};
  const auto framed{size.x() >= left.width() + right.width()};
  const auto textWidth{framed? toDim(size.x() - left.width() - right.width()): size.x()};
  if (framed) {
    overlay(visible, left, position);
    overlay(visible, right, Vector{toDim(size.x() - right.width()), position.y()});
  }
  if (const auto *shown{closedText()}; shown) {
    const auto width{shown->width()};
    const auto extra{static_cast<int>(textWidth) - width};
    auto aligned{Dim{0}};
    switch (horizontalAlignment()) {
      case ContentAlignment::center:
      aligned = toDim(extra / 2);
      break;
      case ContentAlignment::end:
      aligned = toDim(extra);
      break;
      case ContentAlignment::begin:
      default:
      break;
    }
    const auto clippedStart{std::max(Dim{0}, toDim(-std::min(0, static_cast<int>(aligned))))};
    const auto shownWidth{std::max(Dim{0}, std::min(toDim(width - clippedStart), textWidth))};
    if (shown->height() > 0 and shownWidth > 0) {
      overlay(
        visible,
        (*shown)[Rectangle{clippedStart, 0, toDim(clippedStart + shownWidth), 1}],
        Vector{
          toDim(position.x() + (framed? left.width(): Dim{0}) + std::max(Dim{0}, aligned)),
          position.y()
        }
      );
    }
  }
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
