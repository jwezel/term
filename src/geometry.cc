// Geometric shapes
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "geometry.hh"

namespace jwezel {

using fmt::format;
using std::nullopt, std::make_optional;

using Dim = int16_t;

const Dim DimLow = INT16_MIN;
const Dim DimHigh = INT16_MAX;

Vector::Vector(Dim x, Dim y) noexcept:
x_(x), y_(y)
{}

Vector::operator string() const {
  return format("Vec({}, {})", x_, y_);
}

auto Vector::operator<<(std::ostream& os) const -> std::ostream& {
  os << string(*this);
  return os;
}

auto Vector::position(const Vector &position) const -> Vector {
  auto result{position};
  if (result.x() < 0) {
    result.x_ += x_;
  }
  if (result.y() < 0) {
    result.y_ += y_;
  }
  return result;
}

auto Vector::defaultTo(const Vector &default_) const -> Vector {
  if (*this == VectorMin) {
    return default_;
  }
  return Vector(
    (x_ == DimLow || x_ == DimHigh)? default_.x_: x_,
    (y_ == DimLow || y_ == DimHigh)? default_.y_: y_
  );
}

auto Vector::operator +(const Vector &other) const -> Vector {
  return Vector(toDim(x_ + other.x()), toDim(y_ + other.y()));
}

auto Vector::operator +(Dim size) const -> Vector {
  return Vector(toDim(x_ + size), toDim(y_ + size));
}

auto Vector::operator +=(Dim size) -> Vector & {
  x_ += size;
  y_ += size;
  return *this;
}

auto Vector::operator -(const Vector &other) const -> Vector {
  return Vector(toDim(x_ - other.x()), toDim(y_ - other.y()));
}

auto Vector::operator -(Dim size) const -> Vector {
  return Vector(toDim(x_ - size), toDim(y_ - size));
}

auto Vector::operator *(double factor) const -> Vector {
  return Vector{static_cast<Dim>(x_ * factor)};
}

auto Vector::operator /(double divisor) const -> Vector {
  return Vector{static_cast<Dim>(x_ / divisor)};
}

auto Vector::left(Dim shift) const -> Vector {
  return Vector(toDim(x_ - shift), y_);
}

auto Vector::right(Dim shift) const -> Vector {
  return Vector(toDim(x_ + shift), y_);
}

auto Vector::up(Dim shift) const -> Vector {
  return Vector(x_, toDim(y_ - shift));
}

auto Vector::down(Dim shift) const -> Vector {
  return Vector(x_, toDim(y_ + shift));
}

// TODO(j): Test
auto Vector::left(Dim shift) -> Vector & {
  x_ = toDim(x_ - shift);
  return *this;
}

// TODO(j): Test
auto Vector::right(Dim shift) -> Vector & {
  x_ = toDim(x_ + shift);
  return *this;
}

// TODO(j): Test
auto Vector::up(Dim shift) -> Vector & {
  y_ = toDim(y_ - shift);
  return *this;
}

// TODO(j): Test
auto Vector::down(Dim shift) -> Vector & {
  y_ = toDim(y_ + shift);
  return *this;
}

auto Vector::operator |(const Vector &other) const -> Vector {
  return Vector(std::max(x_, other.x()), std::max(y_, other.y()));
}

auto min(const Vector &v1, const Vector &v2) -> Vector {
  return Vector{Dim(std::min(v1.x(), v2.x())), Dim(std::min(v1.y(), v2.y()))};
}

auto max(const Vector &v1, const Vector &v2) -> Vector {
  return Vector{Dim(std::max(v1.x(), v2.x())), Dim(std::max(v1.y(), v2.y()))};
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rectangle::Rectangle(Dim x1, Dim y1_, Dim x2_, Dim y2_) noexcept:
x1_(x1), y1_(y1_), x2_(x2_), y2_(y2_)
{}

Rectangle::Rectangle(const Vector &v1, const Vector &v2) noexcept:
x1_(v1.x()), y1_(v1.y()), x2_(v2.x()), y2_(v2.y())
{}

auto Rectangle::operator ==(const Rectangle &other) const -> bool {
  return x1_ == other.x1() && x2_ == other.x2() && y1_ == other.y1() && y2_ == other.y2();
}

auto Rectangle::operator !=(const Rectangle &other) const -> bool {
  return !(*this == other);
}

Rectangle::operator string() const {
  return format("Rec({}, {}, {}, {})", x1_, y1_, x2_, y2_);
}

auto operator <<(std::ostream& os, const Rectangle &rect) -> std::ostream& {
    os << string(rect);
    return os;
}

auto Rectangle::operator &(const Rectangle &other) const -> optional<Rectangle> {
  const Dim
    rx1 = std::max(x1_, other.x1()),
    ry1 = std::max(y1_, other.y1()),
    rx2 = std::min(x2_, other.x2()),
    ry2 = std::min(y2_, other.y2());
  if (rx1 < rx2 and ry1 < ry2) {
    return Rectangle(rx1, ry1, rx2, ry2);
  }
  return nullopt;
}

auto Rectangle::intersects(const Rectangle &other) const -> bool {
  return std::max(x1_, other.x1()) < std::min(x2_, other.x2()) and std::max(y1_, other.y1()) < std::min(y2_, other.y2());
}

auto Rectangle::operator +(const Vector &vector) const -> Rectangle {
  return Rectangle(toDim(x1_ + vector.x()), toDim(y1_ + vector.y()), toDim(x2_ + vector.x()), toDim(y2_ + vector.y()));
}

auto Rectangle::operator -(const Vector &vector) const -> Rectangle {
  return Rectangle(toDim(x1_ - vector.x()), toDim(y1_ - vector.y()), toDim(x2_ - vector.x()), toDim(y2_ - vector.y()));
}

auto Rectangle::operator |(const Rectangle &other) const -> Rectangle {
  return Rectangle(std::min(x1_, other.x1()), std::min(y1_, other.y1()), std::max(x2_, other.x2()), std::max(y2_, other.y2()));
}

auto Rectangle::operator |=(const Rectangle &other) -> const Rectangle & {
  x1_ = std::min(x1_, other.x1());
  y1_ = std::min(y1_, other.y1());
  x2_ = std::max(x2_, other.x2());
  y2_ = std::max(y2_, other.y2());
  return *this;
}

void Rectangle::intersect(vector<Rectangle> &result, const Rectangle &other) const {
  if (y1_ < other.y1()) {
    result.emplace_back(x1_, y1_, x2_, other.y1());
  }
  if (x1_ < other.x1()) {
    result.emplace_back(x1_, std::max(y1_, other.y1()), other.x1(), std::min(y2_, other.y2()));
  }
  if (x2_ > other.x2()) {
    result.emplace_back(other.x2(), std::max(y1_, other.y1()), x2_, std::min(y2_, other.y2()));
  }
  if (y2_ > other.y2()) {
    result.emplace_back(x1_, other.y2(), x2_, y2_);
  }
}

auto Rectangle::operator -(const Rectangle &other) const -> vector<Rectangle> {
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

auto Rectangle::defaultIntersection(const Rectangle &other) const -> vector<Rectangle> {
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

auto Rectangle::left(Dim shift) const -> Rectangle {
  return Rectangle(toDim(x1_ - shift), y1_, toDim(x2_ - shift), y2_);
}

auto Rectangle::right(Dim shift) const -> Rectangle {
  return Rectangle(toDim(x1_ + shift), y1_, toDim(x2_ + shift), y2_);
}

auto Rectangle::up(Dim shift) const -> Rectangle {
  return Rectangle(x1_, toDim(y1_ - shift), x2_, toDim(y2_ - shift));
}

auto Rectangle::down(Dim shift) const -> Rectangle {
  return Rectangle(x1_, toDim(y1_ + shift), x2_, toDim(y2_ + shift));
}

// TODO(j): Test
auto Rectangle::left(Dim shift) -> Rectangle & {
  x1_ = toDim(x1_ - shift);
  x2_ = toDim(x2_ - shift);
  return *this;
}

// TODO(j): Test
auto Rectangle::right(Dim shift) -> Rectangle & {
  x1_ = toDim(x1_ + shift);
  x2_ = toDim(x2_ + shift);
  return *this;
}

// TODO(j): Test
auto Rectangle::up(Dim shift) -> Rectangle & {
  y1_ = toDim(y1_ - shift);
  y2_ = toDim(y2_ - shift);
  return *this;
}

// TODO(j): Test
auto Rectangle::down(Dim shift) -> Rectangle & {
  y1_ = toDim(y1_ + shift);
  y2_ = toDim(y2_ + shift);
  return *this;
}

auto isJoinedX(const Rectangle &r1, const Rectangle &r2) -> bool {
  return r1.x2() == r2.x1() and r1.y1() == r2.y1() and r1.y2() == r2.y2();
}

auto isJoinedY(const Rectangle &r1, const Rectangle &r2) -> bool {
  return r1.x1() == r2.x1() and r1.x2() == r2.x2() and r1.y2() == r2.y1();
}

auto Rectangle::isJoinedX(const Rectangle &other) const -> bool {
  return jwezel::isJoinedX(*this, other) or jwezel::isJoinedX(other, *this);
}

auto Rectangle::isJoinedY(const Rectangle &other) const -> bool {
  return jwezel::isJoinedY(*this, other) or jwezel::isJoinedY(other, *this);
}

auto Rectangle::isJoined(const Rectangle &other) const -> bool {
  return (isJoinedX(other)) or (isJoinedY(other));
}

auto Rectangle::joinedWith(const Rectangle &other) const -> optional<Rectangle> {
  if (isJoined(other)) {
    return make_optional(*this | other);
  }
  return nullopt;
}

auto Rectangle::height() const -> Dim {
  return diff(y2_, y1_);
}

auto Rectangle::width() const -> Dim {
  return diff(x2_, x1_);
}

auto Rectangle::operator -(Dim size) const -> Rectangle {
  const auto result = Rectangle(toDim(x1_ + size), toDim(y1_ + size), toDim(x2_ - size), toDim(y2_ - size));
  if (result == RectangleDefault) {
    throw RectangleError(string("Result must not become RectangleDefault: )") + string(result));
  }
  assert(result.x1() <= result.x2()); // NOLINT
  assert(result.y1() <= result.y2()); // NOLINT
  return result;
}

auto Rectangle::operator +(Dim size) const -> Rectangle {
  const auto result = Rectangle(toDim(x1_ - size), toDim(y1_ - size), toDim(x2_ + size), toDim(y2_ + size));
  if (result == RectangleDefault) {
    throw RectangleError(string("Result must not become RectangleDefault: ") + string(result));
  }
  assert(result.x1() <= result.x2()); // NOLINT
  assert(result.y1() <= result.y2()); // NOLINT
  return result;
}

auto Rectangle::defaultTo(const Rectangle &default_) const -> Rectangle {
  return Rectangle(
    x1_ == DimLow || x1_ == DimHigh? default_.x1(): x1_,
    y1_ == DimLow || y1_ == DimHigh? default_.y1(): y1_,
    x2_ == DimLow || x2_ == DimHigh? default_.x2(): x2_,
    y2_ == DimLow || y2_ == DimHigh? default_.y2(): y2_
  );
}

auto centered(const Vector &one, const Vector &other) -> Rectangle {
  const Dim
    ox = toDim(other.x() / 2),
    oy = toDim(other.y() / 2),
    tx = toDim(one.x() / 2),
    ty = toDim(one.y() / 2);
  return Rectangle(toDim(ox - tx), toDim(oy - ty), toDim(ox + tx), toDim(oy + ty));
}

auto diff(Dim x1, Dim x2_) -> Dim {
  return toDim(x1 < x2_? x2_ - x1: x1 - x2_);
}

const Rectangle RectangleDefault = Rectangle(DimLow, DimLow, DimLow, DimLow);
const Rectangle RectangleMax = Rectangle(DimLow, DimLow, DimHigh, DimHigh);
const Vector VectorMin = Vector(DimLow, DimLow);
const Vector VectorMax = Vector(DimHigh, DimHigh);

} // namespace jwezel
