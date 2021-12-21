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
#include <string>
#include <optional>
#include <vector>
#include <stdexcept>

namespace jwezel {

using namespace std;

/// Dimension (x, y) in a @class Vector.
typedef int16_t Dim;

extern const Dim DimLow;  ///< Lowest possible value of a dimension
extern const Dim DimHigh; ///< Highest possible value of a dimension

class RectangleError: public logic_error
{
  public:
  RectangleError(const string &what): logic_error(what) {}
};

///
/// A vector
struct Vector {
  /// Constructor
  ///
  /// @param[in]  x
  /// @param[in]  y
  Vector(Dim x=0, Dim y=0);

  ///
  /// Convert Vector to string
  operator string() const;

  ///
  /// Convert Vector to string
  ///
  /// Alias for operator string()
  std::ostream& operator<<(std::ostream& os) const;

  /// Compare two vectors for equality.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     True if equal
  bool operator ==(const Vector &other) const;

  /// Compare two vectors for inequality.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     True in not equal
  bool operator !=(const Vector &other) const;

  ///
  /// Compare vector less than other
  ///
  /// @param[in]  other  The other
  ///
  /// @return     True if less
  bool operator <(const Vector &other) const;

  bool operator >(const Vector &other) const;

  bool operator <=(const Vector &other) const;

  bool operator >=(const Vector &other) const;

  ///
  /// Replace negative elements in @c *this with sum of element in @c *this and
  /// @arg position such that -1 becomes last.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     Vector with adjusted values
  Vector position(const Vector &position) const;

  /// Apply defaults
  ///
  /// Substitutes all default values (DimLow and DimHigh values) with values from @arg default_
  ///
  /// @param[in]  default_  The default
  ///
  /// @return     Vector with defaults applied
  Vector defaultTo(const Vector &default_) const;

  ///
  /// Addition operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the addition
  Vector operator +(const Vector &other) const;

  ///
  /// Enlarge Vector.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Enlarged Vector
  Vector operator +(Dim size) const;

  ///
  /// Subtract vectors.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the subtraction
  Vector operator -(const Vector &other) const;

  ///
  /// Decrease Vector.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Decreased Vector
  Vector operator -(Dim size) const;

  ///
  /// Shift Vector left
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  Vector left(Dim shift=1) const;

  ///
  /// Shift Vector right
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  Vector right(Dim shift=1) const;

  ///
  /// Shift Vector up
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  Vector up(Dim shift=1) const;

  ///
  /// Shift Vector down
  ///
  /// @param[in]  shift  The shift
  ///
  /// @return     Shifted Vector
  Vector down(Dim shift=1) const;

  ///
  /// Span of two Vectors.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     Span of the Vectors
  Vector operator |(const Vector &other) const;

  Dim x;
  Dim y;
};

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
  Rectangle(Dim x1=0, Dim y1=0, Dim x2=0, Dim y2=0);

  ///
  /// Constructor
  ///
  /// @param[in]  v1
  /// @param[in]  v2
  Rectangle(const Vector &v1, const Vector &v2);

  ///
  /// String conversion operator.
  operator string() const;

  ///
  /// Write Vector to stream
  ///
  /// @param      os    The output stream
  ///
  /// @return     Reference to output stream
  friend std::ostream& operator <<(std::ostream& os, const Rectangle &rect);

  ///
  /// Equality operator.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if equal
  bool operator ==(const Rectangle &other) const;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if not equal
  bool operator !=(const Rectangle &other) const;

  ///
  /// Rectangle intersection.
  ///
  /// If @arg other intersects, returns intersection
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Intersecting rectangle [optional]
  optional<Rectangle> operator &(const Rectangle &other) const;

  ///
  /// Test for intersection.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if @a other intersects
  bool intersects(const Rectangle &other) const;

  ///
  /// Shift rectangle by vector.
  ///
  /// @param[in]  v     The vector
  ///
  /// @return     Shifted rectangle
  Rectangle operator +(const Vector &v) const;

  ///
  /// Get area spanning two rectangles.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Rectangle spanning @c *this and @arg other
  Rectangle operator |(const Rectangle &other) const;

  ///
  /// Assign spanning rectangle.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Rectangle spanning @c *this and
  /// @arg        other
  const Rectangle &operator |=(const Rectangle &other);

  ///
  /// Move rectangle by vector.
  ///
  /// @param[in]  vector  The vector
  ///
  /// @return     Moved rectangle
  Rectangle operator -(const Vector &vector) const;

  ///
  /// Get vector of rectangle parts from clipping two rectangles.
  ///
  /// The vector contains all parts except the intersection. If the two rectangles do not intersect,
  /// or if the intersection is @c *this, an empty vector is returned.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Vector of parts
  vector<Rectangle> operator -(const struct Rectangle &other) const;

  ///
  /// Get vector of rectangle parts from clipping two rectangles.
  ///
  /// The vector contains all parts except the intersection. If the two rectangles do not intersect,
  /// the result vector contains only @c *this. If the intersection is @c *this, an empty vector is returned.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     std::vector of parts
  vector<Rectangle> defaultIntersection(const Rectangle &other) const;

  ///
  /// Determines whether the specified @arg other is joined horizontally.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined horizontally, false otherwise.
  bool isJoinedX(const Rectangle &other) const;

  ///
  /// Determines whether the specified @arg other is joined vertically.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined vertically, false otherwise.
  bool isJoinedY(const Rectangle &other) const;

  ///
  /// Determines whether the specified @arg other is joined vertically or horizontally.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     True if the specified @arg other is joined, false otherwise.
  bool isJoined(const Rectangle &other) const;

  ///
  /// Join two rectangles.
  ///
  /// @param[in]  other  Other rectangle
  ///
  /// @return     Joined rectangles
  optional<Rectangle> joinedWith(const Rectangle &other) const;

  ///
  // Get height of rectangle.
  ///
  /// @return     Height of rectangle
  Dim height() const;

  ///
  /// Get width of rectangle.
  ///
  /// @return     Width of rectangle
  Dim width() const;

  ///
  /// Get size of rectangle.
  ///
  /// @return     Size of rectangle
  Vector size() const;

  ///
  /// Get position of rectangle.
  ///
  /// @return     Position of rectangle
  Vector position() const;

  ///
  /// Decrease rectangle equally on all sides.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Decreased rectangle
  Rectangle operator -(Dim size) const;

  ///
  /// Enlarge rectangle equally on all sides.
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Enlarged rectangle
  Rectangle operator +(Dim size) const;

  ///
  /// Apply defaults.
  ///
  /// Substitutes all defaults (DimLow and DimHigh values) with values from supplied Rectangle.
  ///
  /// @param[in]  default_  The default
  ///
  /// @return     Rectangle with defaults applied
  Rectangle defaultTo(const Rectangle &default_) const;

  Dim x1;
  Dim y1;
  Dim x2;
  Dim y2;

  private:
  ///
  /// Intersection with Rectangle in place
  ///
  /// @param[out] result  The result
  /// @param[in]  other   The other
  void intersect(vector<Rectangle> &result, const Rectangle &other) const;
};

/// Get difference between two Dims
///
/// @param[in]  x1
/// @param[in]  x2
///
/// @return     Difference
Dim diff(Dim x1, Dim x2);

/// Get rectangle from one vector centered in another
///
/// @param[in]  one    reference
/// @param[in]  other  vector to be centered within reference
///
/// @return     Rectangle from centering operation
Rectangle centered(const Vector &one, const Vector &other);

///
/// Minimum of two Vectors
///
/// @param[in]  v1    Vector 1
/// @param[in]  v2    Vector 2
///
/// @return     Minimum
Vector min(const Vector &v1, const Vector &v2);

///
/// Maximum of two Vectors
///
/// @param[in]  v1    Vector 1
/// @param[in]  v2    Vector 2
///
/// @return     Maximum
Vector max(const Vector &v1, const Vector &v2);

extern const Rectangle RectangleDefault;
extern const Rectangle RectangleMax;
extern const Vector VectorMin;
extern const Vector VectorMax;

}
