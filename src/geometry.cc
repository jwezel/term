// Geometric shapes
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <string>
#include <fmt/core.h>

#include "geometry.hh"

namespace jwezel {

using namespace fmt;

typedef int16_t Dim;

const Dim DimLow = INT16_MIN;
const Dim DimHigh = INT16_MAX;

Vector::Vector(Dim x, Dim y):
x(x), y(y)
{}

Vector::operator string() const {
  return format("Vec({}, {})", x, y);
}

std::ostream& Vector::operator<<(std::ostream& os) const {
  os << string(*this);
  return os;
}

bool Vector::operator ==(const Vector &other) const {
  return x == other.x && y == other.y;
}

bool Vector::operator !=(const Vector &other) const {
  return !(*this == other);
}

bool Vector::operator <(const Vector &other) const {
  return x < other.x or x == other.x and y < other.y;
}

bool Vector::operator >(const Vector &other) const {
  return other.x < x or x == other.x and other.y < y;
}

bool Vector::operator <=(const Vector &other) const {
  return !(x > other.x or y > other.y);
}

bool Vector::operator >=(const Vector &other) const {
  return !(x < other.x or y < other.y);
}

Vector Vector::position(const Vector &position) const {
  auto result{position};
  if (result.x < 0)
    result.x += x;
  if (result.y < 0)
    result.y += y;
  return result;
}

Vector Vector::defaultTo(const Vector &default_) const {
  if (*this == VectorMin) {
    return default_;
  } else {
    return Vector(
      (x == DimLow || x == DimHigh)? default_.x: x,
      (y == DimLow || y == DimHigh)? default_.y: y
    );
  }
}

Vector Vector::operator +(const Vector &v2) const {
  return Vector(x + v2.x, y + v2.y);
}

Vector Vector::operator +(Dim size) const {
  return Vector(x + size, y + size);
}

Vector &Vector::operator +=(Dim size) {
  x += size;
  y += size;
  return *this;
}

Vector Vector::operator -(const Vector &other) const {
  return Vector(x - other.x, y - other.y);
}

Vector Vector::operator -(Dim size) const {
  return Vector(x - size, y - size);
}

Vector &Vector::operator -=(Dim size) {
  x -= size;
  y -= size;
  return *this;
}

Vector Vector::left(Dim shift) const {
  return Vector(x - shift, y);
}

Vector Vector::right(Dim shift) const {
  return Vector(x + shift, y);
}

Vector Vector::up(Dim shift) const {
  return Vector(x, y - shift);
}

Vector Vector::down(Dim shift) const {
  return Vector(x, y + shift);
}

Vector Vector::operator |(const Vector &v2) const {
  return Vector(std::max(x, v2.x), std::max(y, v2.y));
}

Vector min(const Vector &v1, const Vector &v2) {
  return Vector{Dim(std::min(v1.x, v2.x)), Dim(std::min(v1.y, v2.y))};
}

Vector max(const Vector &v1, const Vector &v2) {
  return Vector{Dim(std::max(v1.x, v2.x)), Dim(std::max(v1.y, v2.y))};
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rectangle::Rectangle(Dim x1, Dim y1, Dim x2, Dim y2):
x1(x1), y1(y1), x2(x2), y2(y2)
{}

Rectangle::Rectangle(const Vector &v1, const Vector &v2):
x1(v1.x), y1(v1.y), x2(v2.x), y2(v2.y)
{}

bool Rectangle::operator ==(const Rectangle &other) const {
  return x1 == other.x1 && x2 == other.x2 && y1 == other.y1 && y2 == other.y2;
}

bool Rectangle::operator !=(const Rectangle &other) const {
  return !(*this == other);
}

Rectangle::operator string() const {
  return format("Rec({}, {}, {}, {})", x1, y1, x2, y2);
}

std::ostream& operator <<(std::ostream& os, const Rectangle &rect) {
    os << string(rect);
    return os;
}

optional<Rectangle> Rectangle::operator &(const Rectangle &other) const {
  const Dim
    rx1 = std::max(x1, other.x1),
    ry1 = std::max(y1, other.y1),
    rx2 = std::min(x2, other.x2),
    ry2 = std::min(y2, other.y2);
  if (rx1 < rx2 and ry1 < ry2)
    return Rectangle(rx1, ry1, rx2, ry2);
  else
    return nullopt;
}

bool Rectangle::intersects(const Rectangle &other) const {
  return std::max(x1, other.x1) < std::min(x2, other.x2) and std::max(y1, other.y1) < std::min(y2, other.y2);
}

Rectangle Rectangle::operator +(const Vector &vector) const {
  return Rectangle(x1 + vector.x, y1 + vector.y, x2 + vector.x, y2 + vector.y);
}

Rectangle Rectangle::operator -(const Vector &vector) const {
  return Rectangle(x1 - vector.x, y1 - vector.y, x2 - vector.x, y2 - vector.y);
}

Rectangle Rectangle::operator |(const Rectangle &other) const {
  return Rectangle(std::min(x1, other.x1), std::min(y1, other.y1), std::max(x2, other.x2), std::max(y2, other.y2));
}

const Rectangle &Rectangle::operator |=(const Rectangle &r2) {
  x1 = std::min(x1, r2.x1);
  y1 = std::min(y1, r2.y1);
  x2 = std::max(x2, r2.x2);
  y2 = std::max(y2, r2.y2);
  return *this;
}

void Rectangle::intersect(vector<Rectangle> &result, const Rectangle &other) const {
  if (y1 < other.y1)
    result.push_back(Rectangle(x1, y1, x2, other.y1));
  if (x1 < other.x1)
    result.push_back(Rectangle(x1, std::max(y1, other.y1), other.x1, std::min(y2, other.y2)));
  if (x2 > other.x2)
    result.push_back(Rectangle(other.x2, std::max(y1, other.y1), x2, std::min(y2, other.y2)));
  if (y2 > other.y2)
    result.push_back(Rectangle(x1, other.y2, x2, y2));
}

vector<Rectangle> Rectangle::operator -(const Rectangle &other) const {
  const auto intersection = *this & other;
  if (!intersection || intersection.value() == *this) {
    // If the two rectangles do not intersect or the intersection is *this, an empty sequence is returned.
    return {};
  }
  // Otherwise, a sequence of up to four rectangles is returned
  vector<Rectangle> result;
  intersect(result, other);
  return result;
}

vector<Rectangle> Rectangle::defaultIntersection(const Rectangle &other) const {
  const auto intersection = *this & other;
  if (!intersection) {
    return {*this};
  }
  if (intersection.value() == *this) {
    // If the two rectangles do not intersect or the intersection is *this, an empty sequence is returned.
    return {};
  }
  // Otherwise, a sequence of up to four rectangles is returned
  vector<Rectangle> result;
  intersect(result, other);
  return result;
}

bool isJoinedX(const Rectangle &r1, const Rectangle &r2) {
  return r1.x2 == r2.x1 and r1.y1 == r2.y1 and r1.y2 == r2.y2;
}

bool isJoinedY(const Rectangle &r1, const Rectangle &r2) {
  return r1.x1 == r2.x1 and r1.x2 == r2.x2 and r1.y2 == r2.y1;
}

bool Rectangle::isJoinedX(const Rectangle &r2) const {
  return jwezel::isJoinedX(*this, r2) or jwezel::isJoinedX(r2, *this);
}

bool Rectangle::isJoinedY(const Rectangle &r2) const {
  return jwezel::isJoinedY(*this, r2) or jwezel::isJoinedY(r2, *this);
}

bool Rectangle::isJoined(const Rectangle &r2) const {
  return (isJoinedX(r2)) or (isJoinedY(r2));
}

optional<Rectangle> Rectangle::joinedWith(const Rectangle &r2) const {
  if (isJoined(r2))
    return make_optional(*this | r2);
  return nullopt;
}

Dim Rectangle::height() const {
  return diff(y2, y1);
}

Dim Rectangle::width() const {
  return diff(x2, x1);
}

Vector Rectangle::size() const {
  return Vector(width(), height());
}

Vector Rectangle::position() const {
  return Vector(x1, y1);
}

Rectangle Rectangle::operator -(Dim size) const {
  const auto result = Rectangle(x1 + size, y1 + size, x2 - size, y2 - size);
  if (result == RectangleDefault)
    throw RectangleError(string("Result must not become RectangleDefault: )") + string(result));
  assert((format("x axis got negativ: {} + {}", string(*this), size), result.x1 <= result.x2));
  assert((format("y axis got negativ: {} + {}", string(*this), size), result.y1 <= result.y2));
  return result;
}

Rectangle Rectangle::operator +(Dim size) const {
  const auto result = Rectangle(x1 - size, y1 - size, x2 + size, y2 + size);
  if (result == RectangleDefault)
    throw RectangleError(string("Result must not become RectangleDefault: ") + string(result));
  assert((format("x axis got negativ: ", string(*this), size), result.x1 <= result.x2));
  assert((format("y axis got negativ: ", string(*this), size), result.y1 <= result.y2));
  return result;
}

Rectangle Rectangle::defaultTo(const Rectangle &default_) const {
  return Rectangle(
    x1 == DimLow || x1 == DimHigh? default_.x1: x1,
    y1 == DimLow || y1 == DimHigh? default_.y1: y1,
    x2 == DimLow || x2 == DimHigh? default_.x2: x2,
    y2 == DimLow || y2 == DimHigh? default_.y2: y2
  );
}

Rectangle centered(const Vector &one, const Vector &other) {
  const Dim
    ox = other.x / 2,
    oy = other.y / 2,
    tx = one.x / 2,
    ty = one.y / 2;
  return Rectangle(ox - tx, oy - ty, ox + tx, oy + ty);
}

const Rectangle RectangleDefault = Rectangle(DimLow, DimLow, DimLow, DimLow);
const Rectangle RectangleMax = Rectangle(DimLow, DimLow, DimHigh, DimHigh);
const Vector VectorMin = Vector(DimLow, DimLow);
const Vector VectorMax = Vector(DimHigh, DimHigh);

Dim diff(Dim x1, Dim x2) {
  return x1 < x2? x2 - x1: x1 - x2;
}

} // namespace
