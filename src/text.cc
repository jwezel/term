// Text

#include <array>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <fmt/color.h>
#include <utf8cpp/utf8.h>

#include <geometry.hh>
#include <rx-ranges.hh>
#include <string.hh>
#include <text.hh>

namespace jwezel {

using namespace std;
using namespace fmt;
using std::string_view;

const vector<pair<u4, QuadTuple>> _quadInitializer = {
  {0x2500, {0, 0, 1, 1, 0}}, // ─
  {0x2501, {0, 0, 2, 2, 0}}, // ━
  {0x2502, {1, 1, 0, 0, 0}}, // │
  {0x2503, {2, 2, 0, 0, 0}}, // ┃
  {0x2504, {0, 0, 1, 1, 3}}, // ┄
  {0x2505, {0, 0, 2, 2, 3}}, // ┅
  {0x2506, {1, 1, 0, 0, 3}}, // ┆
  {0x2507, {2, 2, 0, 0, 3}}, // ┇
  {0x2508, {0, 0, 1, 1, 4}}, // ┈
  {0x2509, {0, 0, 2, 2, 4}}, // ┉
  {0x250A, {1, 1, 0, 0, 4}}, // ┊
  {0x250B, {2, 2, 0, 0, 4}}, // ┋
  {0x250C, {0, 1, 0, 1, 0}}, // ┌
  {0x250D, {0, 1, 0, 2, 0}}, // ┍
  {0x250E, {0, 2, 0, 1, 0}}, // ┎
  {0x250F, {0, 2, 0, 2, 0}}, // ┏
  {0x2510, {0, 1, 1, 0, 0}}, // ┐
  {0x2511, {0, 1, 2, 0, 0}}, // ┑
  {0x2512, {0, 2, 1, 0, 0}}, // ┒
  {0x2513, {0, 2, 2, 0, 0}}, // ┓
  {0x2514, {1, 0, 0, 1, 0}}, // └
  {0x2515, {1, 0, 0, 2, 0}}, // ┕
  {0x2516, {2, 0, 0, 1, 0}}, // ┖
  {0x2517, {2, 0, 0, 2, 0}}, // ┗
  {0x2518, {1, 0, 1, 0, 0}}, // ┘
  {0x2519, {1, 0, 2, 0, 0}}, // ┙
  {0x251A, {2, 0, 1, 0, 0}}, // ┚
  {0x251B, {2, 0, 2, 0, 0}}, // ┛
  {0x251C, {1, 1, 0, 1, 0}}, // ├
  {0x251D, {1, 1, 0, 2, 0}}, // ┝
  {0x251E, {2, 1, 0, 1, 0}}, // ┞
  {0x251F, {1, 2, 0, 1, 0}}, // ┟
  {0x2520, {2, 2, 0, 1, 0}}, // ┠
  {0x2521, {2, 1, 0, 2, 0}}, // ┡
  {0x2522, {1, 2, 0, 2, 0}}, // ┢
  {0x2523, {2, 2, 0, 2, 0}}, // ┣
  {0x2524, {1, 1, 1, 0, 0}}, // ┤
  {0x2525, {1, 1, 2, 0, 0}}, // ┥
  {0x2526, {2, 1, 1, 0, 0}}, // ┦
  {0x2527, {1, 2, 1, 0, 0}}, // ┧
  {0x2528, {2, 2, 1, 0, 0}}, // ┨
  {0x2529, {2, 1, 2, 0, 0}}, // ┩
  {0x252A, {1, 2, 2, 0, 0}}, // ┪
  {0x252B, {2, 2, 2, 0, 0}}, // ┫
  {0x252C, {0, 1, 1, 1, 0}}, // ┬
  {0x252D, {0, 1, 2, 1, 0}}, // ┭
  {0x252E, {0, 1, 1, 2, 0}}, // ┮
  {0x252F, {0, 1, 2, 2, 0}}, // ┯
  {0x2530, {0, 2, 1, 1, 0}}, // ┰
  {0x2531, {0, 2, 2, 1, 0}}, // ┱
  {0x2532, {0, 2, 1, 2, 0}}, // ┲
  {0x2533, {0, 2, 2, 2, 0}}, // ┳
  {0x2534, {1, 0, 1, 1, 0}}, // ┴
  {0x2535, {1, 0, 2, 1, 0}}, // ┵
  {0x2536, {1, 0, 1, 2, 0}}, // ┶
  {0x2537, {1, 0, 2, 2, 0}}, // ┷
  {0x2538, {2, 0, 1, 1, 0}}, // ┸
  {0x2539, {2, 0, 2, 1, 0}}, // ┹
  {0x253A, {2, 0, 1, 2, 0}}, // ┺
  {0x253B, {2, 0, 2, 2, 0}}, // ┻
  {0x253C, {1, 1, 1, 1, 0}}, // ┼
  {0x253D, {1, 1, 2, 1, 0}}, // ┽
  {0x253E, {1, 1, 1, 2, 0}}, // ┾
  {0x253F, {1, 1, 2, 2, 0}}, // ┿
  {0x2540, {2, 1, 1, 1, 0}}, // ╀
  {0x2541, {1, 2, 1, 1, 0}}, // ╁
  {0x2542, {2, 2, 1, 1, 0}}, // ╂
  {0x2543, {2, 1, 2, 1, 0}}, // ╃
  {0x2544, {2, 1, 1, 2, 0}}, // ╄
  {0x2545, {1, 2, 2, 1, 0}}, // ╅
  {0x2546, {1, 2, 1, 2, 0}}, // ╆
  {0x2547, {2, 1, 2, 2, 0}}, // ╇
  {0x2548, {1, 2, 2, 2, 0}}, // ╈
  {0x2549, {2, 2, 2, 1, 0}}, // ╉
  {0x254A, {2, 2, 1, 2, 0}}, // ╊
  {0x254B, {2, 2, 2, 2, 0}}, // ╋
  {0x254C, {0, 0, 1, 1, 2}}, // ╌
  {0x254D, {0, 0, 2, 2, 2}}, // ╍
  {0x254E, {1, 0, 0, 0, 2}}, // ╎
  {0x254F, {2, 2, 0, 0, 2}}, // ╏
  {0x256D, {0, 1, 0, 1, 1}}, // ╭
  {0x256E, {0, 1, 1, 0, 1}}, // ╮
  {0x256F, {1, 0, 1, 0, 1}}, // ╯
  {0x2570, {1, 0, 0, 1, 1}}, // ╰
  {0x2574, {0, 0, 1, 0, 0}}, // ╴
  {0x2575, {1, 0, 0, 0, 0}}, // ╵
  {0x2576, {0, 0, 0, 1, 0}}, // ╶
  {0x2577, {0, 1, 0, 0, 0}}, // ╷
  {0x2578, {0, 0, 2, 0, 0}}, // ╸
  {0x2579, {2, 0, 0, 0, 0}}, // ╹
  {0x257A, {0, 0, 0, 2, 0}}, // ╺
  {0x257B, {0, 2, 0, 0, 0}}, // ╻
  {0x257C, {0, 0, 1, 2, 0}}, // ╼
  {0x257D, {1, 2, 0, 0, 0}}, // ╽
  {0x257E, {0, 0, 2, 1, 0}}, // ╾
  {0x257F, {2, 1, 0, 0, 0}}  // ╿
};

Rgb::Rgb(f4 r, f4 g, f4 b):
r(r), g(g), b(b)
{
  if (r == 0. && g == 0. && b == 0.) {
    r = -1.;
    g = -1.;
    b = -1.;
  }
}

Rgb::operator string() const {
  if (r == -1 && g == -1 && b == -1)
    return "RgbNone";
  return format("Rgb(r={}, g={}, b={})", r, g, b);
}

Hsv::operator string() const {
  if (h == -1 && s == -1 && v == -1)
    return "HsvNone";
  return format("Hsv(h={}, s={}, v={})", h, s, v);
}

Rgb::operator Hsv() const {
  f4 min_, max_, delta;
  Hsv result;

  min_ = r < g? r: g;
  min_ = min_ < b? min_: b;
  max_ = r > g? r: g;
  max_ = max_ > b? max_: b;
  result.v = max_; // v
  delta = max_ - min_;
  if (delta < FLT_EPSILON) {
    result.s = 0;
    result.h = NAN; // undefined, maybe nan?
    return result;
  }
  if (max_ > 0.) {
    // NOTE: if Max is == 0, this divide would cause a crash
    result.s = delta / max_; // s
  } else {
    result.s = 0.;
    result.h = NAN; // undefined
    return result;
  }
  if (r >= max_) {
    // > is bogus, just keeps compiler happy
    result.h = (g - b) / delta; // between yellow & magenta
  } else {
    if (g >= max_) {
      result.h = 2. + (b - r) / delta; // between cyan & yellow
    }
    else {
      result.h = 4. + (r - g) / delta; // between magenta & cyan
    }
  }
  result.h *= 60.;
  if (result.h < 0.) {
    result.h += 360.;
  }
  return result;
}

Hsv::operator Rgb() const {
  f4 hh, p, q, t, ff;
  i8 i;
  Rgb result;

  if (s <= 0.) {
    result.r = v;
    result.g = v;
    result.b = v;
    return result;
  }
  hh = h;
  if (hh >= 360.) {
    hh = 0.;
  }
  hh = hh / 60.;
  i = i8(hh);
  ff = hh - f4(i);
  p = v * (1. - s);
  q = v * (1. - (s * ff));
  t = v * (1. - (s * (1. - ff)));
  switch (i) {
    case 0:
    result.r = v;
    result.g = t;
    result.b = p;
    break;

    case 1:
    result.r = q;
    result.g = v;
    result.b = p;
    break;

    case 2:
    result.r = p;
    result.g = v;
    result.b = t;
    break;

    case 3:
    result.r = p;
    result.g = q;
    result.b = v;
    break;

    case 4:
    result.r = t;
    result.g = p;
    result.b = v;
    break;

    default:
    result.r = v;
    result.g = p;
    result.b = q;
  }
  return result;
}

Rgb Rgb::operator |(const Rgb &other) const {
  // Default color
  if (*this == RgbNone or *this == RgbTransparent) {
    return Rgb(other);
  } else {
    return Rgb(*this);
  }
}

bool Rgb::operator ==(const Rgb &other) const {
  return r == other.r and g == other.g and b == other.b;
}

bool Rgb::operator !=(const Rgb &other) const {
  return r != other.r or g != other.g or b != other.b;
}

bool Hsv::operator ==(const Hsv &other) const {
  return h == other.h and s == other.s and v == other.v;
}

bool Hsv::operator !=(const Hsv &other) const {
  return h != other.h or s != other.s or v != other.v;
}

///
/// Mix two colors
///
/// @param[in]  color1  Color 1
/// @param[in]  color2  Color 2
///
/// @return     Color 1 and color 2 mixed
Rgb Rgb::operator +(const Rgb &color2) const {
  Rgb
    color1_ = *this | color2,
    color2_ = color2 | *this;

  if (color1_ == RgbNone or color1_ == RgbTransparent)
    return color2_;
  else if (color2_ == RgbNone or color2_ == RgbTransparent)
    return color1_;
  else
    return {(color1_.r + color2_.r) / 2, (color1_.g + color2_.g) / 2, (color1_.b + color2_.b) / 2};
}

string attrToString(Attributes attr) {
  string result, sep;

  if (attr & bold) {
    result = "bold";
    sep = ", ";
  }
  if (attr & underline) {
    result += sep;
    result += "underline";
    sep = ", ";
  }
  if (attr & reverse) {
    result += sep;
    result += "reverse";
    sep = ", ";
  }
  if (attr & blink) {
    result += sep;
    result += "blink";
    sep = ", ";
  }
  return result;
}

CharAttributes::CharAttributes(const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix):
fg(fg), bg(bg), attr(attr), mix(mix)
{}

vector<string> AttributeMode2String {
  "default_",
  "merge",
  "mix",
  "replace",
  "ignore"
};

CharAttributes::operator string() const {
  return format("(fg={}, bg={}, attr={}, mix={})", string(fg), string(bg), attrToString(attr), AttributeMode2String[mix]);
}

bool CharAttributes::operator ==(const CharAttributes &other) const {
  return fg == other.fg && bg == other.bg && attr == other.attr && mix == other.mix;
}

bool CharAttributes::operator !=(const CharAttributes &other) const {
  return fg != other.fg && bg != other.bg && attr != other.attr && mix != other.mix;
}

Char::operator string() const {
  string buffer;
  utf8::utf32to8(&rune, &rune + 1, back_inserter(buffer));
  return format("'{}'{}", buffer, string(attributes));
}

Text::Text(string_view str, const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix) {
  if (str.size() > 0) {
    SplitString split(string(str), "\n");
    vector<string_view> lines(split.begin(), split.end());
    size_t width = (
      lines
      | rx::transform([](string_view line) {return line.size();})
      | rx::max()
    ).value_or(0);
    data.reserve(lines.size());
    for (string_view line: lines) {
      String iline;
      iline.reserve(line.size());
      // Copy line
      for (const char &c: line) {
        iline.emplace_back(c, fg, bg, attr, mix);
      }
      // Pad line
      for (size_t i = line.size(); i < width; ++i) {
        iline.emplace_back(' ', fg, bg, attr, mix);
      }
      // Add line
      data.emplace_back(iline);
    }
  }
}

Dim Text::height() const {
  return data.size();
}

Dim Text::width() const {
  return data[0].size();
}

Vector Text::size() const {
  return Vector(width(), height());
}

///
/// Convert String to std::string
///
/// @param[in]  s     String to convert
///
/// @return     std::string_view
const string asString(const String &s) {
  string result;
  vector<Unicode> runes;
  for (const Char &char_: s) {
    runes.push_back(char_.rune);
  }
  utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  return result;
}

Text::operator string() const {
  // String representation
  string result;
  for (const String &line: data) {
    vector<Unicode> runes;
    for (const Char &char_: line) {
      runes.push_back(char_.rune);
    }
    runes.push_back('\n');
    utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  }
  return result;
}

string Text::repr() const {
  // String representation
  string result{"\"\"\"\n"};
  for (const String &line: data) {
    vector<Unicode> runes;
    for (const Char &char_: line) {
      runes.push_back(char_.rune < 32? char_.rune + 0x2400: char_.rune);
    }
    runes.push_back(0x240a);
    runes.push_back('\n');
    utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  }
  result += "\"\"\"\n";
  return result;
}

Text Text::rightAligned(Dim width) {
  width = width == DimLow? width: width;
  if (width < 0)
    throw range_error("width < 0");
  Text result;
  for (String line: data) {
    // Trim line on right
    auto i{line.rbegin()};
    while (i != line.rend() && i->rune == ' ')
      ++i;
    line.resize(line.rend() - i);
    // Space-fill on left
    String filled(width, Char(' ').withAttr(line[0].attributes));
    copy(line.begin(), line.end(), back_inserter(filled));
    // Trim line on left
    String newLine{filled.end() - width, filled.end()};
    // Add line
    result.data.push_back(newLine);
  }
  return result;
}

Text Text::centered(Dim width) {
  width = width == DimLow? width: width;
  if (width < 0)
    throw range_error("width < 0");
  Text result;
  for (const String &line: data) {
    // Trim line
    auto rend{line.rbegin()};
    while (rend != line.rend() && rend->rune == ' ') {
      ++rend;
    }
    auto end = rend.base();
    auto begin{line.begin()};
    while (begin != line.end() && begin->rune == ' ') {
      ++begin;
    }
    auto size = end - begin;
    if (size > width) {
      begin += (size - 1) / 2 - (width - 1) / 2;
      end = begin + width;
      result.data.push_back(String(begin, end));
    } else {
      String newLine(width, Char(' ').withAttr(begin->attributes));
      auto pos = (width - 1) / 2 - (size - 1) / 2;
      copy(begin, end, newLine.begin() + pos);
      result.data.push_back(newLine);
    }
  }
  return result;
}

bool Text::operator ==(const Text &other) const {
  return data == other.data;
}

Char::Char():
rune(0), attributes(RgbNone, RgbNone, {}, default_)
{}

Char::Char(Unicode c, const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix):
rune(c), attributes(fg, bg, attr, mix)
{}

Char::Char(Unicode c, const CharAttributes &attr):
rune(c), attributes(attr)
{}

Char Char::withAttr(const CharAttributes &attr) const{
  // Get a char with attributes set
  Char result = *this;
  result.attributes = attr;
  return result;
}

bool Char::operator ==(const Char &other) const {
  return rune == other.rune and attributes == other.attributes;
}

bool Char::operator !=(const Char &other) const {
  return rune != other.rune or !(attributes == other.attributes);
}

Draw::Draw(u1 strength, u1 dash, bool roundedCorners):
strength(strength), dash(dash), roundedCorners(roundedCorners)
{}

Box::Box(const Rectangle &area, u1 strength, u1 dash, bool roundedCorners):
Draw(strength, dash, roundedCorners),
area(area)
{}

Text::Text(Char c, const Vector &size, const AttributeMode &mixDefault) {
  if (c.attributes.mix == default_) {
    c.attributes.mix = mixDefault;
  }
  /// Create a matrix of size @arg size filled with @arg c
  auto _(rx::fill_n(size.y, rx::fill_n(size.x, c) | rx::to_vector()) | rx::to_vector());
  data.assign(_.begin(), _.end());
}

Char Char::combine(
  const Char          &other,
  const AttributeMode &mixDefault,
  const AttributeMode &overrideMix,
  const AttributeMode &resetMix
) {
  // Combine characters
  auto mix_ =
    (overrideMix == default_)?
      (other.attributes.mix == default_)?
        mixDefault
      :
        other.attributes.mix
    :
      overrideMix;
  auto rune_ = (other.rune == NoneRune)? rune: other.rune;
  Char result;
  switch (mix_) {

    case merge:
    result = Char(
      rune_,
      attributes.fg | other.attributes.fg,
      attributes.bg | other.attributes.bg,
      attributes.attr | other.attributes.attr,
      merge
    );
    break;

    case mix:
    result = Char(
      rune_,
      attributes.fg + other.attributes.fg,
      attributes.bg + other.attributes.bg,
      attributes.attr | other.attributes.attr,
      mix
    );
    break;

    case replace:
    result = other;
    break;

    case ignore:
    result = Char(
      rune_,
      attributes.fg,
      attributes.bg,
      attributes.attr,
      attributes.mix
    );
    break;

    case default_:
    throw domain_error("source mix must not be Default");
    break;
  }
  if (resetMix != default_) {
    result.attributes.mix = resetMix;
  }
  return result;
}

void Text::extend(const Vector &size, const Char &fill) {
  if (width() > 0) {
    for (Dim l = 0; l < height(); ++l) {
      fill_n(back_inserter(data[l]), size.x - data[l].size(), fill);
    }
  }
  fill_n(back_inserter(data), size.y - data.size(), rx::fill_n(size.x, fill) | rx::to_vector());
}

void Text::patch(
  const Text &other,
  const Vector &position,
  const AttributeMode &mixDefaultMode,
  const AttributeMode &overrideMixMode,
  const AttributeMode &resetMixMode
) {
  assert(data.data() != other.data.data());
  Dim
    xdest = max(Dim(0), position.x),
    xbegin = xdest - position.x,
    ydest = max(Dim(0), position.y),
    ybegin = ydest - position.y,
    width_ = min(width() - xdest, other.width() - xbegin),
    height_ = min(height() - ydest, other.height() - ybegin);
  for (Dim l = 0; l < height_; ++l) {
    Dim owidth = min(Dim(other.data[ybegin + l].size()), width_);
    if (xdest + owidth > 0) {
      auto dest = data[ydest + l].begin() + xdest;
      auto source = other.data[ybegin + l].begin();
      while (dest != data[ydest + l].begin() + xdest + owidth) {
        *dest++ = dest->combine(*source++, mixDefaultMode, overrideMixMode, resetMixMode);
      }
    }
  }
}

void Text::patchArea(
  const Text          &other,
  const Rectangle     &area,
  const AttributeMode &mixDefault,
  const AttributeMode &overrideMix,
  const AttributeMode &resetMix
) {
  if (data.data() == other.data.data())
    throw range_error("This and other are the same");
  const Rectangle thisDimensions{0, 0, width(), height()};
  auto area_{area.defaultTo(thisDimensions)};
  if ((area_ & Rectangle{0, 0, this->width(), this->height()}) != area_)
    throw runtime_error("area must be within text");
  auto data_ = other.data.begin();
  for (Dim l = 0; l < area_.height(); ++l) {
    auto
      source = data_->begin(),
      sourceEnd = data_->end();
    auto
      dest = data[area_.y1 + l].begin() + area_.x1,
      limit = data[area_.y1 + l].begin() + size_t(area_.x2);
    while (dest != limit && source != sourceEnd) {
      *dest++ = dest->combine(*source++, mixDefault, overrideMix, resetMix);
    }
    ++data_;
  }
}

void Text::setAttr(const CharAttributes &attr, const Rectangle &area, const AttributeMode &setMix) {
  Rectangle limits = {0, 0, width(), height()};
  optional<Rectangle> area_ = area.defaultTo(limits) & limits;
  if (area_) {
    Rectangle area = area_.value();
    for (Dim l = area.y1; l < area.y2; ++l) {
      for (Dim c = area.x1; c < area.x2; ++c) {
        data[l][c] = data[l][c].combine(Char(NoneRune, attr), default_, ignore, setMix);
      }
    }
  }
}

Text Text::operator [](const Rectangle &area) const {
  optional<Rectangle> area_ = area & Rectangle(0, 0, width(), height());
  Text result;
  if (area_) {
    Rectangle area = area_.value();
    for (Dim line = area.y1; line < min(height(), area.y2); ++line) {
      result.data.push_back(
        String(
          data[line].begin() + area.x1,
          data[line].begin() + max(area.x1, min(area.x2, Dim(data[line].size())))
        )
      );
    }
  }
  return result;
}

Char Text::operator [](const Vector &position) const {
  Vector position_ = size().position(position);
  if (position_.y < height() && position_.x < width()) {
    return data[position_.y][position_.x];
  }
  return Null;
}

Char &Text::operator [](const Vector &position) {
  Vector position_ = size().position(position);
  if (position_.y < height() && position_.x < width()) {
    return data[position_.y][position_.x];
  }
  throw range_error(fmt::format("Position {} is outside text dimensions {}", string(position), string(size())));
}

Char &Text::at(const Vector &position) {
  Vector position_ = size().position(position);
  if (position_.y < height() && position_.x < width()) {
    return data[position_.y][position_.x];
  }
  throw range_error(fmt::format("Position {} is outside text dimensions {}", string(position), string(size())));
}

static unordered_map<Unicode, QuadTuple> initializedBox2Quad() {
  unordered_map<Unicode, QuadTuple> result(_quadInitializer.size() * 2);
  // TODO: try copy()
  for (auto &[rune, quad]: _quadInitializer) {
    result[rune] = quad;
  }
  return result;
}

static QuadValue _QuadTuple2Value(const QuadTuple &tuple) {
  return tuple[0] << 9 | tuple[1] << 7 | tuple[2] << 5 | tuple[3] << 3 | tuple[4];
}

static unordered_map<QuadValue, Unicode> initializedQuad2Box() {
  unordered_map<QuadValue, Unicode> result;
  // TODO: try copy()
  for (auto &[rune, quad]: _quadInitializer) {
    result[_QuadTuple2Value(quad)] = rune;
  }
  return result;
}

void Char::drawLineChar(const Segments &segments, u1 dash, bool roundedCorners) {
  static const unordered_map<Unicode, QuadTuple> box2Quad{initializedBox2Quad()};
  static unordered_map<QuadValue, Unicode> quad2Box{initializedQuad2Box()}; // const not supported

  try {
    QuadTuple quad{0, 0, 0, 0, 0};
    auto quadp = box2Quad.find(rune);
    if (quadp != box2Quad.end()) {
      quad = quadp->second;
    }
    for (int i = 0; i < 4; ++i) {
      if (segments[i] != 0)
        quad[i] = segments[i];
    }
    if (((quad[north] == 1) == (quad[south] == 1)) and (quad[west] == 1 xor quad[east] == 1) and roundedCorners) {
      // On corners of strength 1 lines, `roundedCorners` can be set
      quad[mode] = 1;
    } else if (
      quad[north] == quad[south] and
      quad[west] == quad[east] and
      (quad[north] != 0) != (quad[west] != 0) and
      dash > 1 and dash < 5
    ) {
      // On straight lines, `dash`ing can be set
      quad[mode] = dash;
    }
    rune = quad2Box[_QuadTuple2Value(quad)];
  } catch (const out_of_range &) {
    // ignore
  }
}


Rectangle drawHorizontalLine(
  Text &text,
  const Vector &initialPosition,
  Dim endPosition,
  u1 strength,
  u1 dash,
  bool roundedCorners,
  bool extendBegin,
  bool extendEnd
) {
  Dim e = (endPosition == DimHigh? text.width(): endPosition + (endPosition < 0? text.width(): 0));
  Dim b = initialPosition.x;
  for (Dim p = b; p < e; ++p)
    if (p == b and not extendBegin)
      text.at(Vector(p, initialPosition.y)).drawLineChar({0, 0, 0, strength}, dash, roundedCorners);
    else if (p == e - 1 and not extendEnd)
      text.at(Vector(p, initialPosition.y)).drawLineChar({0, 0, strength, 0}, dash, roundedCorners);
    else
      text.at(Vector(p, initialPosition.y)).drawLineChar({0, 0, strength, strength}, dash, roundedCorners);
  return Rectangle(initialPosition.x, initialPosition.y, initialPosition.x + e + 1, initialPosition.y + 1);
}

Rectangle drawVerticalLine(
  Text &text,
  const Vector &initialPosition,
  Dim endPosition,
  u1 strength,
  u1 dash,
  bool roundedCorners,
  bool extendBegin,
  bool extendEnd
) {
  Dim e = (endPosition == DimHigh? text.height(): endPosition + (endPosition < 0? text.height(): 0));
  Dim b = initialPosition.y;
  for (Dim p = initialPosition.y; p < e; ++p)
    if (p == b and not extendBegin)
      text.at(Vector(initialPosition.x, p)).drawLineChar({0, strength, 0, 0}, dash, roundedCorners);
    else if (p == e - 1 and not extendEnd)
      text.at(Vector(initialPosition.x, p)).drawLineChar({strength, 0, 0, 0}, dash, roundedCorners);
    else
      text.at(Vector(initialPosition.x, p)).drawLineChar({strength, strength, 0, 0}, dash, roundedCorners);
  return Rectangle(initialPosition.x, initialPosition.y, initialPosition.x + 1, initialPosition.y + e + 1);
}

Rectangle Text::line(
    const Vector &position,
    Dim endPosition,
    Orientation orientation,
    u1 strength,
    u1 dash,
    bool roundedCorners,
    bool extendBegin,
    bool extendEnd
) {
  if (strength < 1 or strength > 2)
    throw range_error(fmt::format("Invalid strength: {}", strength));
  if (orientation == Horizontal)
    return drawHorizontalLine(
      *this,
      position,
      endPosition,
      strength,
      dash,
      roundedCorners,
      extendBegin,
      extendEnd
    );
  else
    return drawVerticalLine(
      *this,
      position,
      endPosition,
      strength,
      dash,
      roundedCorners,
      extendBegin,
      extendEnd
    );
}

///
/// Limit value
///
/// If value is negative it denotes "from the right".
///
/// @param[in]  v     Value
/// @param[in]  m     Maximum
///
/// @return     Value clipped to maximum
int limit(int v, int m) {
  return v < 0? v + m: min(v, m - 1);
}

vector<Rectangle> Text::box(const Box &box) {
  vector<Rectangle> result;
  Rectangle area = box.area.defaultTo(Rectangle(0, 0, width(), height()));
  return vector<Rectangle>{
    // top
    line(
      Vector(area.x1, area.y1),
      limit(area.x2, area.width() + 1),
      Horizontal,
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // bottom
    line(
      Vector(area.x1, area.y2 - 1),
      limit(area.x2, area.width() + 1),
      Horizontal,
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // left
    line(
      Vector(area.x1, area.y1),
      limit(area.y2, area.height() + 1),
      Vertical,
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // right
    line(
      Vector(area.x2 - 1, area.y1),
      limit(area.y2, area.height() + 1),
      Vertical,
      box.strength,
      box.dash,
      box.roundedCorners
    )
  };
}

} // namespace
