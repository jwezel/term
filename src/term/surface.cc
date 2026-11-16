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
  if (fragments1.empty()) {
    return;
  }
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
      fragments1.swap(fragments);
      if (fragments1.empty()) {
        return;
      }
    }
  }
}

void cover(Surface::Element &element1, const Surface::Element &element2) {
  split(element1.area(), element1.fragments(), element2.fragments());
}

///
/// Cut an area out of fragments; what remains keeps its order
///
/// @param      fragments  The fragments
/// @param[in]  area       The area
void subtract(vector<Surface::Fragment> &fragments, const Rectangle &area) {
  const auto first{
    std::ranges::find_if(fragments, [&area](const Surface::Fragment &f) {return f.area.intersects(area);})
  };
  if (first == fragments.end()) {
    return;
  }
  vector<Surface::Fragment> result(fragments.begin(), first);
  for (auto fragment = first; fragment != fragments.end(); ++fragment) {
    if (fragment->area.intersects(area)) {
      for (const auto &part: fragment->area.defaultIntersection(area)) {
        result.emplace_back(part, fragment->element);
      }
    } else {
      result.push_back(*fragment);
    }
  }
  fragments.swap(result);
}

///
/// Add an area to fragments, joined with the fragments that it adjoins exactly
///
/// Without joining, the fragments would only ever split further when elements come and go.
///
/// @param      fragments  The fragments
/// @param[in]  area       The area, which must not overlap the fragments
/// @param      element    The element the fragments belong to
void join(vector<Surface::Fragment> &fragments, const Rectangle &area, Surface::Element *element) {
  auto joined{area};
  for (auto again{true}; again;) {
    again = false;
    for (auto fragment = fragments.begin(); fragment != fragments.end(); ++fragment) {
      if (const auto result{fragment->area.joinedWith(joined)}; result) {
        joined = result.value();
        fragments.erase(fragment);
        again = true;
        break;
      }
    }
  }
  fragments.emplace_back(joined, element);
}

auto areasOf(const vector<Surface::Fragment> &fragments) -> vector<Rectangle> {
  vector<Rectangle> result;
  result.reserve(fragments.size());
  for (const auto &fragment: fragments) {
    result.push_back(fragment.area);
  }
  return result;
}

auto contains(const Rectangle &area, const Vector &position) -> bool {
  return position.x() >= area.x1() and position.x() < area.x2() and position.y() >= area.y1() and position.y() < area.y2();
}

} // namespace

template<class Range>
auto SurfaceUpdates(const Range &fragments) -> Updates {
  Updates result;
  result.reserve(static_cast<size_t>(std::ranges::distance(fragments)));
  for (const auto &fragment: fragments) {
    const auto &area = fragment.area;
    result.emplace_back(
      Vector(area.x1(), area.y1()),
      fragment.element->text(area - fragment.element->area().position())
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

void Surface::cover(long pos) {
  auto *element{zorder_[pos]};
  element->fragments_ = {Surface::Fragment{element->area(), element}};
  for (unsigned j = pos + 1; j < zorder_.size(); ++j) {
    jwezel::cover(*element, *zorder_[j]);
  }
  element->canonicalFragments_ = element->fragments_.size();
}

void Surface::tidy(long from, long to) {
  const size_t margin{4};
  for (auto z = std::max(from, 0L); z < std::min(to, static_cast<long>(zorder_.size())); ++z) {
    if (zorder_[z]->fragments_.size() > 2 * zorder_[z]->canonicalFragments_ + margin) {
      cover(z);
    }
  }
}

void Surface::expose(vector<Rectangle> regions, long above, long lowest, vector<Fragment> &updates) {
  for (auto z = above - 1; z >= lowest and !regions.empty(); --z) {
    auto *element{zorder_[z]};
    const auto area{element->area()};
    vector<Rectangle> rest;
    for (const auto &region: regions) {
      if (const auto part{region & area}; part) {
        join(element->fragments_, part.value(), element);
        updates.emplace_back(part.value(), element);
      }
      std::ranges::copy(region.defaultIntersection(area), std::back_inserter(rest));
    }
    regions.swap(rest);
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
  // The element shows what is not covered by elements above it
  cover(ze);
  // and covers the elements below it
  const auto area{element->area()};
  for (auto z = ze - 1; z >= 0; --z) {
    auto * const elementBelow = zorder_[z];
    if (elementBelow->area().intersects(area)) {
      subtract(elementBelow->fragments_, area);
    }
  }
  // TODO(j): Find a better way to test for the backdrop. Creating a device update
  // for the backdrop is not necessary and creates problems
  if (ze) {
    update(element->fragments_);
  }
  tidy(0, ze);
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
  vector<Fragment> updates;
  const auto ze{position(element)};
  const auto zb{position(destination, 0)};
  // What the element showed is shown by the elements below it now
  auto regions{areasOf(element->fragments_)};
  zorder_.erase(zorder_.begin() + ze);
  element->fragments_.clear();
  expose(std::move(regions), ze, zb, updates);
  tidy(zb, ze);
  update(updates);
}

void Surface::reshapeElement(Element *element, const Rectangle &area) {
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
    const auto previous{areasOf(element->fragments_)};
    element->moveEvent(area);
    const auto newArea{element->area()};
    cover(ze);
    // The element covers the elements below it in its new area
    for (auto z = ze - 1; z >= 0; --z) {
      auto * const elementBelow = zorder_[z];
      if (elementBelow->area().intersects(newArea)) {
        subtract(elementBelow->fragments_, newArea);
      }
    }
    // and uncovers what it showed before and does not show now
    vector<Rectangle> regions;
    for (const auto &region: previous) {
      std::ranges::copy(region.defaultIntersection(newArea), std::back_inserter(regions));
    }
    updates = element->fragments_;
    expose(std::move(regions), ze, 0, updates);
    tidy(0, ze);
  }
  update(updates);
}

void Surface::reorder(int source, int destination) {
  if (source <= 0) {
    throw logic_error("Lowest element must be the 'backdrop', which must not be moved");
  }
  // Nothing goes below the backdrop, nor beyond the top
  destination = std::clamp(destination, 1, static_cast<int>(zorder_.size()));
  if (destination == source or destination == source + 1) { // the element stays where it is
    return;
  }
  vector<Fragment> updates;
  auto *element{zorder_[source]};
  const auto area{element->area()};
  if (destination < source) {
    // Move down: the elements it is moved below show what it showed in their areas
    expose(areasOf(element->fragments_), source, destination, updates);
    for (auto z = destination; z < source; ++z) {
      subtract(element->fragments_, zorder_[z]->area());
    }
    std::shift_right(zorder_.begin() + destination, zorder_.begin() + source + 1, 1);
    zorder_[destination] = element;
    tidy(destination, source + 1);
  } else {
    // Move up: the element shows what the elements it is moved above showed in its area
    for (auto z = source + 1; z < destination; ++z) {
      auto *elementPassed{zorder_[z]};
      vector<Rectangle> gained;
      for (const auto &fragment: elementPassed->fragments_) {
        if (const auto part{fragment.area & area}; part) {
          gained.push_back(part.value());
        }
      }
      if (!gained.empty()) {
        for (const auto &part: gained) {
          join(element->fragments_, part, element);
          updates.emplace_back(part, element);
        }
        subtract(elementPassed->fragments_, area);
      }
    }
    std::shift_left(zorder_.begin() + source, zorder_.begin() + destination, 1);
    zorder_[destination - 1] = element;
    tidy(source, destination);
  }
  update(updates);
}

void Surface::above(Surface::Element * element, Surface::Element * destination) {
  if (element != destination) {
    reorder(position(element), destination? position(destination) + 1: static_cast<int>(zorder_.size()));
  }
}

void Surface::below(Surface::Element * element, Surface::Element * destination) {
  if (element != destination) {
    reorder(position(element), destination? position(destination): static_cast<int>(zorder_.size()));
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
  // The element that is visible at the position is the topmost that has it in its area
  for (auto element = zorder_.rbegin(); element != zorder_.rend(); ++element) {
    if (contains((*element)->area(), position)) {
      for (auto &fragment: (*element)->fragments_) {
        if (contains(fragment.area, position)) {
          return &fragment;
        }
      }
      return nullptr;
    }
  }
  return nullptr;
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
  Element *destination{element};
  if (!destination) { // one up
    const auto next{static_cast<size_t>(surface()->position(this)) + 1};
    if (next >= surface()->zorder().size()) {
      return false; // already on top
    }
    destination = surface()->zorder()[next];
  }
  surface()->above(this, destination);
  return true;
}

auto TextElement::below(TextElement *element) -> bool {
  Element *destination{element};
  if (!destination) { // one down
    const auto previous{surface()->position(this) - 1};
    if (previous < 1) {
      return false; // already at the bottom, the backdrop is below
    }
    destination = surface()->zorder()[static_cast<size_t>(previous)];
  }
  surface()->below(this, destination);
  return true;
}

namespace {

// Get the element at a z-order position; negative positions count from the top
auto elementAt(const Surface &surface, int position) -> Surface::Element * {
  const auto size{static_cast<int>(surface.zorder().size())};
  const auto index{position < 0? position + size: position};
  if (index < 0 or index >= size) {
    throw std::out_of_range(format("z-order position {} is outside of 0 .. {}", position, size - 1));
  }
  return surface.zorder()[static_cast<size_t>(index)];
}

} // namespace

auto TextElement::above(int position) -> bool {
  surface()->above(this, elementAt(*surface(), position));
  return true;
}

auto TextElement::below(int position) -> bool {
  surface()->below(this, elementAt(*surface(), position));
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
