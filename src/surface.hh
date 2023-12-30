#pragma once

#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <initializer_list>
#include <memory>

BOOST_GEOMETRY_REGISTER_POINT_2D(jwezel::Vector, jwezel::Dim, cs::cartesian, x(), y());
BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(jwezel::Rectangle, jwezel::Vector, x1(), y1(), x2(), y2());
auto const MAX_RTREE_NODE_SIZE{16};
auto const MIN_RTREE_NODE_SIZE{4};

namespace jwezel {

using std::initializer_list;

struct Surface {
  struct Element;

  struct Fragment {
    explicit Fragment(const Rectangle &area, struct Element *element):
    area(area),
    element(element)
    {}

    explicit operator string() const;

    auto operator ==(const Surface::Fragment &other) const -> bool = default;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Rectangle area;
    struct Element *element{};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
  };

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
    virtual auto moveEvent(const Rectangle &area) -> bool = 0;

    virtual void update(const vector<Rectangle> &areas);

    [[nodiscard]] auto fragments() -> auto & {return fragments_;}

    [[nodiscard]] auto fragments() const -> const auto & {return fragments_;}

    [[nodiscard]] auto surface() const -> auto *{return surface_;}

    private:
    vector<Fragment> fragments_;
    Surface *surface_{0};
    friend struct Surface;
  };

  explicit Surface(Device *device, initializer_list<Element *> elements={});

  Surface() = default;

  void removeRtreeFragments(Surface::Element &element);

  void insertRtreeFragments(Surface::Element &element);

  void addElement(Element * element, Element *below=0);

  void deleteElement(Element *element, Element *destination=0);

  void reshapeElement(Element *element, const Rectangle &area);

  void above(Surface::Element * element, Surface::Element * destination=0);

  void below(Surface::Element * element, Surface::Element * destination=0);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  auto minSize(const Element *exclude=0) const -> Vector;

  [[nodiscard]] auto find(const Vector &position) const -> Fragment *;

  auto position(Element *element, int default_=-1) -> int;

  [[nodiscard]] auto zorder() const -> const auto & {return zorder_;}

  struct ref
  {
    explicit ref(Device *device, initializer_list<Element *> elements={}):
    ptr_{new Surface{device, elements}}
    {}

    ref() = default;

    void removeRtreeFragments(Surface::Element &element) {ptr_->removeRtreeFragments(element);}

    void insertRtreeFragments(Surface::Element &element) {ptr_->insertRtreeFragments(element);}

    void addElement(Element * element, Element *below=0) {ptr_->addElement(element, below);}

    void deleteElement(Element *element, Element *destination=0) {ptr_->deleteElement(element, destination);}

    void reshapeElement(Element *element, const Rectangle &area) {ptr_->reshapeElement(element, area);}

    void above(Surface::Element * element, Surface::Element *destination=0) {ptr_->above(element, destination);}

    void below(Surface::Element * element, Surface::Element *destination=0) {ptr_->below(element, destination);}

    ///
    /// Get minimum size to accomodate all elements
    ///
    /// @param      exclude  Last element to exclude
    ///
    /// @return     Minimum size
    [[nodiscard]] auto minSize(const Element *exclude=0) const -> Vector {return ptr_->minSize(exclude);}

    [[nodiscard]] auto find(const Vector &position) const -> Fragment * {return ptr_->find(position);}

    [[nodiscard]] auto position(Element *element, int default_=-1) -> int {return ptr_->position(element, default_);}

    [[nodiscard]] auto zorder() const -> const auto & {return ptr_->zorder();}

    [[nodiscard]] auto ptr() -> auto {return ptr_.get();}

    private:
    std::shared_ptr<Surface> ptr_;
  };

  private:
  void reorder(int source, int destination);

  void cover(int pos);

  void uncover(int begin, int end);

  using RtreeEntry = std::pair<Rectangle, Surface::Fragment *>;

  vector<Element *> zorder_;
  Device *device_{};
  boost::geometry::index::rtree<RtreeEntry, boost::geometry::index::quadratic<MAX_RTREE_NODE_SIZE, MIN_RTREE_NODE_SIZE>> rtree;
};

template<class Range>
auto SurfaceUpdates(const Range &fragments) -> Updates;

} //namespace jwezel
