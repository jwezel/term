#include "drop_down.hh"
#include "root.hh"

#include <algorithm>

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

namespace {

const Unicode Escape{0x1b};

auto limit(Dim index, const std::vector<Text> &items) -> Dim {
  return items.empty()? Dim{-1}: std::clamp(index, Dim{0}, toDim(items.size() - 1));
}

auto contains(const Rectangle &area, const Vector &position) -> bool {
  return bool{area & Rectangle{position, position + Vector{1, 1}}};
}

} // namespace

DropDown::DropDown(Container &parent, const Rectangle &area, const std::vector<Text> &items, Dim selected, const Char &background):
Widget{area, background, parent},
items_{items},
selected_{limit(selected, items)}
{
  render();
  registerHandlers();
}

DropDown::DropDown(Container &parent, const std::vector<Text> &items, Dim selected, const Char &background):
Widget{RectangleDefault, background, parent},
items_{items},
selected_{limit(selected, items)}
{
  render();
  registerHandlers();
}

void DropDown::registerHandlers() {
  onKey([this](const ui::Element &, const Event &event) {return handleKey(event);});
  onMouseButton([this](const ui::Element &, const Event &event) {return handleMouseButton(event);});
  onMouseMove([this](const ui::Element &, const Event &event) {return handleMouseMove(event);});
}

void DropDown::items(const std::vector<Text> &items) {
  close();
  items_ = items;
  selected_ = limit(selected_, items_);
  render();
}

void DropDown::select(Dim index) {
  selected_ = limit(index, items_);
  render();
}

void DropDown::open() {
  if (popup_ or items_.empty()) {
    return;
  }
  const auto self{bounds()};
  auto width{self.width()};
  const auto &glyphs{uiStyle().popup};
  const auto padding{toDim(Text{glyphs.left}.width() + Text{glyphs.right}.width())};
  for (const auto &item: items_) {
    width = std::max(width, toDim(item.width() + padding));
  }
  const auto rows{std::min(toDim(items_.size()), std::max(Dim{1}, maxRows_))};
  popup_.emplace(
    *root(),
    Rectangle{self.x1(), self.y2(), toDim(self.x1() + width), toDim(self.y2() + rows)},
    items_,
    std::max(selected_, Dim{0}),
    *this,
    background()
  );
  render(); // looks in use
}

void DropDown::close() {
  const auto wasOpen{popup_.has_value()};
  popup_.reset();
  dragging_ = false;
  if (wasOpen) {
    render(); // does not look in use anymore
  }
}

void DropDown::maxRows(Dim rows) {
  maxRows_ = std::max(Dim{1}, rows);
}

auto DropDown::onSelect(const Handler &handler) -> int {
  selectHandlers_.push_back(handler);
  return static_cast<int>(selectHandlers_.size() - 1);
}

void DropDown::move(const Rectangle &area) {
  close();
  Widget::move(area);
}

void DropDown::focusChanged(bool focused) {
  if (!focused) {
    close();
  }
  Widget::focusChanged(focused);
}

void DropDown::pressedElsewhere() {
  close();
}

void DropDown::choose(Dim index, const Event &event) {
  select(index);
  close();
  for (const auto &handler : selectHandlers_) {
    handler(*this, event);
  }
}

auto DropDown::handleKey(const Event &event) -> bool {
  const auto key{dynamic_cast<KeyEvent *>(event())->key()};
  if (!popup_) {
    if (key == ' ' or key == Key::Enter or key == Key::Down) {
      open();
      return true;
    }
    return false;
  }
  const auto current{popup_->highlighted()};
  const auto last{toDim(items_.size() - 1)};
  switch (key) {
    case Key::Up:
    popup_->highlight(toDim(current - 1));
    break;

    case Key::Down:
    popup_->highlight(toDim(current + 1));
    break;

    case Key::Home:
    popup_->highlight(0);
    break;

    case Key::End:
    popup_->highlight(last);
    break;

    case Key::PageUp:
    popup_->highlight(toDim(current - popup_->rows()));
    break;

    case Key::PageDown:
    popup_->highlight(toDim(current + popup_->rows()));
    break;

    case Key::Enter:
    case ' ':
    choose(current, event);
    break;

    case Escape:
    close();
    break;

    default:
    return false;
  }
  return true;
}

auto DropDown::handleMouseButton(const Event &event) -> bool {
  auto *mouse{dynamic_cast<MouseButtonEvent *>(event())};
  const auto position{mouse->position()};
  if (mouse->modifiers().mod5) { // the wheel: button 1 is up, button 2 is down
    if (popup_ and mouse->action() == MouseAction::Down) {
      popup_->highlight(toDim(popup_->highlighted() + (mouse->button() == MouseButton::Button1? -1: 1)));
      return true;
    }
    return false;
  }
  if (mouse->button() != MouseButton::Button1) {
    return false;
  }
  const auto inside{contains(bounds(), position)};
  if (mouse->action() == MouseAction::Down) {
    if (popup_) {
      if (const auto item{popup_->itemAt(position)}; item >= 0) {
        choose(item, event);
        return true;
      }
      if (inside) {
        close();
        return true;
      }
      return false;
    }
    if (inside) {
      open();
      dragging_ = popup_.has_value();
      return true;
    }
    return false;
  }
  // The button was released: if it opened the list, letting go over an item chooses it, letting
  // go over the drop down leaves the list open, and anywhere else closes it again
  if (!dragging_) {
    return false;
  }
  dragging_ = false;
  if (popup_) {
    if (const auto item{popup_->itemAt(position)}; item >= 0) {
      choose(item, event);
    } else if (!inside) {
      close();
    }
  }
  return true;
}

auto DropDown::handleMouseMove(const Event &event) -> bool {
  if (!popup_) {
    return false;
  }
  const auto item{popup_->itemAt(dynamic_cast<MouseMoveEvent *>(event())->position())};
  if (item < 0 or item == popup_->highlighted()) {
    return false;
  }
  popup_->highlight(item);
  return true;
}

void DropDown::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0) {
    return;
  }
  const auto state{static_cast<size_t>(widgetState(popup_.has_value()))};
  const auto &glyphs{uiStyle().dropDown[state]};
  const Text left{glyphs.left}, right{glyphs.right};
  auto visible{canvas(dropDownRoles[state])};
  const auto framed{size.x() >= left.width() + right.width()};
  const auto textWidth{framed? toDim(size.x() - left.width() - right.width()): size.x()};
  if (framed) {
    overlay(visible, left, Vector{0, 0});
    overlay(visible, right, Vector{toDim(size.x() - right.width()), 0});
  }
  if (selected_ >= 0) {
    const auto &item{items_[static_cast<size_t>(selected_)]};
    const auto width{std::min(item.width(), textWidth)};
    if (item.height() > 0 and width > 0) {
      overlay(visible, item[Rectangle{0, 0, width, 1}], Vector{framed? left.width(): Dim{0}, 0});
    }
  }
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
