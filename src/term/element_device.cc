#include "element_device.hh"

namespace jwezel {

auto ElementDevice::Underlay::text(const Rectangle &area) const -> Text {
  return device_->ownText(area);
}

ElementDevice::ElementDevice(Surface *surface, const Rectangle &area, const Char &background, TextElement *below):
TextElement{surface, area, background, below}
{}

auto ElementDevice::ownText(const Rectangle &area) const -> Text {
  const auto own{Rectangle{Vector{0, 0}, TextElement::text().size()}};
  const auto area_{area == RectangleMax? own: area};
  Text result{background(), area_.size()};
  if (const auto part{area_ & own}) {
    result.patch(
      TextElement::text(*part),
      part->position() - area_.position(),
      AttributeMode::replace,
      AttributeMode::replace
    );
  }
  return result;
}

void ElementDevice::update(const Updates &updates) {
  vector<Rectangle> areas;
  areas.reserve(updates.size());
  for (const auto &update_: updates) {
    areas.emplace_back(update_.position, update_.position + update_.text.size());
  }
  Surface::Element::update(areas);
}

void ElementDevice::update(const vector<Rectangle> &areas) {
  Surface::Element::update(areas);
}

void ElementDevice::update(const Rectangle &area) {
  Surface::Element::update(area);
}

auto ElementDevice::text(const Rectangle &area) const -> Text {
  const auto own{Rectangle{Vector{0, 0}, TextElement::text().size()}};
  const auto area_{area == RectangleMax? own: area};
  Text result{ownText(area_)};
  for (const auto *element: content_.zorder()) {
    if (element == &underlay_) {
      continue;
    }
    for (const auto &fragment: element->fragments()) {
      const auto part{fragment.area & area_};
      if (part) {
        result.patch(
          element->text(*part - element->area().position()),
          part->position() - area_.position(),
          AttributeMode::replace,
          AttributeMode::replace
        );
      }
    }
  }
  return result;
}

auto ElementDevice::moveEvent(const Rectangle &area) -> bool {
  TextElement::moveEvent(area);
  return true;
}

auto ElementDevice::shows(const Vector &position, const Surface::Element *owner) const -> bool {
  if (this == owner) {
    return true;
  }
  const auto local{position - area().position()};
  const auto *fragment{content_.find(local)};
  return fragment and fragment->element->shows(local, owner);
}

} // namespace jwezel
