#pragma once

#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <initializer_list>
#include <memory>
#include <unordered_map>

namespace jwezel {

using std::initializer_list, std::unique_ptr, std::unordered_map;

struct Surface {
  struct Element {
    virtual ~Element() = default;

    Element(const Element &) = default;

    Element(Element &&) = delete;

    auto operator=(const Element &) -> Element & = default;

    auto operator=(Element &&) -> Element & = delete;

    explicit Element(const Rectangle &area) : fragments{area} {}

    [[nodiscard]] virtual auto text(const Rectangle &area) const -> Text = 0;

    [[nodiscard]] virtual auto area() const -> Rectangle = 0;

    virtual void move(const Rectangle &area) = 0;

    // NOLINTNEXTLINE
    vector<Rectangle> fragments;
  };

  struct Fragment {
    explicit Fragment(Rectangle area, Element *element) : area(area),element(element) {}

    explicit operator string() const;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Rectangle area;
    Element *element{};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
  };

  Surface() = default;

  Surface(initializer_list<Element *>);

  auto addElement(Surface::Element * element, Surface::Element *below) -> Updates;

  auto deleteElement(Element *element) -> Updates;

  auto reshapeElement(Element *element, const Rectangle &area) -> Updates;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  vector<Element *> zorder;
  unordered_map<Element *, unique_ptr<Element>> elements{};
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

auto SurfaceUpdates(const vector<Surface::Fragment> &fragments) -> Updates;

} //namespace jwezel
