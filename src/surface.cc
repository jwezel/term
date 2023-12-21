#include "geometry.hh"
#include "surface.hh"
#include "update.hh"

#include <algorithm>
#include <format>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/geometry.hpp>

namespace jwezel {

using std::ranges::views::transform, std::ranges::views::filter;
using std::format;
using std::vector, std::domain_error;

namespace
{
void split(const Rectangle &area, Surface::Element &element1, const Surface::Element &element2) {
  vector<Surface::Fragment> &fragments1{element1.fragments};
  const vector<Surface::Fragment> &fragments2{element2.fragments};
  for (const auto &fragment2: fragments2) {
    if (area.intersects(fragment2.area)) {
      vector<Surface::Fragment> fragments;
      for (const auto &fragment1: fragments1) {
        const auto intersection{fragment1.area.defaultIntersection(fragment2.area)};
        std::ranges::copy(
          intersection | transform([&element1](const Rectangle &r){return Surface::Fragment(r, &element1);}),
          back_inserter(fragments)
        );
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
fragments{Fragment{area, this}},
surface_{surface}
{}

void Surface::Element::update(const vector<Rectangle> &areas) {
  vector<Fragment> updates;
  for (const auto &fragment: fragments) {
    for (const auto &area_: areas) {
      const auto update_{fragment.area & area_ + area().position()};
      if (update_) {
        updates.emplace_back(update_.value(), this);
      }
    }
  }
  surface_->device_->update(SurfaceUpdates(updates));

}

Surface::Fragment::operator string() const {
  return format("Fragment({}, {})", string(area), long(element));
}

Surface::Surface(Device *device, initializer_list<Element *> initializer):
device_{device}
{
  for (auto e: initializer) {
    addElement(e);
  }
}

void Surface::removeRtreeFragments(Surface::Element &element) {
  for (auto &fragment: element.fragments) {
    rtree.remove(std::make_pair(fragment.area, &fragment));
  }
}

void Surface::insertRtreeFragments(Surface::Element &element) {
  for (auto &fragment: element.fragments) {
    rtree.insert(std::make_pair(fragment.area, &fragment));
  }
}

void Surface::addElement(Surface::Element *element, Surface::Element *below) {
  // Add element to surface
  const auto insertPos =
    below?
      std::find(zorder.begin(), zorder.end(), below)
    :
      zorder.end();
  const auto ze = std::distance(zorder.begin(), zorder.insert(insertPos, element));
  // Create own fragments from overlaying fragments when inserting it below the top
  for (auto j = ze + 1; j < int(zorder.size()); ++j) {
    split(element->area(), *element, *zorder[j]);
  }
  // Create fragments of all elements overlayed by this
  for (auto z = ze - 1; z >= 0; --z) {
    auto *const elementBelow = zorder[z];
    if (elementBelow->area().intersects(element->area())) {
      removeRtreeFragments(*elementBelow);
      split(elementBelow->area(), *elementBelow, *element);
      insertRtreeFragments(*elementBelow);
    }
  }
  // TODO(j): Find a better way to test for the backdrop. Creating a device update
  // for the backdrop is not necessary and creates problems
  if (ze) {
    device_->update(SurfaceUpdates(element->fragments));
  }
  insertRtreeFragments(*element);
}

void Surface::deleteElement(Element *element) {
  Updates result;
  const auto zit{std::find(zorder.begin(), zorder.end(), element)};
  if (zit == zorder.end()) {
    throw domain_error(format("Element {} not found in zorder\n", static_cast<void*>(element)));
  }
  const auto ze = std::distance(zorder.begin(), zit);
  vector<Fragment> updates;
  // Remove element from surface
  removeRtreeFragments(*element);
  zorder.erase(zorder.begin() + ze);
  // Recreate fragments covered by removed element
  for (auto z1 = ze - 1; z1 >= 0; --z1) {
    if (zorder[z1]->area().intersects(element->area())) {
      Element * const zelement = zorder[z1];
      removeRtreeFragments(*zelement);
      zelement->fragments = {Surface::Fragment{zelement->area(), zelement}};
      insertRtreeFragments(*zelement);
      for (auto z2 = z1 + 1; z2 < int(zorder.size()); ++z2) {
        removeRtreeFragments(*zelement);
        split(zelement->area(), *zelement, *zorder[z2]);
        insertRtreeFragments(*zelement);
      }
      // Add surface updates for these fragments
      std::ranges::copy(
        zelement->fragments |
        transform([element](const Surface::Fragment &f) {return f.area & element->area();}) |
        filter([](const std::optional<Rectangle> &r) {return r.has_value();}) |
        transform([zelement](const std::optional<Rectangle> &r){return Surface::Fragment{r.value(), zelement};}),
        std::back_inserter(updates)
      );
    }
  }
  device_->update(SurfaceUpdates(updates));
}

void Surface::reshapeElement(Element *element, const Rectangle &area) { //NOLINT(readability-function-cognitive-complexity)
  vector<Fragment> updates;
  if (element->area() != area) {
    const auto zpos = std::find(zorder.begin(), zorder.end(), element);
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
        removeRtreeFragments(*element_);
        // Create fragments from elements on top it
        element_->fragments = {Surface::Fragment{element_->area(), element_}};
        for (auto i = j + 1; i < int(zorder.size()); ++i) {
          split(element_->area(), *element_, *zorder[i]);
        }
        insertRtreeFragments(*element_);
        // Surface update fragments
        for (const auto &fragment: element_->fragments) {
          for (const auto &damageArea: damageAreas) {
            const auto intersection = fragment.area & damageArea;
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

auto Surface::find(const Vector &position) const -> Fragment * {
  vector<RtreeEntry> result;
  rtree.query(boost::geometry::index::contains(Rectangle{position, position + Vector{1, 1}}), std::back_inserter(result));
  switch (result.size()) {
    case 0:
    return 0;

    case 1:
    return result[0].second;

    default:
    throw std::logic_error(format("Found {} results. Expected only one", result.size()));
  }
}

} // namespace jwezel
