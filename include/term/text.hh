///
/// @defgroup   TEXT text
///
/// This file implements the Text class.
///
/// @author     J
/// @date       2020

#pragma once

#include "basic.hh"
#include "geometry.hh"

#include <array>
#include <string>
#include <string_view>

namespace jwezel {

using std::string, std::array, std::string_view;

using Unicode = char32_t;

const Unicode NoneRune = 0xffffffff;

///
/// RGB color attributes.
struct Rgb {
  explicit Rgb(f4 r=-1., f4 g=0., f4 b=0.) noexcept;

  ~Rgb() = default;

  Rgb(Rgb &&) = default;

  auto operator=(Rgb &&) -> Rgb & = delete;

  explicit operator string() const;

  explicit operator struct Hsv() const noexcept;

  Rgb(const Rgb & color) = default;

  auto operator = (const Rgb & color) -> Rgb & = default;

  auto operator |(const Rgb &other) const -> Rgb;

  auto operator ==(const Rgb &other) const -> bool;

  auto operator !=(const Rgb &other) const -> bool;

  auto operator +(const Rgb &color2) const -> Rgb;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  f4 r; ///< red (0 .. 1)
  f4 g; ///< green (0 .. 1)
  f4 b; ///< blue (0 .. 1)
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

///
/// HSV color attributes.
struct Hsv {
  Hsv(): h(-1), s(-1), v(-1) {}

  Hsv(f4 h, f4 s, f4 v): h(h), s(s), v(v) {}

  explicit operator string() const;

  explicit operator Rgb() const;

  auto operator ==(const Hsv &other) const -> bool;

  auto operator !=(const Hsv &other) const -> bool;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  f4 h; ///< hue (angle in decimal degrees (0 .. 360))
  f4 s; ///< saturation (0 .. 1)
  f4 v; ///< value (0 .. 1)
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

const Rgb
  RgbTransparent =  Rgb{-2.0, -2.0, -2.0},
  RgbNone =         Rgb{-1.0, -1.0, -1.0},
  RgbWhite =        Rgb{1.0, 1.0, 1.0},
  RgbBlack =        Rgb{0.0, 0.0, 0.0},
  RgbGray1 =        Rgb{0.1, 0.1, 0.1},
  RgbGray2 =        Rgb{0.2, 0.2, 0.2},
  RgbGray3 =        Rgb{0.3, 0.3, 0.3},
  RgbGray4 =        Rgb{0.4, 0.4, 0.4},
  RgbGray5 =        Rgb{0.5, 0.5, 0.5},
  RgbGray6 =        Rgb{0.6, 0.6, 0.6},
  RgbGray7 =        Rgb{0.7, 0.7, 0.7},
  RgbGray8 =        Rgb{0.8, 0.8, 0.8},
  RgbGray9 =        Rgb{0.9, 0.9, 0.9},
  RgbRed =          Rgb{1.0, 0.0, 0.0},
  RgbRed1 =         Rgb{0.1, 0.0, 0.0},
  RgbRed2 =         Rgb{0.2, 0.0, 0.0},
  RgbRed3 =         Rgb{0.3, 0.0, 0.0},
  RgbRed4 =         Rgb{0.4, 0.0, 0.0},
  RgbRed5 =         Rgb{0.5, 0.0, 0.0},
  RgbRed6 =         Rgb{0.6, 0.0, 0.0},
  RgbRed7 =         Rgb{0.7, 0.0, 0.0},
  RgbRed8 =         Rgb{0.8, 0.0, 0.0},
  RgbRed9 =         Rgb{0.9, 0.0, 0.0},
  RgbGreen =        Rgb{0.0, 1.0, 0.0},
  RgbGreen1 =       Rgb{0.0, 0.1, 0.0},
  RgbGreen2 =       Rgb{0.0, 0.2, 0.0},
  RgbGreen3 =       Rgb{0.0, 0.3, 0.0},
  RgbGreen4 =       Rgb{0.0, 0.4, 0.0},
  RgbGreen5 =       Rgb{0.0, 0.5, 0.0},
  RgbGreen6 =       Rgb{0.0, 0.6, 0.0},
  RgbGreen7 =       Rgb{0.0, 0.7, 0.0},
  RgbGreen8 =       Rgb{0.0, 0.8, 0.0},
  RgbGreen9 =       Rgb{0.0, 0.9, 0.0},
  RgbBlue =         Rgb{0.0, 0.0, 1.0},
  RgbBlue1 =        Rgb{0.0, 0.0, 0.1},
  RgbBlue2 =        Rgb{0.0, 0.0, 0.2},
  RgbBlue3 =        Rgb{0.0, 0.0, 0.3},
  RgbBlue4 =        Rgb{0.0, 0.0, 0.4},
  RgbBlue5 =        Rgb{0.0, 0.0, 0.5},
  RgbBlue6 =        Rgb{0.0, 0.0, 0.6},
  RgbBlue7 =        Rgb{0.0, 0.0, 0.7},
  RgbBlue8 =        Rgb{0.0, 0.0, 0.8},
  RgbBlue9 =        Rgb{0.0, 0.0, 0.9},
  RgbYellow =       Rgb{1.0, 1.0, 0.0},
  RgbYellow1 =      Rgb{0.1, 0.1, 0.0},
  RgbYellow2 =      Rgb{0.2, 0.2, 0.0},
  RgbYellow3 =      Rgb{0.3, 0.3, 0.0},
  RgbYellow4 =      Rgb{0.4, 0.4, 0.0},
  RgbYellow5 =      Rgb{0.5, 0.5, 0.0},
  RgbYellow6 =      Rgb{0.6, 0.6, 0.0},
  RgbYellow7 =      Rgb{0.7, 0.7, 0.0},
  RgbYellow8 =      Rgb{0.8, 0.8, 0.0},
  RgbYellow9 =      Rgb{0.9, 0.9, 0.0},
  RgbMagenta =      Rgb{1.0, 0.0, 1.0},
  RgbMagenta1 =     Rgb{0.1, 0.0, 0.1},
  RgbMagenta2 =     Rgb{0.2, 0.0, 0.2},
  RgbMagenta3 =     Rgb{0.3, 0.0, 0.3},
  RgbMagenta4 =     Rgb{0.4, 0.0, 0.4},
  RgbMagenta5 =     Rgb{0.5, 0.0, 0.5},
  RgbMagenta6 =     Rgb{0.6, 0.0, 0.6},
  RgbMagenta7 =     Rgb{0.7, 0.0, 0.7},
  RgbMagenta8 =     Rgb{0.8, 0.0, 0.8},
  RgbMagenta9 =     Rgb{0.9, 0.0, 0.9},
  RgbCyan =         Rgb{0.0, 1.0, 1.0},
  RgbCyan1 =        Rgb{0.0, 0.1, 0.1},
  RgbCyan2 =        Rgb{0.0, 0.2, 0.2},
  RgbCyan3 =        Rgb{0.0, 0.3, 0.3},
  RgbCyan4 =        Rgb{0.0, 0.4, 0.4},
  RgbCyan5 =        Rgb{0.0, 0.5, 0.5},
  RgbCyan6 =        Rgb{0.0, 0.6, 0.6},
  RgbCyan7 =        Rgb{0.0, 0.7, 0.7},
  RgbCyan8 =        Rgb{0.0, 0.8, 0.8},
  RgbCyan9 =        Rgb{0.0, 0.9, 0.9};

const int HighColor{255};

///
/// Attributes for the Char type.
using Attributes = u1;

const Attributes
  clear = 0U,
  bold = 1U,
  underline = 2U,
  reverse = 4U,
  blink = 8U;

///
/// Attribute combining mode for the Char type.
enum class AttributeMode: u1 {
  default_, ///< default
  merge,
  mix,
  replace,
  ignore
};

///
/// Alignment
enum class Alignment: u1 {
  left,
  center,
  right
};

const extern vector<string> AttributeMode2String;

///
/// This struct describes character attributes.
struct CharAttributes {
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Rgb           fg;   ///< foreground
  Rgb           bg;   ///< background
  Attributes    attr; ///< attributes
  AttributeMode mix;  ///< attribute mapping
  // NOLINTEND(misc-non-private-member-variables-in-classes)

  ///
  /// Constructor
  ///
  /// @param[in]  fg    The foreground
  /// @param[in]  bg    The background
  /// @param[in]  attr  The attribute
  /// @param[in]  mix   The mix
  explicit CharAttributes(
    const Rgb &fg=RgbNone,
    const Rgb &bg=RgbNone,
    const Attributes &attr={},
    const AttributeMode &mix=AttributeMode::default_
  );

  ///
  /// String conversion operator.
  explicit operator string() const;

  ///
  /// Equality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the equality
  auto operator ==(const CharAttributes &other) const -> bool;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the inequality
  auto operator !=(const CharAttributes &other) const -> bool;
};

///
/// Line orientation
enum Orientation {
  Horizontal,
  Vertical
};

///
/// Line segments
using Segments = array<u1, 4>;

///
/// Quad
enum QuadTupleField {
  north,
  south,
  west,
  east,
  mode
};

///
/// Quad tuple: north, south, west, east, mode
using QuadTuple = array<u1, mode + 1>;

///
/// Quad value (unsigned with encoded bit fields)
using QuadValue = u2;

/// Quad fields shift positions
const unsigned
  QuadNorthShift = 9U,
  QuadSouthShift = 7U,
  QuadWestShift = 5U,
  QuadEasthShift = 3U;

const u1
  MinDash = 1,
  MaxDash = 2;

///
/// Line drawing attributes.
struct Draw {
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  u1    strength = 1;
  u1    dash = 0;
  bool  roundedCorners = false;
  // NOLINTEND(misc-non-private-member-variables-in-classes)

  ///
  /// Constructor
  ///
  /// @param[in]  strength        The strength
  /// @param[in]  dash            The dash
  /// @param[in]  roundedCorners  The rounded corners
  Draw(u1 strength, u1 dash, bool roundedCorners);
};

struct Line {
  Vector      position{0, 0};
  Dim         endPosition = DimHigh;
  Orientation orientation = Horizontal;
  bool        extendBegin = false;
  bool        extendEnd = false;
};

///
/// This struct describes a box.
struct Box: public Draw {
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Rectangle area = RectangleMax;
  // NOLINTEND(misc-non-private-member-variables-in-classes)

  explicit Box(const Rectangle &area=RectangleMax, u1 strength=1, u1 dash=0, bool roundedCorners=false);
};

///
/// This struct describes a character.
struct Char {
  Char();
  ///
  /// Constructor
  ///
  /// @param[in]  c     Character
  /// @param[in]  fg    The foreground
  /// @param      bg    The background
  /// @param[in]  attr  The attribute
  /// @param[in]  mix   The mix
  explicit Char(
    Unicode c,
    const Rgb &fg=RgbNone,
    const Rgb &bg=RgbNone,
    const Attributes &attr={},
    const AttributeMode &mix=AttributeMode::default_
  );

  ///
  /// Constructor
  ///
  /// @param[in]  c     Character
  /// @param[in]  attr  The attribute
  Char(Unicode c, const CharAttributes &attr);

  ///
  /// String conversion operator.
  explicit operator string() const;

  ///
  /// Char with attributes replaced
  ///
  /// @param[in]  attr  The attributes
  ///
  /// @return     Char with new attributes
  [[nodiscard]] auto withAttr(const CharAttributes &attr) const -> Char;

  [[nodiscard]] auto combine(
    const Char &other,
    const AttributeMode &mixDefaultMode=AttributeMode::merge,
    const AttributeMode &overrideMixMode=AttributeMode::default_,
    const AttributeMode &resetMixMode=AttributeMode::default_
  ) const -> Char;

  ///
  /// Draw line in Char.
  ///
  /// @param[in]  segments        The line segments
  /// @param[in]  dash            The dash mode
  /// @param[in]  roundedCorners  The rounded corners mode
  void drawLineChar(const Segments &segments, u1 dash, bool roundedCorners);

  ///
  /// Equality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     Whether Chars are equal
  auto operator ==(const Char &other) const -> bool;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     Whether Chars are unequal
  ///
  auto operator !=(const Char &other) const -> bool;

  [[nodiscard]] auto utf8() const -> string;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Unicode         rune;       ///< Unicode code point
  CharAttributes  attributes; ///< attributes
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

inline auto operator ""_C(char ch) -> Char {
  return Char(ch);
}

// NOLINTBEGIN(cert-err58-cpp)
const Char
  Space(32),
  Null(0),
  Transparent(32, RgbTransparent, RgbTransparent);
// NOLINTEND(cert-err58-cpp)

using String = vector<Char>;

extern auto asString(const String &s) -> string;

///
/// This struct describes a rectangle of text.
struct Text {
  Text() = default;

  ///
  /// Constructor
  ///
  /// An empty string constitutes zero lines, while a new line at the end of a line always adds an empty line.
  ///
  /// @param[in]  str   The string
  /// @param[in]  fg    The foreground
  /// @param[in]  bg    The background
  /// @param[in]  attr  The attribute
  /// @param[in]  mix   The mix
  explicit Text(
    const string_view &str,
    const Rgb &fg=RgbNone,
    const Rgb &bg=RgbNone,
    const Attributes &attr={},
    const AttributeMode &mix=AttributeMode::default_
  );

  ///
  /// Constructor
  ///
  /// @param[in]  str   The string
  /// @param[in]  attr  CharAttributes
  inline Text(string_view str, const CharAttributes &attr):
  Text(str, attr.fg, attr.bg, attr.attr)
  {}

  ///
  /// Constructor
  ///
  /// @param[in]  c           The character
  /// @param[in]  size        The size
  /// @param[in]  mixDefault  The mix default
  Text(Char c, const Vector &size, const AttributeMode &mixDefault=AttributeMode::default_);

  ///
  /// Get height.
  ///
  /// @return     Height
  [[nodiscard]] auto height() const -> Dim;

  ///
  /// Get width.
  ///
  /// @return     Width
  [[nodiscard]] auto width() const -> Dim;

  ///
  /// Get size.
  ///
  /// @return     Size
  [[nodiscard]] auto size() const -> Vector;

  ///
  /// String conversion operator.
  explicit operator string() const;

  [[nodiscard]] auto repr() const -> string;

  ///
  /// Get right-aligned Text
  ///
  /// @param[in]  width  The width
  ///
  /// @return     Text, right-adjusted to @c width characters
  [[nodiscard]] auto rightAligned(Dim width=DimLow) const -> Text;

  ///
  /// Get horizontally centered text
  ///
  /// @param[in]  width  The width
  ///
  /// @return     Centered Text
  [[nodiscard]] auto centered(Dim width=DimLow) const -> Text;

  ///
  /// Extend text to specified dimensions
  ///
  /// @param[in]  size  The size
  /// @param[in]  fill  Fill character
  void extend(const Vector &size, const Char &fill=Space);

  ///
  /// Fill text with character
  ///
  /// @param[in]  fill  Fill character
  auto fill(const Char &fill=Space, const Rectangle &area=RectangleMax) -> Rectangle;

  ///
  /// Put one text rectangle into another at a specific position
  ///
  /// @param[in]  other            The other
  /// @param[in]  position         The position
  /// @param[in]  mixDefaultMode   The mix default mode
  /// @param[in]  overrideMixMode  The override mix mode
  /// @param[in]  resetMixMode     The reset mix mode
  void patch(
    const Text &other,
    const Vector &position=Vector{0, 0},
    const AttributeMode &mixDefaultMode=AttributeMode::replace,
    const AttributeMode &overrideMixMode=AttributeMode::default_,
    const AttributeMode &resetMixMode=AttributeMode::default_
  );

  ///
  /// Put text into sub-rectangle
  ///
  /// @param[in]  other        The other
  /// @param[in]  area         The area
  /// @param[in]  mixDefault   The mix default
  /// @param[in]  overrideMix  The override mix
  /// @param[in]  resetMix     The reset mix
  void patchArea(
    const Text &other,
    const Rectangle &area=RectangleMax,
    const AttributeMode &mixDefault=AttributeMode::replace,
    const AttributeMode &overrideMix=AttributeMode::default_,
    const AttributeMode &resetMix=AttributeMode::default_
  );

  ///
  /// Resize text
  ///
  /// @param[in]  size  The size
  void resize(const Vector &size, const Char &fill);

  ///
  /// Set Char attributes.
  ///
  /// @param[in]  attr    The attribute
  /// @param[in]  area    The area
  /// @param[in]  setMix  The set mix
  void setAttr(const CharAttributes &attr, const Rectangle &area=RectangleMax, const AttributeMode &setMix=AttributeMode::default_);

  ///
  /// Get sub-rectangle.
  ///
  /// @param[in]  area  The area
  ///
  /// @return     The result of the array indexer
  auto operator [](const Rectangle &area) const -> Text;

  ///
  /// Get char at position.
  ///
  /// Negative elements in position are substituted with `text dimension - position`
  ///
  /// @param[in]  position  The position
  ///
  /// @return     Char (Null if position is outside of text)
  auto operator [](const Vector &position) const -> Char;

  ///
  /// Get reference to char at position.
  ///
  /// Negative elements in position are substituted with `text dimension - position`
  ///
  /// @param[in]  position  The position
  ///
  /// @return     Char (Null if position is outside of text)
  auto operator [](const Vector &position) -> Char &;

  ///
  /// Get char at position.
  ///
  /// If position is outside of text dimensions, throws range_error.
  ///
  /// Negative elements in position are substituted with `text dimension - position`
  ///
  /// @param[in]  position  The position
  ///
  /// @return     Char
  auto at(const Vector &position) -> Char &;

  ///
  /// Draw horizontal or vertical line.
  ///
  /// If endPosition is DimHigh the line is drawn all the way to the right or bottom
  ///
  /// @param[in]  line  The line
  ///
  /// @return     Rectangle spanning the area of modification
  auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> Rectangle;

  auto withLine(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> Text &;

  auto box(const Box &box=Box{}) -> vector<Rectangle>;

  auto withBox(const Box &box=Box{}) -> Text &;

  auto operator ==(const Text &other) const -> bool;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  vector<String> data; ///< text
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace jwezel
