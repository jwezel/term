#include "geometry.hh"
#include "surface.hh"
#include "update.hh"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <vector>

#include <format>

namespace jwezel {

using std::ranges::views::transform, std::ranges::views::filter;
using std::format;
using std::vector, std::domain_error;

namespace
{
void split(const Rectangle &area, vector<Rectangle> &fragments1, const vector<Rectangle> &fragments2) {
  for (const auto &fragment2: fragments2) {
    if (area.intersects(fragment2)) {
      vector<Rectangle> fragments;
      for (const auto &fragment1: fragments1) {
        const auto intersection{fragment1.defaultIntersection(fragment2)};
        std::copy(intersection.begin(), intersection.end(), back_inserter(fragments));
      }
      fragments1 = fragments;
    }
  }
}
} // namespace

template<class Range>
auto SurfaceUpdates(const Range &fragments) -> Updates {
  Updates result;
  result.reserve(std::ranges::distance(fragments));
  for (const auto &fragment: fragments) {
    const auto area = fragment.area;
    result.emplace_back(
      Update{
        Vector(fragment.area.x1(), fragment.area.y1()),
        fragment.element->text(
          area -
          fragment.element->area().position()
        )
      }
    );
  }
  return result;
}

Surface::Element::Element(Surface *surface, const Rectangle &area):
fragments{area},
surface_{surface}
{}

void Surface::Element::update(const vector<Rectangle> &areas) {
  vector<Fragment> updates;
  for (const auto &fragment: fragments) {
    for (const auto &area_: areas) {
      const auto update{fragment & area_ + area().position()};
      if (update) {
        updates.emplace_back(update.value(), this);
      }
    }
  }
  surface_->device_->update(SurfaceUpdates(updates));

}

Surface::Fragment::operator string() const {
  return format("Fragment({})", string(area));
}

Surface::Surface(Device *device, initializer_list<Element *> initializer):
zorder{initializer},
device_{device}
{}

void Surface::addElement(Surface::Element *element, Surface::Element *below) {
  // Add element to surface
  const auto insertPos =
    below?
      find(zorder.begin(), zorder.end(), below)
    :
      zorder.end();
  const auto ze = std::distance(zorder.begin(), zorder.insert(insertPos, element));
  // Create own fragments from overlaying fragments when inserting it below the top
  for (auto j = ze + 1; j < int(zorder.size()); ++j) {
    split(element->area(), element->fragments, zorder[j]->fragments);
  }
  // Create fragments of all elements overlayed by this
  for (auto z = ze - 1; z >= 0; --z) {
    auto *const elementBelow = zorder[z];
    if (elementBelow->area().intersects(element->area())) {
      split(elementBelow->area(), elementBelow->fragments, element->fragments);
    }
  }
  device_->update(
    SurfaceUpdates(
      element->fragments | transform([element](const Rectangle &area){return Fragment{area, element};})
    )
  );
}

void Surface::deleteElement(Element *element) {
  Updates result;
  const auto zit{find(zorder.begin(), zorder.end(), element)};
  if (zit == zorder.end()) {
    throw domain_error(format("Element {} not found in zorder\n", static_cast<void*>(element)));
  }
  const auto ze = std::distance(zorder.begin(), zit);
  vector<Fragment> updates;
  // Remove element from surface
  zorder.erase(zorder.begin() + ze);
  // Recreate fragments covered by removed element
  for (auto z1 = ze - 1; z1 >= 0; --z1) {
    if (zorder[z1]->area().intersects(element->area())) {
      Element *zelement = zorder[z1];
      zelement->fragments = {zelement->area()};
      for (auto z2 = z1 + 1; z2 < int(zorder.size()); ++z2) {
        split(zelement->area(), zelement->fragments, zorder[z2]->fragments);
      }
      // Add surface updates for these fragments
      std::ranges::copy(
        zelement->fragments |
        transform([element](const Rectangle &r) {return r & element->area();}) |
        filter([](const std::optional<Rectangle> &r) {return r.has_value();}) |
        transform([zelement](const std::optional<Rectangle> &r) {return Fragment{r.value(), zelement};}),
        std::back_inserter(updates)
      );
    }
  }
  device_->update(SurfaceUpdates(updates));
}

void Surface::reshapeElement(Element *element, const Rectangle &area) { //NOLINT(readability-function-cognitive-complexity)
  vector<Fragment> updates;
  if (element->area() != area) {
    const auto zpos = find(zorder.begin(), zorder.end(), element);
    if (zpos == zorder.end()) {
      throw range_error(format("Element {} not found", static_cast<void*>(element)));
    }
    const auto ze = std::distance(zorder.begin(), zpos);
    if (ze == 0) {
      throw logic_error("Lowest element must be the 'backdrop', which must not be moved");
    }
    auto damageAreas{element->area().defaultIntersection(area)};
    damageAreas.push_back(area);
    const auto searchArea = element->area() | area;
    element->moveEvent(area);
    // Recreate fragments
    for (auto j = ze; j >= 0; --j) {
      auto * const element_ = zorder[j];
      if (element_->area().intersects(searchArea)) {
        // Create fragments from elements on top it
        element_->fragments = {element_->area()};
        for (auto i = j + 1; i < int(zorder.size()); ++i) {
          split(element_->area(), element_->fragments, zorder[i]->fragments);
        }
        // Surface update fragments
        for (const auto &fragment: element_->fragments) {
          for (const auto &damageArea: damageAreas) {
            const auto intersection = fragment & damageArea;
            if (intersection) {
              updates.emplace_back(intersection.value(), element_);
            }
          }
        }
      }
    }
  }
  device_->update(SurfaceUpdates(updates));
}

auto Surface::minSize(const Element *exclude) const -> Vector {
  Vector result{0, 0};
  bool consider{false};
  for (auto *const element: zorder) {
    if (consider) {
      result = max(result, element->area().position2());
    }
    if (exclude and element == exclude) {
      consider = true;
    }
  }
  return result;
}

} // namespace jwezel
