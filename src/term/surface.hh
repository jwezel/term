#pragma once

#include <iostream>
#include <term/device.hh>
#include <term/geometry.hh>
#include <term/text.hh>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <initializer_list>

// NOLINTBEGIN
BOOST_GEOMETRY_REGISTER_POINT_2D(jwezel::Vector, jwezel::Dim, cs::cartesian, x(), y());
BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(jwezel::Rectangle, jwezel::Vector, x1(), y1(), x2(), y2());
// NOLINTEND

const auto maxRtreeElements = 16;
const auto minRtreeElements = 4;

namespace jwezel {

using std::initializer_list, std::pair;

struct Surface {
  struct Element;

  struct Fragment {
    explicit Fragment(const Rectangle &area, struct Element *element):
    area(area),
    element(element)
    {}

    explicit operator string() const;

    auto operator ==(const Surface::Fragment &other) const -> bool = default;

    // xNOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Rectangle area;
    struct Element *element{};
    // xNOLINTEND(misc-non-private-member-variables-in-classes)
  };

  ///
  /// Abstract Element
  ///
  /// The following method are to be overridden:
  ///
  /// - area
  struct Element {
    explicit Element(Surface *surface);

    Element() = default;

    virtual ~Element();

    Element(const Element &) = default;

    Element(Element &&) = default;

    auto operator=(const Element &) -> Element & = default;

    auto operator=(Element &&) -> Element & = default;

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

    virtual void move(const Rectangle &area);

    ///
    /// Move element
    ///
    /// @param[in]  area  The area
    virtual auto moveEvent(const Rectangle &/*area*/) -> bool {return false;}

    virtual void update(const vector<Rectangle> &areas);

    virtual void update(const Rectangle &area_);

    [[nodiscard]] auto fragments() -> auto & {return fragments_;}

    [[nodiscard]] auto fragments() const -> const auto & {return fragments_;}

    [[nodiscard]] auto surface() const -> auto *{return surface_;}

    private:
    vector<Fragment> fragments_;
    Surface *surface_{0};
    friend struct Surface;
  };

  explicit Surface(Device *device, initializer_list<Element *> ={});

  Surface() = default;

  virtual ~Surface() = default;

  void update(const vector<Fragment> &updates);

  void removeRtreeFragments(Surface::Element &element);

  void insertRtreeFragments(Surface::Element &element);

  virtual void addElement(Element *element, Element *below=0);

  virtual void deleteElement(Element *element, Element *destination=0);

  virtual void reshapeElement(Element *element, const Rectangle &area);

  void above(Surface::Element *element, Surface::Element *destination=0);

  void below(Surface::Element *element, Surface::Element *destination=0);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  auto minSize(const Element *exclude=0) const -> Vector;

  [[nodiscard]] auto find(const Vector &position) const -> Fragment *;

  [[nodiscard]] auto position(Element *element, int default_=-1) -> int;

  [[nodiscard]] inline auto zorder() const -> const auto & {return zorder_;}

  private:
  void reorder(int source, int destination);

  void cover(long pos);

  using RtreeEntry = pair<Rectangle, Fragment *>;

  vector<Element *> zorder_;
  Device *device_{};
  boost::geometry::index::rtree<RtreeEntry, boost::geometry::index::quadratic<maxRtreeElements, minRtreeElements>> rtree;
};

struct TextElement: Surface::Element {
  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  explicit TextElement(Surface *surface, const Rectangle &area, const Char &background=Space, TextElement *below=0);

  TextElement() = default;

  TextElement(const TextElement &) = default;

  TextElement(TextElement &&) = default;

  auto operator=(const TextElement &) -> TextElement & = default;

  auto operator=(TextElement &&) -> TextElement & = default;

  ///
  /// Destroy TextElement
  ~TextElement() override;

  /// String representation of TextElement
  explicit operator string() const;

  ///
  /// Write string to element
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  auto write(const Vector &position, const string_view &str) -> TextElement &;

  ///
  /// Write text to text element
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  auto write(const Vector &position, const Text &txt_) -> TextElement &;

  ///
  /// Fill text element with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  auto fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) -> TextElement &;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> TextElement &;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  auto box(const Box &box = Box{}) -> TextElement &;

  auto moveEvent(const Rectangle &/*area*/) -> bool override;

  auto above(TextElement *element=0) -> bool;

  auto below(TextElement *element=0) -> bool;

  auto above(int position=0) -> bool;

  auto below(int position=-1) -> bool;

  [[nodiscard]] inline auto position() const {return position_;}

  void position(const Vector &position);

  [[nodiscard]] inline auto background() const {return background_;}

  ///
  /// Get text element area
  ///
  /// @return     TextElement area
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of text element text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area = RectangleMax) const -> Text override;

  private:
  Vector position_;
  Char background_;
  Text text_;
};

} //namespace jwezel
