#include "label.hh"
#include "box.hh"

#include <algorithm>

namespace jwezel::ui {

Label::Label(Container &parent, const Rectangle &area, const Text &text, const Char &background):
Widget{area, background, parent},
text_{text}
{
  focusPolicy(FocusPolicy::none);
  render();
  layoutChanged(); // the text is what the label prefers to be
}

Label::Label(Container &parent, const Text &text, const Char &background):
Widget{RectangleDefault, background, parent},
text_{text}
{
  focusPolicy(FocusPolicy::none);
  render();
  layoutChanged();
}

Label::~Label() {
  if (buddy_) {
    std::erase(buddy_->layoutDependents_, this);
  }
}

void Label::content(const Text &text) {
  text_ = text;
  render();
  layoutChanged();
}

void Label::buddy(Widget &widget) {
  if (buddy_ == &widget) {
    return;
  }
  if (buddy_) {
    std::erase(buddy_->layoutDependents_, this);
  }
  buddy_ = &widget;
  buddy_->layoutDependents_.push_back(this);
  render();
  layoutChanged();
}

auto Label::preferredSize() -> Vector {
  auto size{text_.size()};
  if (buddy_ and !uiStyle().labelSuffix.empty()) {
    const auto line{text_.height() > 0? toDim(text_.height() - 1): Dim{0}};
    const auto lastLineWidth{text_.data.empty()? Dim{0}: toDim(text_.data.back().size())};
    size.x() = std::max(size.x(), toDim(lastLineWidth + Text{uiStyle().labelSuffix}.width()));
    size.y() = std::max(size.y(), toDim(line + Text{uiStyle().labelSuffix}.height()));
  }
  const auto *labelBox{dynamic_cast<const Box *>(parent())};
  const auto *buddyBox{buddy_? dynamic_cast<const Box *>(buddy_->parent()): nullptr};
  if (buddy_ and labelBox and buddyBox and labelBox != buddyBox and labelBox->orientation() == buddyBox->orientation()) {
    const auto buddySize{buddy_->preferredSize()};
    if (labelBox->orientation() == Orientation::vertical) {
      size.y() = std::max(size.y(), buddySize.y());
    } else {
      size.x() = std::max(size.x(), buddySize.x());
    }
  }
  return size;
}

void Label::render() {
  if (area().size().x() <= 0 or area().size().y() <= 0) {
    return;
  }
  auto contentSize{text_.size()};
  auto suffix{Text{}};
  auto suffixPosition{Vector{0, 0}};
  if (buddy_ and !uiStyle().labelSuffix.empty()) {
    suffix = Text{uiStyle().labelSuffix};
    const auto line{text_.height() > 0? toDim(text_.height() - 1): Dim{0}};
    const auto column{text_.data.empty()? Dim{0}: toDim(text_.data.back().size())};
    suffixPosition = Vector{column, line};
    contentSize.x() = std::max(contentSize.x(), toDim(column + suffix.width()));
    contentSize.y() = std::max(contentSize.y(), toDim(line + suffix.height()));
  }
  auto visible{canvas(labelRole)};
  const auto position{contentPosition(contentSize)};
  overlay(visible, text_, position);
  if (!suffix.data.empty()) {
    overlay(visible, suffix, position + suffixPosition);
  }
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
