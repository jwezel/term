#pragma once

#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <initializer_list>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <memory>

BOOST_GEOMETRY_REGISTER_POINT_2D(jwezel::Vector, jwezel::Dim, cs::cartesian, x(), y());
BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(jwezel::Rectangle, jwezel::Vector, x1(), y1(), x2(), y2());

namespace jwezel {

using std::initializer_list;

namespace impl
{
struct Surface;
}

struct Element;

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

  explicit Element(struct Surface &surface, const Rectangle &area);

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

  vector<Fragment> fragments;
  struct impl::Surface *surface_{0};
};

namespace impl
{
struct Surface {
  Surface() = default;

  void removeRtreeFragments(Element &element);

  void insertRtreeFragments(Element &element);

  explicit Surface(Device *device, initializer_list<Element *> elements={});

  void addElement(Element * element, Element *below=0);

  void deleteElement(Element *element, Element *destination=0);

  void reshapeElement(Element *element, const Rectangle &area);

  void above(Element * element, Element * destination=0);

  void below(Element * element, Element * destination=0);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  [[nodiscard]] auto minSize(const Element *exclude=0) const -> Vector;

  [[nodiscard]] auto find(const Vector &position) const -> Fragment *;

  [[nodiscard]] auto position(Element *element, int default_=-1) -> int;

  [[nodiscard]] auto device() const -> auto {return device_;}

  [[nodiscard]] auto zorder() const -> auto {return zorder_;}

  private:
  ///
  /// Move element at @c source to @c destination
  ///
  /// @param[in]  source       The source
  /// @param[in]  destination  The destination
  ///
  void reorder(int source, int destination);

  ///
  /// Create fragments for element at position @c pos from all covering elements
  ///
  /// @param[in]  pos   The position
  ///
  void cover(int pos);

  ///
  /// { function_description }
  ///
  /// @param[in]  begin  The begin
  /// @param[in]  end    The end
  ///
  void uncover(int begin, int end);

  typedef std::pair<Rectangle, Fragment *> RtreeEntry;

  vector<Element *> zorder_;
  Device *device_{};
  boost::geometry::index::rtree<RtreeEntry, boost::geometry::index::quadratic<16>> rtree_;
  friend struct jwezel::Element;
};

template<class Range>
auto SurfaceUpdates(const Range &fragments) -> Updates;

} // namespace impl

struct Surface {
  Surface() = default;

  explicit Surface(Device *device, initializer_list<Element *> elements={}):
  p_{new impl::Surface{device, elements}}
  {}

  ///
  /// Adds an element.
  ///
  /// @param      element  The element
  /// @param      below    The below
  ///
  void addElement(Element * element, Element *below=0) {p_->addElement(element, below);}

  ///
  /// Removes an element
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void deleteElement(Element *element, Element *destination=0) {p_->deleteElement(element, destination);}

  ///
  /// { function_description }
  ///
  /// @param      element  The element
  /// @param[in]  area     The area
  ///
  void reshapeElement(Element *element, const Rectangle &area) {p_->reshapeElement(element, area);}

  ///
  /// { function_description }
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void above(Element * element, Element * destination=0) {p_->above(element, destination);}

  ///
  /// { function_description }
  ///
  /// @param      element      The element
  /// @param      destination  The destination
  ///
  void below(Element * element, Element * destination=0) {p_->below(element, destination);}

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  [[nodiscard]] auto minSize(const Element *exclude=0) const -> Vector {return p_->minSize(exclude);}

  [[nodiscard]] auto find(const Vector &position) const -> Fragment * {return p_->find(position);}

  [[nodiscard]] int position(Element *element, int default_=-1) {return p_->position(element, default_);}

  [[nodiscard]] auto device() const -> auto {return p_->device();}

  [[nodiscard]] auto zorder() const -> auto {return p_->zorder();}

  [[nodiscard]] auto operator &() const -> auto {return p_.get();}

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

  typedef std::pair<Rectangle, Fragment *> RtreeEntry;

  std::shared_ptr<impl::Surface> p_;
};

} //namespace jwezel
