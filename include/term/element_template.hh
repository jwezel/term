#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <memory>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <range/v3/action/push_back.hpp>
#include <range/v3/action/remove_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>

namespace jwezel {

using namespace ranges;

template<typename ELEMENT, typename BASE=ELEMENT>
struct Elements {
  struct Fragment {
    Rectangle area;
    BASE *element;

    operator string() const;
  };

  std::vector<BASE *> zorder;
  std::unordered_map<ELEMENT *, std::unique_ptr<ELEMENT>> elements;

  tuple<ELEMENT *, Updates> addElement(const jwezel::Rectangle &area_, const jwezel::Char &background, ELEMENT *below, ELEMENT *) {
    // Setup
    const auto topArea = zorder.back()->area();
    const auto area =
      area_ == RectangleDefault?
        Rectangle(topArea.x1 + 1, topArea.y1 + 1, topArea.x2 + 1, topArea.y2 + 1)
      :
        area_;
    // Create element
    auto element = new ELEMENT(area, background);
    elements.insert(make_pair(element, unique_ptr<ELEMENT>(element)));
    // focusElement = element;
    // Add element
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
    return std::make_tuple(
      element,
      screenUpdates(
      element->fragments | views::transform([element](const Rectangle &area){return Fragment{area, element};}) | to<vector>()
      )
    );
  }

  Updates deleteElement(ELEMENT *element) {
    Updates result;
    if (unfocus(element)) {
      const auto zit{find(zorder.begin(), zorder.end(), element)};
      if (zit == zorder.end()) {
        throw domain_error(format("Element {} not found in zorder\n", fmt::ptr(element)));
      }
      const auto z = std::distance(zorder.begin(), zit);
      vector<Fragment> updates;
      // Remove element from screen
      zorder.erase(zorder.begin() + z);
      // Recreate fragments covered by removed element
      for (auto z1 = z - 1; z1 >= 0; --z1) {
        if (zorder[z1]->area().intersects(element->area())) {
          BASE *zelement = zorder[z1];
          zelement->fragments = {zelement->area()};
          for (auto z2 = z1 + 1; z2 < int(zorder.size()); ++z2) {
            split(zelement->area(), zelement->fragments, zorder[z2]->fragments);
          }
          // Add screen updates for these fragments
          updates |= push_back(
            zelement->fragments |
            views::transform([element](const Rectangle &r) {return r & element->area();}) |
            views::remove_if([](const std::optional<Rectangle> &r) {return !r.has_value();}) |
            views::transform([zelement](const std::optional<Rectangle> &r) {return Fragment{r.value(), zelement};})
          );
        }
      }
      result = screenUpdates(updates);
      // Remove element from registry
      if (!elements.erase(element)) {
        throw domain_error(format("Element {} not found in elements\n", fmt::ptr(element)));
      }
    }
    return result;
  }

  Updates reshapeElement(ELEMENT *element, const Rectangle &area) {
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
          const auto element = zorder[j];
          element->fragments = {element->area()};
          for (auto i = j + 1; i < int(zorder.size()); ++i) {
            split(element->area(), element->fragments, zorder[i]->fragments);
          }
          // Update surface fragments
          for (const auto &fragment: element->fragments) {
            for (const auto &damageArea: damageAreas) {
              const auto intersection = fragment & damageArea;
              if (intersection) {
                updates.push_back(Fragment{intersection.value(), element});
              }
            }
          }
        }
      }
    }
    return screenUpdates(updates);
  }
};

} // namespace jwezel
