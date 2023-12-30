#pragma once

#include "device.hh"
#include "geometry.hh"
#include "text.hh"

#include <initializer_list>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <memory>

BOOST_GEOMETRY_REGISTER_POINT_2D(jwezel::Vector, jwezel::Dim, cs::cartesian, x(), y());
BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(jwezel::Rectangle, jwezel::Vector, x1(), y1(), x2(), y2());
constexpr auto MAX_RTREE_ELEMENTS{16};
constexpr auto MIN_RTREE_ELEMENTS{4};

namespace jwezel {

using std::initializer_list;

namespace impl
{
  struct Surface;
}

struct Surface {
  explicit Surface(Device *device, initializer_list<struct Element *> elements={});

  Surface() = default;

  Surface(const Surface &) = default;

  Surface(Surface &&) = default;

  auto operator=(const Surface &) -> Surface & = default;

  auto operator=(Surface &&) -> Surface & = default;

  ~Surface() = default;

  ///
  /// Adds an element.
  ///
  /// @param      element  The element
  /// @param      below    The below
  ///
  void addElement(struct Element * element, struct Element *below=0);

  ///
  /// Removes an element
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void deleteElement(struct Element *element, struct Element *destination=0);

  ///
  /// { function_description }
  ///
  /// @param      element  The element
  /// @param[in]  area     The area
  ///
  void reshapeElement(struct Element *element, const Rectangle &area);

  ///
  /// { function_description }
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void above(struct Element * element, struct Element * destination=0);

  ///
  /// { function_description }
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void below(struct Element * element, struct Element * destination=0);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  [[nodiscard]] auto minSize(const struct Element *exclude=0) const -> Vector;

  [[nodiscard]] auto find(const Vector &position) const -> struct Fragment *;

  [[nodiscard]] auto position(struct Element *element, int default_=-1) -> int;

  [[nodiscard]] auto device() const -> Device *;

  [[nodiscard]] auto zorder() const -> vector<Element *> &;

  [[nodiscard]] auto ptr() const -> impl::Surface *;

  [[nodiscard]] auto ref() const -> impl::Surface &;

  private:
  ///
  /// { function_description }
  ///
  /// @param[in]  source       The source
  /// @param[in]  destination  The destination
  ///
  void reorder(int source, int destination);

  ///
  /// { function_description }
  ///
  /// @param[in]  pos   The position
  ///
  void cover(int pos);

  std::shared_ptr<impl::Surface> p_;
};


struct Fragment {
  explicit Fragment(const Rectangle &area, struct Element *element):
  area(area),
  element(element)
  {}

  explicit operator string() const;

  bool operator ==(const struct Fragment &other) const = default;

  Rectangle area;
  struct Element *element{};
};

struct Element {
  explicit Element(struct impl::Surface *surface, const Rectangle &area);

  explicit Element(const struct Surface &surface, const Rectangle &area);

  Element() = delete;

  virtual ~Element() = default;

  Element(const Element &) = default;

  Element(Element &&) = default;

  ///
  /// Assignment operator
  ///
  /// @param[in]  <unnamed>  { parameter_description }
  ///
  /// @return     The result of the assignment
  ///
  auto operator=(const Element &) -> Element & = default;

  ///
  /// Assignment operator
  ///
  /// @param      <unnamed>  { parameter_description }
  ///
  /// @return     The result of the assignment
  ///
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
  ///
  /// @return     { description_of_the_return_value }
  ///
  virtual bool moveEvent(const Rectangle &area) = 0;

  /// Updates the given areas
  ///
  /// @param[in]  areas  The areas
  virtual void update(const vector<Rectangle> &areas);

  bool above(Element *element);

  bool above(int position=-1);

  bool below(Element *element);

  bool below(int position=0);

  vector<Fragment> fragments_;
  struct jwezel::Surface surface_;
};

} //namespace jwezel
