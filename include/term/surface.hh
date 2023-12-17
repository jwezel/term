#pragma once

#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <initializer_list>

namespace jwezel {

using std::initializer_list;

struct Surface {
  struct Element {
    explicit Element(Surface *surface, const Rectangle &area);

    Element() = delete;

    virtual ~Element() = default;

    Element(const Element &) = default;

    Element(Element &&) = default;

    auto operator=(const Element &) -> Element & = default;

    auto operator=(Element &&) -> Element & = delete;

    ///
    /// Get text rectangle
    ///
    /// @param[in]  area  The area
    ///
    /// @return     Text of the specified rectangle
    [[nodiscard]] virtual auto text(const Rectangle &area=RectangleMax) const -> Text = 0;

    ///
    /// Get area
    ///
    /// @return     The area of the element
    [[nodiscard]] virtual auto area() const -> Rectangle = 0;

    ///
    /// Move element
    ///
    /// @param[in]  area  The area
    virtual bool moveEvent(const Rectangle &area) = 0;

    virtual void update(const vector<Rectangle> &areas);

    vector<Rectangle> fragments; // NOLINT
    Surface *surface_{0}; // NOLINT
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

  explicit Surface(Device *device, initializer_list<Element *> ={});

  void addElement(Element * element, Element *below=0);

  void deleteElement(Element *element);

  void reshapeElement(Element *element, const Rectangle &area);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  auto minSize(const struct Element *exclude=0) const -> Vector;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  vector<Element *> zorder;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
  private:
  Device *device_{};
};

template<class Range>
auto SurfaceUpdates(const Range &fragments) -> Updates;

} //namespace jwezel
