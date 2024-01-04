#include "geometry.hh"
#include "surface.hh"
#include "text.hh"
#include "update.hh"

#include <algorithm>
#include <format>
#include <iostream>
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

///
/// Get non-intersecting parts from two ranges of fragments
///
/// @param[in]  area        The area
/// @param      fragments1  The fragments 1
/// @param[in]  fragments2  The fragments 2
void split(const Rectangle & area, vector<Surface::Fragment> &fragments1, const vector<Surface::Fragment> &fragments2) {
  auto *element{fragments1[0].element};
  for (const auto &fragment2: fragments2) {
    if (area.intersects(fragment2.area)) {
      vector<Surface::Fragment> fragments;
      for (const auto &fragment1: fragments1) {
        const auto intersection{fragment1.area.defaultIntersection(fragment2.area)};
        std::ranges::copy(
          intersection | transform([element](const Rectangle &r){return Surface::Fragment(r, element);}),
          back_inserter(fragments)
        );
      }
      fragments1 = fragments;
    }
  }
}

void cover(Surface::Element &element1, const Surface::Element &element2) {
  split(element1.area(), element1.fragments(), element2.fragments());
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
          area - fragment.element->area().position()
        )
      }
    );
  }
  return result;
}

Surface::Element::Element(Surface *surface):
surface_{surface}
{}

Surface::Element::~Element() = default;

void Surface::Element::update(const vector<Rectangle> &areas) {
  vector<Fragment> updates;
  for (const auto &fragment: fragments_) {
    for (const auto &area_: areas) {
      const auto update_{fragment.area & area_ + area().position()};
      if (update_) {
        updates.emplace_back(update_.value(), this);
      }
    }
  }
  surface_->update(updates);
}

void Surface::Element::update(const Rectangle &area_) {
  vector<Fragment> updates;
  for (const auto &fragment: fragments_) {
    const auto update_{fragment.area & area_ + area().position()};
    if (update_) {
      updates.emplace_back(update_.value(), this);
    }
  }
  surface_->update(updates);
}

void Surface::Element::move(const Rectangle &area) {
  surface()->reshapeElement(this, area);
}

Surface::Fragment::operator string() const {
  return format("Fragment({}, {})", string(area), long(element));
}

Surface::Surface(Device *device, initializer_list<Element *> initializer):
device_{device}
{
  for (auto *element: initializer) {
    addElement(element);
  }
}

void Surface::update(const vector<Fragment> &updates) {
  device_->update(SurfaceUpdates(updates));
}

void Surface::removeRtreeFragments(Surface::Element &element) {
  for (auto &fragment: element.fragments()) {
    rtree.remove(std::make_pair(fragment.area, &fragment));
  }
}

void Surface::insertRtreeFragments(Surface::Element &element) {
  for (auto &fragment: element.fragments()) {
    rtree.insert(std::make_pair(fragment.area, &fragment));
  }
}

void Surface::cover(long pos) {
  auto *element{zorder_[pos]};
  element->fragments_ = {Surface::Fragment{element->area(), element}};
  for (unsigned j = pos + 1; j < zorder_.size(); ++j) {
    jwezel::cover(*element, *zorder_[j]);
  }
}

void Surface::addElement(Surface::Element *element, Surface::Element *below) {
  // Add element to surface
  const auto insertPos =
    below?
      std::find(zorder_.begin(), zorder_.end(), below)
    :
      zorder_.end();
  const auto ze{std::distance(zorder_.begin(), zorder_.insert(insertPos, element))};
  // Create own fragments from overlaying fragments when inserting it below the top
  cover(ze);
  // Create fragments of all elements overlayed by this
  for (auto z = ze - 1; z >= 0; --z) {
    auto * const elementBelow = zorder_[z];
    if (elementBelow->area().intersects(element->area())) {
      removeRtreeFragments(*elementBelow);
      jwezel::cover(*elementBelow, *element);
      insertRtreeFragments(*elementBelow);
    }
  }
  // TODO(j): Find a better way to test for the backdrop. Creating a device update
  // for the backdrop is not necessary and creates problems
  if (ze) {
    update(element->fragments_);
  }
  insertRtreeFragments(*element);
}

auto Surface::position(Element *element, int default_) -> int {
  if (!element) {
    if (default_ == -1) {
      return zorder_.size(); // NOLINT
    }
    return default_;
  }
  const auto zit{std::find(zorder_.begin(), zorder_.end(), element)};
  if (zit == zorder_.end()) {
    throw domain_error(format("Element {} not found in zorder\n", static_cast<const void*>(element)));
  }
  return std::distance(zorder_.begin(), zit); // NOLINT
}

void Surface::deleteElement(Element *element, Element *destination) {
  Updates result;
  vector<Fragment> updates;
  const auto ze{position(element)};
  const auto zb{position(destination, 0)};
  // Remove element from surface
  removeRtreeFragments(*element);
  zorder_.erase(zorder_.begin() + ze);
  // Recreate fragments covered by removed element
  for (auto z1 = ze - 1; z1 >= zb; --z1) {
    Element * const zelement = zorder_[z1];
    if (zelement->area().intersects(element->area())) {
      removeRtreeFragments(*zelement);
      cover(z1);
      insertRtreeFragments(*zelement);
      // Add surface updates for these fragments
      std::ranges::copy(
        zelement->fragments_ |
        transform([element](const Surface::Fragment &f) {return f.area & element->area();}) |
        filter([](const std::optional<Rectangle> &r) {return r.has_value();}) |
        transform([zelement](const std::optional<Rectangle> &r){return Surface::Fragment{r.value(), zelement};}),
        std::back_inserter(updates)
      );
    }
  }
  update(updates);
}

void Surface::reshapeElement(Element *element, const Rectangle &area) { //NOLINT(readability-function-cognitive-complexity)
  vector<Fragment> updates;
  if (element->area() != area) {
    const auto zpos = std::find(zorder_.begin(), zorder_.end(), element);
    if (zpos == zorder_.end()) {
      throw range_error(format("Element {} not found", static_cast<void*>(element)));
    }
    const auto ze = std::distance(zorder_.begin(), zpos);
    if (ze == 0) {
      throw logic_error("Lowest element must be the 'backdrop', which must not be moved");
    }
    auto damageAreas{element->area().defaultIntersection(area)};
    damageAreas.push_back(area);
    auto const searchArea = element->area() | area;
    element->moveEvent(area);
    // Recreate fragments
    for (auto j = ze; j >= 0; --j) {
      auto * const element_ = zorder_[j];
      if (element_->area().intersects(searchArea)) {
        removeRtreeFragments(*element_);
        // Create fragments from elements on top of it
        cover(j);
        insertRtreeFragments(*element_);
        // Surface update fragments
        for (const auto &fragment: element_->fragments_) {
          for (const auto &damageArea: damageAreas) {
            auto const intersection = fragment.area & damageArea;
            if (intersection) {
              updates.emplace_back(intersection.value(), element_);
            }
          }
        }
      }
    }
  }
  update(updates);
}

void Surface::reorder(int source, int destination) {
  vector<Fragment> updates;
  if (destination < source) {
    // Move down
    auto *element{zorder_[source]};
    std::shift_right(zorder_.begin() + destination, zorder_.begin() + source + 1, 1);
    zorder_[destination] = element;
    auto const searchArea = zorder_[destination]->area();
    // Recreate fragments
    for (auto j = source; j >= destination; --j) {
      auto * const element_ = zorder_[j];
      auto elementIntersection{element_->area() & searchArea};
      if (elementIntersection) {
        // Create fragments from elements on top of it
        removeRtreeFragments(*element_);
        cover(j);
        insertRtreeFragments(*element_);
        if (element_ != zorder_[destination]) {
          // Surface update fragments
          for (const auto &fragment: element_->fragments_) {
            auto const intersection = fragment.area & elementIntersection.value();
            if (intersection) {
              updates.emplace_back(intersection.value(), element_);
            }
          }
        }
      }
    }
  } else {
    // Move up
    auto *element{zorder_[source]};
    std::shift_left(zorder_.begin() + source, zorder_.begin() + destination--, 1);
    zorder_[destination] = element;
    auto oldFragments = element->fragments_;
    auto const searchArea = zorder_[destination]->area();
    for (auto j = destination; j >= source; --j) {
      auto * const element_ = zorder_[j];
      auto elementIntersection{element_->area() & searchArea};
      if (elementIntersection) {
        // Create fragments from elements on top of it
        removeRtreeFragments(*element_);
        cover(j);
        insertRtreeFragments(*element_);
        if (element_ != zorder_[destination]) {
          // Surface update fragments
          for (const auto &fragment: element_->fragments_) {
            auto const intersection = fragment.area & elementIntersection.value();
            if (intersection) {
              updates.emplace_back(intersection.value(), element_);
            }
          }
        }
      }
    }
    // Surface updates
    updates = element->fragments_;
    split(element->area(), updates, oldFragments);
  }
  update(updates);
}

void Surface::above(Surface::Element * element, Surface::Element * destination) {
  if (element != destination) {
    reorder(position(element), position(destination) + 1);
  }
}

void Surface::below(Surface::Element * element, Surface::Element * destination) {
  if (element != destination) {
    reorder(position(element), position(destination));
  }
}

auto Surface::minSize(const Element *exclude) const -> Vector {
  Vector result{0, 0};
  bool consider{false};
  for (auto *const element: zorder_) {
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

//~TextElement~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TextElement::TextElement(Surface *surface, const Rectangle &area, const Char &background, TextElement *below):
Element{surface},
position_(area.position()),
background_(background),
text_{background, area.size()}
{
  surface->addElement(this, below);
}

TextElement::~TextElement() {
  try {
    surface()->deleteElement(this);
  } catch (std::exception & error) {
    std::cerr << error.what() << '\n';
  }
}

TextElement::operator string() const {
  return format("TextElement(id={}, area={})", long(this), string(area()));
}

auto TextElement::write(const Vector &position, const string_view &str) -> TextElement & {
  return write(position, Text(str, RgbNone, RgbNone, {}, AttributeMode::mix));
}

auto TextElement::write(const Vector &position, const Text &txt_) -> TextElement & {
  text_.patch(txt_, position);
  update({Rectangle{position, position + txt_.size()}});
  return *this;
}

auto TextElement::fill(const Char &fillChar, const Rectangle &area) -> TextElement & {
  update({text_.fill(fillChar, area)});
  return *this;
}

auto TextElement::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> TextElement & {
  update({text_.line(line, strength, dash, roundedCorners)});
  return *this;
}

auto TextElement::box(const Box &box) -> TextElement & {
  update(text_.box(box));
  return *this;
}

auto TextElement::moveEvent(const Rectangle &area) -> bool {
  position(area.position());
  text_.resize(area.size(), background());
  return true;
}

auto TextElement::above(TextElement *element) -> bool {
  surface()->above(this, element? element: surface()->zorder()[surface()->position(this) + 1]);
  return true;
}

auto TextElement::below(TextElement *element) -> bool {
  surface()->below(this, element? element: surface()->zorder()[surface()->position(this) - 1]);
  return true;
}

auto TextElement::above(int position) -> bool {
  surface()->above(this, surface()->zorder()[position < 0? position + surface()->zorder().size(): position]);
  return true;
}

auto TextElement::below(int position) -> bool {
  surface()->below(this, surface()->zorder()[position < 0? position + surface()->zorder().size(): position]);
  return true;
}

void TextElement::position(const Vector &position) {
  position_ = position;
}

auto TextElement::area() const -> Rectangle {
  return Rectangle{position(), position() + text_.size()};
}

auto TextElement::text(const Rectangle &area) const -> Text {
  return text_[area];
}

} // namespace jwezel
