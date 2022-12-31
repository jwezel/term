#include "surface.hh"
#include "fmt/format.h"
#include "update.hh"

#include <algorithm>

#include <range/v3/action/push_back.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/action/remove_if.hpp>
#include <range/v3/view/transform.hpp>

namespace jwezel {

using namespace ranges;
using fmt::format;

static void split(const Rectangle &area, vector<Rectangle> &fragments1, const vector<Rectangle> fragments2) {
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

Updates SurfaceUpdates(const vector<Surface::Fragment> &fragments) {
  Updates result;
  result.reserve(fragments.size());
  for (const auto &fragment: fragments) {
    const auto area = fragment.area;
    result.emplace_back(
      Update{
        Vector(fragment.area.x1, fragment.area.y1),
        fragment.element->text(area - Vector(fragment.element->area().x1, fragment.element->area().y1))
      }
    );
  }
  return result;
}

Surface::Fragment::operator string() const {
  return format("Fragment({})", string(area));
}

Surface::Surface(initializer_list<Element *> initializer):
zorder{initializer}
{}

Updates Surface::addElement(Surface::Element *element, Surface::Element *below) {
  // Create element
  elements.insert(make_pair(element, unique_ptr<Element>(element)));
  // Add element to surface
  const auto insertPos =
    below?
      find(zorder.begin(), zorder.end(), below)
    :
      zorder.end();
  const auto i = std::distance(zorder.begin(), zorder.insert(insertPos, element));
  // Create own fragments from overlaying fragments when inserting it below the top
  for (auto j = i + 1; j < int(zorder.size()); ++j) {
    split(element->area(), element->fragments, zorder[j]->fragments);
  }
  // Create fragments of all elements overlayed by this
  for (auto z = i - 1; z >= 0; --z) {
    const auto elementBelow = zorder[z];
    if (elementBelow->area().intersects(element->area())) {
      split(elementBelow->area(), elementBelow->fragments, element->fragments);
    }
  }
  return SurfaceUpdates(
    element->fragments | views::transform([element](const Rectangle &area){return Fragment{area, element};}) | to<vector>()
  );
}

Updates Surface::deleteElement(Element *element) {
  Updates result;
  const auto zit{find(zorder.begin(), zorder.end(), element)};
  if (zit == zorder.end()) {
    throw domain_error(format("Element {} not found in zorder\n", fmt::ptr(element)));
  }
  const auto z = std::distance(zorder.begin(), zit);
  vector<Fragment> updates;
  // Remove element from surface
  zorder.erase(zorder.begin() + z);
  // Recreate fragments covered by removed element
  for (auto z1 = z - 1; z1 >= 0; --z1) {
    if (zorder[z1]->area().intersects(element->area())) {
      Element *zelement = zorder[z1];
      zelement->fragments = {zelement->area()};
      for (auto z2 = z1 + 1; z2 < int(zorder.size()); ++z2) {
        split(zelement->area(), zelement->fragments, zorder[z2]->fragments);
      }
      // Add surface updates for these fragments
      updates |= push_back(
        zelement->fragments |
        views::transform([element](const Rectangle &r) {return r & element->area();}) |
        views::remove_if([](const std::optional<Rectangle> &r) {return !r.has_value();}) |
        views::transform([zelement](const std::optional<Rectangle> &r) {return Fragment{r.value(), zelement};})
      );
    }
  }
  result = SurfaceUpdates(updates);
  // Remove element from registry
  if (!elements.erase(element)) {
    throw domain_error(format("Element {} not found in elements\n", fmt::ptr(element)));
  }
  return result;
}

Updates Surface::reshapeElement(Element *element, const Rectangle &area) {
  vector<Fragment> updates;
  if (element->area() != area) {
    const auto zpos = find(zorder.begin(), zorder.end(), element);
    if (zpos == zorder.end()) {
      throw range_error(format("Element {} not found", fmt::ptr(element)));
    }
    const auto z = std::distance(zorder.begin(), zpos);
    auto damageAreas{element->area().defaultIntersection(area)};
    damageAreas.push_back(area);
    const auto searchArea = element->area() | area;
    element->move(area);
    // Recreate fragments
    for (auto j = z; j >= 0; --j) {
      if (zorder[j]->area().intersects(searchArea)) {
        // Element fragments
        const auto element_ = zorder[j];
        element_->fragments = {element_->area()};
        for (auto i = j + 1; i < int(zorder.size()); ++i) {
          split(element_->area(), element_->fragments, zorder[i]->fragments);
        }
        // Surface update fragments
        for (const auto &fragment: element_->fragments) {
          for (const auto &damageArea: damageAreas) {
            const auto intersection = fragment & damageArea;
            if (intersection) {
              updates.push_back(Fragment{intersection.value(), element_});
            }
          }
        }
      }
    }
  }
  return SurfaceUpdates(updates);
}

} // namespace jwezel