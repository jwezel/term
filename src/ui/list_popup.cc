#include "list_popup.hh"

#include <algorithm>

namespace jwezel::ui {

ListPopup::ListPopup(
  Container &parent,
  const Rectangle &area,
  const std::vector<Text> &items,
  Dim highlighted,
  Widget &owner,
  const Char &background
):
Widget{area, background, parent},
items_{items},
highlighted_{0},
owner_{owner}
{
  highlight(highlighted);
}

void ListPopup::highlight(Dim index) {
  const auto count{toDim(items_.size())};
  highlighted_ = std::clamp(index, Dim{0}, std::max(Dim{0}, toDim(count - 1)));
  const auto visible{std::max(Dim{1}, rows())};
  if (highlighted_ < first_) {
    first_ = highlighted_;
  }
  if (highlighted_ >= toDim(first_ + visible)) {
    first_ = toDim(highlighted_ - visible + 1);
  }
  render();
}

auto ListPopup::itemAt(const Vector &position) -> Dim {
  const auto area{bounds()};
  const Rectangle point{position, position + Vector{1, 1}};
  if (!(area & point)) {
    return -1;
  }
  const auto item{toDim(first_ + position.y() - area.y1())};
  return item < toDim(items_.size())? item: -1;
}

void ListPopup::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0) {
    return;
  }
  const auto &glyphs{uiStyle().popup};
  const Text left{glyphs.left}, right{glyphs.right};
  const auto padding{toDim(left.width() + right.width())};
  auto visible{canvas(popupRole)};
  for (Dim row = 0; row < size.y(); ++row) {
    const auto index{toDim(first_ + row)};
    if (index >= toDim(items_.size())) {
      break;
    }
    const auto &item{items_[static_cast<size_t>(index)]};
    const auto width{std::min(item.width(), toDim(size.x() - padding))};
    overlay(visible, left, Vector{0, row});
    overlay(visible, right, Vector{toDim(size.x() - right.width()), row});
    if (item.height() > 0 and width > 0) {
      overlay(visible, item[Rectangle{0, 0, width, 1}], Vector{left.width(), row});
    }
    if (index == highlighted_) {
      visible.setAttr(colors()[popupHighlightRole], Rectangle{0, row, size.x(), toDim(row + 1)}, AttributeMode::replace);
    }
  }
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
