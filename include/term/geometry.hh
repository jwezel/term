///
/// @defgroup   GEOMETRY geometry
///
/// This file defines Vector and Rectangle types.
///
/// @author     J
/// @date       2020-12-27

#pragma once

// Geometric shapes
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace jwezel {

using std::string, std::logic_error, std::vector, std::optional, std::range_error;
using std::format;

/// Dimension (x, y) in a @class Vector.
using Dim = int16_t;

extern const Dim DimLow;  ///< Lowest possible value of a dimension
extern const Dim DimHigh; ///< Highest possible value of a dimension

class RectangleError: public logic_error
{
  public:
  explicit RectangleError(const string &what): logic_error(what) {}
};

template<typename T>
inline auto toDim(T v)->Dim {
#ifndef NDEBUG
  if (static_cast<Dim>(v) > DimHigh || static_cast<Dim>(v) < DimLow) {
    throw range_error(format("Dim {} is out of range", v));
  }
#endif
  return static_cast<Dim>(v);
}

///
/// A vector
struct Vector {
  /// Constructor
  ///
  /// @param[in]  x
  /// @param[in]  y
  explicit Vector(Dim x=0, Dim y=0) noexcept;

  ///
  /// Convert Vector to string
  [[nodiscard]] explicit operator string() const;

  ///
  /// Convert Vector to string
  ///
  /// Alias for operator string()
  auto operator<<(std::ostream& os) const -> std::ostream&;

  /// Compare two vectors for equality.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True if equal
  [[nodiscard]] auto operator==(const Vector &other) const -> bool {
    return x_ == other.x() && y_ == other.y();
  }

  /// Compare two vectors for inequality.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True in not equal
  [[nodiscard]] auto operator!=(const Vector &other) const -> bool {
    return !(*this == other);
  }

  ///
  /// Compare vector less than other
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True if less
  [[nodiscard]] auto operator<(const Vector &other) const -> bool {
    return x_ < other.x() or x_ == other.x() and y_ < other.y();
  }

  ///
  /// Compare vector greater than other
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True if greater
  [[nodiscard]] auto operator>(const Vector &other) const -> bool {
    return other.x() < x_ or x_ == other.x() and other.y() < y_;
  }

  ///
  /// Compare vector less or equal than other
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True if less or equal
  [[nodiscard]] auto operator<=(const Vector &other) const -> bool {
    return x_ <= other.x() && y_ <= other.y();
  }

  ///
  /// Compare vector greater than equal than other
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     True if greater than equal
  [[nodiscard]] auto operator>=(const Vector &other) const -> bool {
    return x_ >= other.x() && y_ >= other.y();
  }

  ///
  /// Replace negative elements in @c *this with sum of element in @c *this and
  /// @arg position such that -1 becomes last.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     Vector with adjusted values
  [[nodiscard]] auto position(const Vector &position) const -> Vector;

  /// Apply defaults
  ///
  /// Substitutes all default values (DimLow and DimHigh values) with values from @arg default_
  ///
  /// @param[in]  default_  The default
  ///
  /// @return     Vector with defaults applied
  [[nodiscard]] auto defaultTo(const Vector &default_) const -> Vector;

  ///
  /// Addition operator.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     The result of the addition
  [[nodiscard]] auto operator +(const Vector &other) const -> Vector;

  ///
  /// Subtract vectors.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     The result of the subtraction
  [[nodiscard]] auto operator -(const Vector &other) const -> Vector;

  ///
  /// Enlarge Vector
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Enlarged Vector
  [[nodiscard]] auto operator +(Dim size) const -> Vector;

  ///
  /// Enlarge Vector destructively
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Enlarged Vector
  auto operator +=(Dim size) -> Vector &;

  ///
  /// Decrease Vector
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Decreased Vector
  [[nodiscard]] auto operator -(Dim size) const -> Vector;

  ///
  /// Decrease Vector destructively
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Decreased Vector
  auto operator-=(Dim size) -> Vector & {
    x_ = toDim(x_ - size);
    y_ = toDim(y_ - size);
    return *this;
  }

  ///
  /// Multiply Vector with factor
  ///
  /// @param[in]  factor  The factor
  ///
  /// @return     The result of the multiplication
  [[nodiscard]] auto operator *(double factor) const -> Vector;

  ///
  /// Divide Vector by divisor
  ///
  /// @param[in]  divisor  The divisor
  ///
  /// @return     The result of the division
  [[nodiscard]] auto operator /(double divisor) const -> Vector;

  ///
  /// Shift Vector left
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  [[nodiscard]] auto left(Dim shift=1) const -> Vector;

  ///
  /// Shift Vector right
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  [[nodiscard]] auto right(Dim shift=1) const -> Vector;

  ///
  /// Shift Vector up
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  [[nodiscard]] auto up(Dim shift=1) const -> Vector;

  ///
  /// Shift Vector down
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  [[nodiscard]] auto down(Dim shift=1) const -> Vector;

  ///
  /// Shift Vector left
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  auto left(Dim shift=1) -> Vector &;

  ///
  /// Shift Vector right
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  auto right(Dim shift=1) -> Vector &;

  ///
  /// Shift Vector up
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  auto up(Dim shift=1) -> Vector &;

  ///
  /// Shift Vector down
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  auto down(Dim shift=1) -> Vector &;

  ///
  /// Span of two Vectors.
  ///
  /// @param[in]  other  The other vector
  ///
  /// @return     Span of the Vectors
  auto operator |(const Vector &other) const -> Vector;

  [[nodiscard]] inline auto x() const -> Dim {return x_;}

  [[nodiscard]] inline auto y() const -> Dim {return y_;}

  [[nodiscard]] inline auto x() -> Dim & {return x_;}

  [[nodiscard]] inline auto y() -> Dim & {return y_;}

  private:
  Dim x_;
  Dim y_;
};

///
/// Minimum of two Vectors
///
/// @param[in]  v1    Vector 1
/// @param[in]  v2    Vector 2
///
/// @return     Minimum
auto min(const Vector &v1, const Vector &v2) -> Vector;

///
/// Maximum of two Vectors
///
/// @param[in]  v1    Vector 1
/// @param[in]  v2    Vector 2
///
/// @return     Maximum
auto max(const Vector &v1, const Vector &v2) -> Vector;

///
/// A rectangle
struct Rectangle {

  ///
  /// Constructor
  ///
  /// @param[in]  x1
  /// @param[in]  y1
  /// @param[in]  x2
  /// @param[in]  y2
  explicit Rectangle(Dim x1=0, Dim y1=0, Dim x2=0, Dim y2=0) noexcept;

  ///
  /// Constructor
  ///
  /// @param[in]  v1
  /// @param[in]  v2
  Rectangle(const Vector &v1, const Vector &v2) noexcept;

  ///
  /// String conversion operator.
  [[nodiscard]] explicit operator string() const;

  ///
  /// Write Vector to stream
  ///
  /// @param      os    The output stream
  ///
  /// @return     Reference to output stream
  friend auto operator <<(std::ostream& os, const Rectangle &rect) -> std::ostream&;

  ///
  /// Equality operator.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if equal
  [[nodiscard]] auto operator ==(const Rectangle &other) const -> bool;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if not equal
  [[nodiscard]] auto operator !=(const Rectangle &other) const -> bool;

  ///
  /// Rectangle intersection.
  ///
  /// If @arg other intersects, returns intersection
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Intersecting rectangle [optional]
  [[nodiscard]] auto operator &(const Rectangle &other) const -> optional<Rectangle>;

  ///
  /// Test for intersection.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if @a other intersects
  [[nodiscard]] auto intersects(const Rectangle &other) const -> bool;

  ///
  /// Shift rectangle by vector.
  ///
  /// @param[in]  v     The vector
  ///
  /// @return     Shifted rectangle
  [[nodiscard]] auto operator +(const Vector &v) const -> Rectangle;

  ///
  /// Get area spanning two rectangles.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Rectangle spanning @c *this and @arg other
  [[nodiscard]] auto operator |(const Rectangle &other) const -> Rectangle;

  ///
  /// Assign spanning rectangle.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Rectangle spanning @c *this and
  /// @arg        other
  auto operator |=(const Rectangle &other) -> const Rectangle &;

  ///
  /// Move rectangle by vector.
  ///
  /// @param[in]  vector  The vector
  ///
  /// @return     Moved rectangle
  [[nodiscard]] auto operator -(const Vector &vector) const -> Rectangle;

  ///
  /// Get vector of rectangle parts from clipping two rectangles.
  ///
  /// The vector contains all parts except the intersection. If the two rectangles do not intersect,
  /// or if the intersection is @c *this, an empty vector is returned.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Vector of parts
  [[nodiscard]] auto operator -(const struct Rectangle &other) const -> vector<Rectangle>;

  ///
  /// Shift Rectangle left
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  [[nodiscard]] auto left(Dim shift=1) const -> Rectangle;

  ///
  /// Shift Rectangle right
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  [[nodiscard]] auto right(Dim shift=1) const -> Rectangle;

  ///
  /// Shift Rectangle up
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  [[nodiscard]] auto up(Dim shift=1) const -> Rectangle;

  ///
  /// Shift Rectangle down
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  [[nodiscard]] auto down(Dim shift=1) const -> Rectangle;

  ///
  /// Shift Rectangle left
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  auto left(Dim shift=1) -> Rectangle &;

  ///
  /// Shift Rectangle right
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  auto right(Dim shift=1) -> Rectangle &;

  ///
  /// Shift Rectangle up
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  auto up(Dim shift=1) -> Rectangle &;

  ///
  /// Shift Rectangle down
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Rectangle
  auto down(Dim shift=1) -> Rectangle &;

  ///
  /// Get vector of rectangle parts from clipping two rectangles.
  ///
  /// The vector contains all parts except the intersection. If the two rectangles do not intersect,
  /// the result vector contains only @c *this. If the intersection is @c *this, an empty vector is returned.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     std::vector of parts
  [[nodiscard]] auto defaultIntersection(const Rectangle &other) const -> vector<Rectangle>;

  ///
  /// Determines whether the specified @arg other is joined horizontally.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined horizontally, false otherwise.
  [[nodiscard]] auto isJoinedX(const Rectangle &other) const -> bool;

  ///
  /// Determines whether the specified @arg other is joined vertically.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined vertically, false otherwise.
  [[nodiscard]] auto isJoinedY(const Rectangle &other) const -> bool;

  ///
  /// Determines whether the specified @arg other is joined vertically or horizontally.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined, false otherwise.
  [[nodiscard]] auto isJoined(const Rectangle &other) const -> bool;

  ///
  /// Join two rectangles.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Joined rectangles
  [[nodiscard]] auto joinedWith(const Rectangle &other) const -> optional<Rectangle>;

  ///
  // Get height of rectangle.
  ///
  /// @return     Height of rectangle
  [[nodiscard]] auto height() const -> Dim;

  ///
  /// Get width of rectangle.
  ///
  /// @return     Width of rectangle
  [[nodiscard]] auto width() const -> Dim;

  ///
  /// Get size of rectangle.
  ///
  /// @return     Size of rectangle
  [[nodiscard]] auto size() const -> Vector {
    return Vector(width(), height());
  }

  ///
  /// Get position of rectangle.
  ///
  /// @return     Position of rectangle
  [[nodiscard]] auto position() const -> Vector { return Vector(x1_, y1_); }

  ///
  /// Get position2 of rectangle.
  ///
  /// @return     Position of rectangle
  [[nodiscard]] auto position2() const -> Vector { return Vector(x2_, y2_); }

  ///
  /// Decrease rectangle equally on all sides.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Decreased rectangle
  [[nodiscard]] auto operator -(Dim size) const -> Rectangle;

  ///
  /// Enlarge rectangle equally on all sides.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Enlarged rectangle
  [[nodiscard]] auto operator +(Dim size) const -> Rectangle;

  ///
  /// Apply defaults.
  ///
  /// Substitutes all defaults (DimLow and DimHigh values) with values from supplied Rectangle.
  ///
  /// @param[in]  default_  The default
  ///
  /// @return     Rectangle with defaults applied
  [[nodiscard]] auto defaultTo(const Rectangle &default_) const -> Rectangle;

  [[nodiscard]] auto x1() const -> Dim {return x1_;}

  [[nodiscard]] auto x2() const -> Dim {return x2_;}

  [[nodiscard]] auto y1() const -> Dim {return y1_;}

  [[nodiscard]] auto y2() const -> Dim {return y2_;}

  [[nodiscard]] auto x1() -> Dim & {return x1_;}

  [[nodiscard]] auto x2() -> Dim & {return x2_;}

  [[nodiscard]] auto y1() -> Dim & {return y1_;}

  [[nodiscard]] auto y2() -> Dim & {return y2_;}

  private:
  ///
  /// Intersection with Rectangle in place
  ///
  /// @param[out] result  The result
  /// @param[in]  other   The other
  void intersect(vector<Rectangle> &result, const Rectangle &other) const;

  Dim x1_;
  Dim y1_;
  Dim x2_;
  Dim y2_;
};

/// Get difference between two Dims
///
/// @param[in]  x1
/// @param[in]  x2
///
/// @return     Difference
[[nodiscard]] auto diff(Dim x1, Dim x2) -> Dim;

/// Get rectangle from one vector centered in another
///
/// @param[in]  one    reference
/// @param[in]  other  vector to be centered within reference
///
/// @return     Rectangle from centering operation
[[nodiscard]] auto centered(const Vector &one, const Vector &other) -> Rectangle;

extern const Rectangle RectangleDefault;
extern const Rectangle RectangleMax;
extern const Vector VectorMin;
extern const Vector VectorMax;

} // namespace jwezel
