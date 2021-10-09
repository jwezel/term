///
/// @defgroup   TEXT text
///
/// This file implements the Text class.
///
/// @author     J
/// @date       2020

#pragma once

#include <array>
#include <vector>
#include <string>

#include <rx-ranges.hh>
#include <basic.hh>
#include <geometry.hh>

namespace jwezel {

using namespace std;

typedef char32_t Unicode;

const Unicode NoneRune = 0xffffffff;

///
/// RGB color attributes.
struct Rgb {
  f4 r; ///< red (0 .. 1)
  f4 g; ///< green (0 .. 1)
  f4 b; ///< blue (0 .. 1)

  Rgb(f4 r=-1., f4 g=0., f4 b=0.);

  operator string() const;

  operator struct Hsv() const;

  Rgb(const Rgb & color) = default; //: r(color.r), g(color.g), b(color.b) {};

  Rgb operator |(const Rgb &other) const;

  bool operator ==(const Rgb &other) const;

  bool operator !=(const Rgb &other) const;

  Rgb operator +(const Rgb &color2) const;
};

///
/// HSV color attributes.
struct Hsv {
  f4 h; ///< hue (angle in decimal degrees (0 .. 360))
  f4 s; ///< saturation (0 .. 1)
  f4 v; ///< value (0 .. 1)

  Hsv(): h(-1), s(-1), v(-1) {}

  Hsv(f4 h, f4 s, f4 v): h(h), s(s), v(v) {}

  operator string() const;

  operator Rgb() const;

  bool operator ==(const Hsv &other) const;

  bool operator !=(const Hsv &other) const;
};

const Rgb
  RgbTransparent =  {-2.0, -2.0, -2.0},
  RgbNone =         {-1.0, -1.0, -1.0},
  RgbWhite =        {1.0, 1.0, 1.0},
  RgbBlack =        {0.0, 0.0, 0.0},
  RgbGray1 =        {0.1, 0.1, 0.1},
  RgbGray2 =        {0.2, 0.2, 0.2},
  RgbGray3 =        {0.3, 0.3, 0.3},
  RgbGray4 =        {0.4, 0.4, 0.4},
  RgbGray5 =        {0.5, 0.5, 0.5},
  RgbGray6 =        {0.6, 0.6, 0.6},
  RgbGray7 =        {0.7, 0.7, 0.7},
  RgbGray8 =        {0.8, 0.8, 0.8},
  RgbGray9 =        {0.9, 0.9, 0.9};

///
/// Attributes for the Char type.
typedef u8 Attributes;

const Attributes
  bold = 1,
  underline = 2,
  reverse = 4,
  blink = 8;

///
/// Attribute combining mode for the Char type.
enum AttributeMode {
  default_, ///< default
  merge,
  mix,
  replace,
  ignore
};

///
/// Alignment
enum Alignment {
  left,
  center,
  right
};

extern vector<string> AttributeMode2String;

///
/// This struct describes character attributes.
struct CharAttributes {
  Rgb           fg;   ///< foreground
  Rgb           bg;   ///< background
  Attributes    attr; ///< attributes
  AttributeMode mix;  ///< attribute mapping

  ///
  /// Constructor
  ///
  /// @param[in]  fg    The foreground
  /// @param[in]  bg    The background
  /// @param[in]  attr  The attribute
  /// @param[in]  mix   The mix
  CharAttributes(
    const Rgb &fg=RgbNone,
    const Rgb &bg=RgbNone,
    const Attributes &attr={},
    const AttributeMode &mix=AttributeMode::merge
  );

  ///
  /// String conversion operator.
  ///
  operator string() const;

  ///
  /// Equality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the equality
  ///
  bool operator ==(const CharAttributes &other) const;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     The result of the inequality
  ///
  bool operator !=(const CharAttributes &other) const;
};

///
/// Line orientation
enum Orientation {
  Horizontal,
  Vertical
};

///
/// Line segments
typedef u1 Segments[8];

///
/// Quad tuple: north, south, west, east, mode
typedef array<u1, 5> QuadTuple;

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
/// Quad value (unsigned with encoded bit fields)
typedef u2 QuadValue;

///
/// Line drawing attributes.
struct Draw {
  u1    strength;
  u1    dash;
  bool  roundedCorners;

  ///
  /// Constructor
  ///
  /// @param[in]  strength        The strength
  /// @param[in]  dash            The dash
  /// @param[in]  roundedCorners  The rounded corners
  Draw(u1 strength, u1 dash, bool roundedCorners);
};

///
/// This struct describes a box.
struct Box: public Draw {
  Rectangle area;

  Box(const Rectangle &area=RectangleMax, u1 strength=1, u1 dash=0, bool roundedCorners=false);
};

///
/// This struct describes a character.
struct Char {
  Unicode         rune;       ///< Unicode code point
  CharAttributes  attributes; ///< attributes

  Char();
  ///
  /// Constructor
  ///
  /// @param[in]  c     Character
  /// @param[in]  fg    The foreground
  /// @param      bg    The background
  /// @param[in]  attr  The attribute
  /// @param[in]  mix   The mix
  Char(Unicode c, const Rgb &fg=RgbNone, const Rgb &bg=RgbNone, const Attributes &attr={}, const AttributeMode &mix=default_);

  ///
  /// Constructor
  ///
  /// @param[in]  c     Character
  /// @param[in]  attr  The attribute
  Char(Unicode c, const CharAttributes &attr);

  ///
  /// String conversion operator.
  operator string() const;

  ///
  /// Char with attributes replaced
  ///
  /// @param[in]  attr  The attributes
  ///
  /// @return     Char with new attributes
  Char withAttr(const CharAttributes &attr);

  Char combine(
    const Char &other,
    const AttributeMode &mixDefaultMode=merge,
    const AttributeMode &overrideMixMode=default_,
    const AttributeMode &resetMixMode=default_
  );

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
  bool operator ==(const Char &other) const;

  ///
  /// Inequality operator.
  ///
  /// @param[in]  other  The other
  ///
  /// @return     Whether Chars are unequal
  ///
  bool operator !=(const Char &other) const;
};

const Char
  Space(32),
  Null(0),
  Transparent(32, RgbTransparent, RgbTransparent);

///
/// This struct describes a rectangle of text.
struct Text {
  typedef vector<Char> String;

  vector<String> data; ///< text

  Text() {}

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
  Text(
    string_view str,
    const Rgb &fg=RgbNone,
    const Rgb &bg=RgbNone,
    const Attributes &attr={},
    const AttributeMode &mix=default_
  );

  ///
  /// Constructor
  ///
  /// @param[in]  c           The character
  /// @param[in]  size        The size
  /// @param[in]  mixDefault  The mix default
  Text(Char c, const Vector &size, const AttributeMode &mixDefault=merge);

  ///
  /// Get height.
  ///
  /// @return     Height
  Dim height() const;

  ///
  /// Get width.
  ///
  /// @return     Width
  Dim width() const;

  ///
  /// Get size.
  ///
  /// @return     Size
  Vector size() const;

  ///
  /// String conversion operator.
  operator string() const;

  string repr() const;

  ///
  /// Get right-aligned Text
  ///
  /// @param[in]  width  The width
  ///
  /// @return     Text, right-adjusted to @c width characters
  Text rightAligned(Dim width=DimLow);

  Text centered(Dim width=DimLow, Alignment alignment=left);

  void extend(const Vector &size, const Char &fill=Space);

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
    const Vector &position={0, 0},
    const AttributeMode &mixDefaultMode=merge,
    const AttributeMode &overrideMixMode=default_,
    const AttributeMode &resetMixMode=default_
  );

  ///
  /// Get range to sub-rectangle
  ///
  /// @param[in]  area  The area
  ///
  /// @return     range
  auto iter(const Rectangle &area=RectangleMax) const;

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
    const AttributeMode &mixDefault=merge,
    const AttributeMode &overrideMix=default_,
    const AttributeMode &resetMix=default_
  );

  ///
  /// Set Char attributes.
  ///
  /// @param[in]  attr    The attribute
  /// @param[in]  area    The area
  /// @param[in]  setMix  The set mix
  void setAttr(const CharAttributes &attr, const Rectangle &area=RectangleMax, const AttributeMode &setMix=default_);

  ///
  /// Get sub-ractangle.
  ///
  /// @param[in]  area  The area
  ///
  /// @return     The result of the array indexer
  Text operator [](const Rectangle &area) const;

  ///
  /// Get char at position.
  ///
  /// @param[in]  position  The position
  ///
  /// @return     Char (Null if position is outside of text)
  Char operator [](const Vector &position) const;

  ///
  /// Get char at position.
  ///
  /// If position is outside of text dimensions, throws range_error.
  ///
  /// @param[in]  position  The position
  ///
  /// @return     Char
  Char at(const Vector &position);

  ///
  /// Draw horizontal or vertical line.
  ///
  /// If endPosition is DimHigh the line is drawn all the way to the right or bottom
  ///
  /// @param[in]  line  The line
  ///
  /// @return     Rectangle spanning the area of modification
  Rectangle line(
    const Vector &position,
    Dim endPosition,
    Orientation orientation,
    u1 strength,
    u1 dash,
    bool roundedCorners,
    bool extendBegin,
    bool extendEnd
  );

  vector<Rectangle> box(const Box &box);

  bool operator ==(const Text &other) const;
};

}
