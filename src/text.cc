// Text

#include <cassert>
#include <geometry.hh>
#include <string.hh>
#include <text.hh>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include <utf8cpp/utf8.h>
#include <utf8cpp/utf8/cpp11.h>

namespace jwezel {

using
  std::domain_error,
  std::min,
  std::max,
  std::out_of_range,
  std::pair,
  std::runtime_error,
  std::unordered_map;
using std::format;
using std::string_view;
using std::ranges::views::split;
using std::ranges::views::transform;

// NOLINTNEXTLINE(cert-err58-cpp)
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

Rgb::Rgb(f4 r, f4 g, f4 b) noexcept:
r(r), g(g), b(b)
{
  if (r == 0. && g == 0. && b == 0.) {
    r = -1.;
    g = -1.;
    b = -1.;
  }
}

Rgb::operator string() const {
  if (r == -1 && g == -1 && b == -1) {
    return "RgbNone";
  }
  return format("Rgb(r={}, g={}, b={})", r, g, b);
}

Hsv::operator string() const {
  if (h == -1 && s == -1 && v == -1) {
    return "HsvNone";
  }
  return format("Hsv(h={}, s={}, v={})", h, s, v);
}

Rgb::operator Hsv() const noexcept {
  f4
    min_ = NAN,
    max_ = NAN,
    delta = NAN;
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
      result.h = 2. + (b - r) / delta; // between cyan & yellow  NOLINT
    }
    else {
      result.h = 4. + (r - g) / delta; // between magenta & cyan  NOLINT
    }
  }
  result.h *= 60.; // NOLINT
  if (result.h < 0.) {
    result.h += 360.; // NOLINT
  }
  return result;
}

Hsv::operator Rgb() const {
  f4
    hh = NAN,
    pp = NAN,
    qq = NAN,
    tt = NAN,
    ff = NAN;
  i8 ii = 0;
  Rgb result;

  if (s <= 0.) {
    result.r = v;
    result.g = v;
    result.b = v;
    return result;
  }
  hh = h;
  // NOLINTBEGIN
  if (hh >= 360.) {
    hh = 0.;
  }
  hh = hh / 60.;
  ii = i8(hh);
  ff = hh - f4(ii);
  pp = v * (1. - s);
  qq = v * (1. - (s * ff));
  tt = v * (1. - (s * (1. - ff)));
  // NOLINTEND
  switch (ii) {
    case 0:
    result.r = v;
    result.g = tt;
    result.b = pp;
    break;

    case 1:
    result.r = qq;
    result.g = v;
    result.b = pp;
    break;

    case 2:
    result.r = pp;
    result.g = v;
    result.b = tt;
    break;

    case 3:
    result.r = pp;
    result.g = qq;
    result.b = v;
    break;

    case 4:
    result.r = tt;
    result.g = pp;
    result.b = v;
    break;

    default:
    result.r = v;
    result.g = pp;
    result.b = qq;
  }
  return result;
}

auto Rgb::operator |(const Rgb &other) const -> Rgb {
  // Default color
  if (*this == RgbNone or *this == RgbTransparent) {
    return Rgb{other};
  }
  return Rgb{*this};
}

auto Rgb::operator ==(const Rgb &other) const -> bool {
  return r == other.r and g == other.g and b == other.b;
}

auto Rgb::operator !=(const Rgb &other) const -> bool {
  return r != other.r or g != other.g or b != other.b;
}

auto Hsv::operator ==(const Hsv &other) const -> bool {
  return h == other.h and s == other.s and v == other.v;
}

auto Hsv::operator !=(const Hsv &other) const -> bool {
  return h != other.h or s != other.s or v != other.v;
}

///
/// Mix two colors
///
/// @param[in]  color1  Color 1
/// @param[in]  color2  Color 2
///
/// @return     Color 1 and color 2 mixed
auto Rgb::operator +(const Rgb &color2) const -> Rgb {
  const Rgb
    color1_ = *this | color2,
    color2_ = color2 | *this;

  if (color1_ == RgbNone or color1_ == RgbTransparent) {
    return color2_;
  }
  if (color2_ == RgbNone or color2_ == RgbTransparent) {
    return color1_;
  }
  return Rgb{(color1_.r + color2_.r) / 2, (color1_.g + color2_.g) / 2, (color1_.b + color2_.b) / 2};
}

auto attrToString(Attributes attr) -> string {
  string result, sep;

  if ((attr & bold) != 0U) {
    result = "bold";
    sep = ", ";
  }
  if ((attr & underline) != 0U) {
    result += sep;
    result += "underline";
    sep = ", ";
  }
  if ((attr & reverse) != 0U) {
    result += sep;
    result += "reverse";
    sep = ", ";
  }
  if ((attr & blink) != 0U) {
    result += sep;
    result += "blink";
    sep = ", ";
  }
  return result;
}

CharAttributes::CharAttributes(const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix):
fg(fg), bg(bg), attr(attr), mix(mix)
{}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,cert-err58-cpp)
const vector<string> AttributeMode2String {
  "AttributeMode::default_",
  "merge",
  "mix",
  "replace",
  "ignore"
};

CharAttributes::operator string() const {
  return format("(fg={}, bg={}, attr={}, mix={})", string(fg), string(bg), attrToString(attr), AttributeMode2String[vector<string>::size_type(mix)]);
}

auto CharAttributes::operator ==(const CharAttributes &other) const -> bool {
  return fg == other.fg && bg == other.bg && attr == other.attr && mix == other.mix;
}

auto CharAttributes::operator !=(const CharAttributes &other) const -> bool {
  return fg != other.fg && bg != other.bg && attr != other.attr && mix != other.mix;
}

Char::Char():
rune(0), attributes(RgbNone, RgbNone, {}, AttributeMode::default_)
{}

Char::Char(Unicode c, const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix):
rune(c), attributes(fg, bg, attr, mix)
{}

Char::Char(Unicode c, const CharAttributes &attr):
rune(c), attributes(attr)
{}

auto Char::withAttr(const CharAttributes &attr) const -> Char {
  // Get a char with attributes set
  Char result = *this;
  result.attributes = attr;
  return result;
}

auto Char::operator ==(const Char &other) const -> bool {
  return rune == other.rune and attributes == other.attributes;
}

auto Char::operator !=(const Char &other) const -> bool {
  return rune != other.rune or !(attributes == other.attributes);
}

Char::operator string() const {
  string buffer;
  utf8::utf32to8(&rune, &rune + 1, back_inserter(buffer));
  return format("'{}'{}", buffer, string(attributes));
}

auto Char::utf8() const -> string {
  string result;
  utf8::utf32to8(&rune, &rune + 1, back_inserter(result));
  return result;
}

///
/// Convert String to std::string
///
/// @param[in]  s     String to convert
///
/// @return     std::string_view
auto asString(const String &s) -> string {
  vector<Unicode> runes;
  runes.reserve(s.size() * 3 / 2);
  for (const Char &char_: s) {
    runes.push_back(char_.rune);
  }
  string result;
  if (result.capacity() < runes.size()) {
    result.reserve(runes.size() * 3 / 2);
  }
  utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  return result;
}

Text::Text(const string_view &str_, const Rgb &fg, const Rgb &bg, const Attributes &attr, const AttributeMode &mix) {
  const auto _str{utf8::utf8to32(string(str_))};
  if (!_str.empty()) {
    auto splits{_str | split('\n')}; // const not allowed
    const size_t width = std::ranges::max(splits | transform([](auto line) {return line.size();}));
    data.reserve(std::ranges::distance(splits));
    for (auto line: splits) {
      String iline;
      iline.reserve(line.size());
      // Copy line
      for (const Unicode &ch: line) {
        iline.emplace_back(ch, fg, bg, attr, mix);
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

Text::Text(Char c, const Vector &size, const AttributeMode &mixDefault):
data(
  vector<String>(
    max(size.y(), Dim(1)),
    String(
      max(size.x(), Dim(1)),
      Char(
        c.rune,
        c.attributes.fg,
        c.attributes.bg,
        c.attributes.attr,
        mixDefault == AttributeMode::default_? c.attributes.mix: mixDefault
      )
    )
  )
)
{}

auto Text::height() const -> Dim {
  return toDim(data.size());
}

auto Text::width() const -> Dim {
  return toDim(!data.empty()? data[0].size(): 0);
}

auto Text::size() const -> Vector {
  return Vector(width(), height());
}

Text::operator string() const {
  // String representation
  string result;
  vector<Unicode> runes;
  if (!data.empty()) {
    runes.reserve(data[0].size() + 1);
  }
  for (const String &_line: data) {
    runes.clear();
    for (const Char &char_: _line) {
      runes.push_back(char_.rune);
    }
    runes.push_back('\n');
    utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  }
  return result;
}

auto Text::repr() const -> string {
  // String representation
  static const auto UnicodeControlPicturesStart{0x2400};
  string result{"\"\"\"\n"};
  vector<Unicode> runes;
  runes.reserve(data.empty()? 4: data[0].size() + 1);
  for (const String &_line: data) {
    runes.clear();
    for (const Char &char_: _line) {
      runes.push_back(
        // Replace control characters with corresponding Unicode rune
        char_.rune < ' '? char_.rune + UnicodeControlPicturesStart: char_.rune
      );
    }
    runes.push_back(L'¬');
    runes.push_back('\n');
    utf8::utf32to8(runes.begin(), runes.end(), back_inserter(result));
  }
  result += "\"\"\"\n";
  return result;
}

auto Text::rightAligned(Dim width) const -> Text {
  width = width == DimLow? this->width(): width;
  if (width < 0) {
    throw range_error("width < 0");
  }
  Text result;
  for (String _line: data) {
    // Trim line on right
    auto ci{_line.rbegin()};
    while (ci != _line.rend() && ci->rune == ' ') {
      ++ci;
    }
    _line.resize(_line.rend() - ci);
    // Space-fill on left
    String filled(width, ' '_C.withAttr(_line[0].attributes));
    copy(_line.begin(), _line.end(), back_inserter(filled));
    // Trim line on left
    String newLine{filled.end() - width, filled.end()};
    // Add line
    result.data.push_back(newLine);
  }
  return result;
}

auto Text::centered(Dim width) const -> Text {
  width = width == DimLow? this->width(): width;
  if (width < 0) {
    throw range_error("width < 0");
  }
  Text result;
  for (const String &_line: data) {
    // Trim line
    auto rend{_line.rbegin()};
    while (rend != _line.rend() && rend->rune == ' ') {
      ++rend;
    }
    auto end = rend.base();
    auto begin{_line.begin()};
    while (begin != _line.end() && begin->rune == ' ') {
      ++begin;
    }
    const auto _size = end - begin;
    if (_size > width) {
      begin += (_size - 1) / 2 - (width - 1) / 2;
      end = begin + width;
      result.data.emplace_back(begin, end);
    } else {
      String newLine(width, ' '_C.withAttr(begin->attributes));
      const auto pos = (width - 1) / 2 - (_size - 1) / 2;
      copy(begin, end, newLine.begin() + pos);
      result.data.push_back(newLine);
    }
  }
  return result;
}

auto Text::operator ==(const Text &other) const -> bool {
  return data == other.data;
}

Draw::Draw(u1 strength, u1 dash, bool roundedCorners):
strength(strength), dash(dash), roundedCorners(roundedCorners)
{}

Box::Box(const Rectangle &area, u1 strength, u1 dash, bool roundedCorners):
Draw(strength, dash, roundedCorners),
area(area)
{}

auto Char::combine(
  const Char          &other,
  const AttributeMode &mixDefault,
  const AttributeMode &overrideMix,
  const AttributeMode &resetMix
) const -> Char {
  // Combine characters
  const auto mix_ =
    (overrideMix == AttributeMode::default_)?
      (other.attributes.mix == AttributeMode::default_)?
        mixDefault
      :
        other.attributes.mix
    :
      overrideMix;
  const auto rune_ = (other.rune == NoneRune)? rune: other.rune;
  Char result;
  switch (mix_) {

    case AttributeMode::merge:
    result = Char(
      rune_,
      attributes.fg | other.attributes.fg,
      attributes.bg | other.attributes.bg,
      attributes.attr | other.attributes.attr,
      AttributeMode::merge
    );
    break;

    case AttributeMode::mix:
    result = Char(
      rune_,
      attributes.fg + other.attributes.fg,
      attributes.bg + other.attributes.bg,
      attributes.attr | other.attributes.attr,
      AttributeMode::mix
    );
    break;

    case AttributeMode::replace:
    result = other;
    break;

    case AttributeMode::ignore:
    result = Char(
      rune_,
      attributes.fg,
      attributes.bg,
      attributes.attr,
      attributes.mix
    );
    break;

    case AttributeMode::default_:
    throw domain_error("source mix must not be Default");
    break;
  }
  if (resetMix != AttributeMode::default_) {
    result.attributes.mix = resetMix;
  }
  return result;
}

namespace
{
auto initializedBox2Quad() -> unordered_map<Unicode, QuadTuple> {
  unordered_map<Unicode, QuadTuple> result(_quadInitializer.size() * 2);
  // TODO(jwezel): try copy()
  for (const auto &[rune, quad]: _quadInitializer) {
    result[rune] = quad;
  }
  return result;
}

auto QuadTuple2Value_(const QuadTuple &tuple) -> QuadValue {
  return
    static_cast<unsigned>((tuple[0] << QuadNorthShift)) |
    static_cast<unsigned>((tuple[1] << QuadSouthShift)) |
    static_cast<unsigned>((tuple[2] << QuadWestShift)) |
    static_cast<unsigned>((tuple[3] << QuadEasthShift)) |
    tuple[4];
}

auto initializedQuad2Box() -> unordered_map<QuadValue, Unicode> {
  unordered_map<QuadValue, Unicode> result;
  // TODO(jwezel): try copy()
  for (const auto &[rune, quad]: _quadInitializer) {
    result[QuadTuple2Value_(quad)] = rune;
  }
  return result;
}
}  // namespace

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
      const auto segment{segments.at(i)};
      if (segment != 0) {
        quad.at(i) = segment;
      }
    }
    if (
      ((quad[north] == 1) == (quad[south] == 1)) and
      ((quad[west] == 1) != (quad[east] == 1)) and roundedCorners
    ) {
      // On corners of strength 1 lines, `roundedCorners` can be set
      quad[mode] = 1;
    } else if (
      quad[north] == quad[south] and quad[west] == quad[east] and
      (quad[north] != 0) != (quad[west] != 0) and dash >= MinDash and
      dash <= MaxDash
    ) {
      // On straight lines, `dash`ing can be set
      quad[mode] = dash;
    }
    rune = quad2Box[QuadTuple2Value_(quad)];
  } catch (const out_of_range &) { // NOLINT(bugprone-empty-catch)
    // TODO(j) WTF
  }
}

void Text::extend(const Vector &size, const Char &fill) {
  if (width() > 0) {
    for (Dim l = 0; l < height(); ++l) {
      if (size.x() > toDim(data[l].size())) {
        fill_n(back_inserter(data[l]), size.x() - data[l].size(), fill);
      }
    }
  }
  if (size.y() > toDim(data.size())) {
    fill_n(back_inserter(data), size.y() - data.size(), String(size.x(), fill));
  }
}

auto Text::fill(const Char &fill, const Rectangle &area) -> Rectangle {
  auto area_{area == RectangleMax? Rectangle{Vector{0, 0}, size()}: (extend(Vector{area.x2(), area.y2()}, fill), area)};
  const auto _line{String(area_.width(), fill)};
  for (auto l = area_.y1(); l < area_.y2(); ++l) {
    std::copy(_line.begin(), _line.end(), data[l].begin() + area_.x1());
  }
  return area_;
}

void Text::patch(
  const Text &other,
  const Vector &position,
  const AttributeMode &mixDefaultMode,
  const AttributeMode &overrideMixMode,
  const AttributeMode &resetMixMode
) {
  assert(data.data() != other.data.data()); // NOLINT
  const Dim
    xdest = std::max(toDim(0), position.x()),
    xbegin = toDim(xdest - position.x()),
    ydest = std::max(toDim(0), position.y()),
    ybegin = toDim(ydest - position.y()),
    width_ = toDim(std::min(width() - xdest, other.width() - xbegin)),
    height_ = toDim(std::min(height() - ydest, other.height() - ybegin));
  for (Dim l = 0; l < height_; ++l) {
    const Dim owidth = std::min(toDim(other.data[ybegin + l].size()), width_);
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
  if (data.data() == other.data.data()) {
    throw range_error("This and other are the same");
  }
  const Rectangle thisDimensions{0, 0, width(), height()};
  const auto area_{area.defaultTo(thisDimensions)};
  if ((area_ & Rectangle{0, 0, this->width(), this->height()}) != area_) {
    throw runtime_error("area must be within text");
  }
  auto data_ = other.data.begin();
  for (Dim l = 0; l < area_.height(); ++l) {
    auto source = data_->begin();
    const auto sourceEnd = data_->end();
    auto
      dest = data[area_.y1() + l].begin() + area_.x1(),
      _limit = data[area_.y1() + l].begin() + area_.x2();
    while (dest != _limit && source != sourceEnd) {
      *dest++ = dest->combine(*source++, mixDefault, overrideMix, resetMix);
    }
    ++data_;
  }
}

void Text::resize(const Vector &size, const Char &fill) {
  if (size.y() < height()) {
    data.erase(data.begin() + size.y(), data.end());
  }
  if (size.x() < width()) {
    for (auto &_line: data) {
      _line.erase(_line.begin() + size.x(), _line.end());
    }
  }
  extend(size, fill);
}

void Text::setAttr(const CharAttributes &attr, const Rectangle &area, const AttributeMode &setMix) {
  const auto limits = Rectangle{0, 0, width(), height()};
  const std::optional<Rectangle> area_ = area.defaultTo(limits) & limits;
  if (area_) {
    const Rectangle _area = area_.value();
    for (Dim l = _area.y1(); l < _area.y2(); ++l) {
      for (Dim c = _area.x1(); c < _area.x2(); ++c) {
        data[l][c] = data[l][c].combine(Char(NoneRune, attr), AttributeMode::default_, AttributeMode::ignore, setMix);
      }
    }
  }
}

auto Text::operator [](const Rectangle &area) const -> Text {
  const std::optional<Rectangle> area_ = area & Rectangle(0, 0, width(), height());
  Text result;
  if (area_) {
    const Rectangle _area = area_.value();
    for (Dim _line = _area.y1(); _line < std::min(height(), _area.y2()); ++_line) {
      result.data.emplace_back(
        data[_line].begin() + _area.x1(),
        data[_line].begin() + std::max(_area.x1(), std::min(_area.x2(), toDim(data[_line].size())))
      );
    }
  }
  return result;
}

auto Text::operator [](const Vector &position) const -> Char {
  const Vector position_ = size().position(position);
  if (position_.y() < height() && position_.x() < width()) {
    return data[position_.y()][position_.x()];
  }
  return Null;
}

auto Text::operator [](const Vector &position) -> Char & {
  const Vector position_ = size().position(position);
  if (position_.y() < height() && position_.x() < width()) {
    return data[position_.y()][position_.x()];
  }
  throw range_error(format("Position {} is outside text dimensions {}", string(position), string(size())));
}

auto Text::at(const Vector &position) -> Char & {
  const Vector position_ = size().position(position);
  if (position_.y() < height() && position_.x() < width()) {
    return data[position_.y()][position_.x()];
  }
  throw range_error(format("Position {} is outside text dimensions {}", string(position), string(size())));
}

namespace {
auto drawHorizontalLine(
  Text &text,
  const Vector &initialPosition,
  Dim endPosition,
  u1 strength,
  u1 dash,
  bool roundedCorners,
  bool extendBegin,
  bool extendEnd
) -> Rectangle {
  const Dim
    ep = toDim(endPosition == DimHigh? text.width(): endPosition + (endPosition < 0? text.width(): 0)),
    bp = initialPosition.x();
  for (Dim p = bp; p < ep; ++p) {
    if (p == bp and not extendBegin) {
      text.at(Vector(p, initialPosition.y())).drawLineChar({0, 0, 0, strength}, dash, roundedCorners);
    } else if (p == ep - 1 and not extendEnd) {
      text.at(Vector(p, initialPosition.y())).drawLineChar({0, 0, strength, 0}, dash, roundedCorners);
    } else {
      text.at(Vector(p, initialPosition.y())).drawLineChar({0, 0, strength, strength}, dash, roundedCorners);
    }
  }
  return Rectangle{
    initialPosition.x(), initialPosition.y(), toDim(initialPosition.x() + ep + 1), toDim(initialPosition.y() + 1)
  };
}

auto drawVerticalLine(
  Text &text,
  const Vector &initialPosition,
  Dim endPosition,
  u1 strength,
  u1 dash,
  bool roundedCorners,
  bool extendBegin,
  bool extendEnd
) -> Rectangle {
  const Dim
    ep = toDim(endPosition == DimHigh? text.height(): endPosition + (endPosition < 0? text.height(): 0)),
    bp = initialPosition.y();
  for (Dim p = initialPosition.y(); p < ep; ++p) {
    if (p == bp and not extendBegin) {
      text.at(Vector(initialPosition.x(), p)).drawLineChar({0, strength, 0, 0}, dash, roundedCorners);
    } else if (p == ep - 1 and not extendEnd) {
      text.at(Vector(initialPosition.x(), p)).drawLineChar({strength, 0, 0, 0}, dash, roundedCorners);
    } else {
      text.at(Vector(initialPosition.x(), p)).drawLineChar({strength, strength, 0, 0}, dash, roundedCorners);
    }
  }
  return Rectangle{
    initialPosition.x(), initialPosition.y(), toDim(initialPosition.x() + 1), toDim(initialPosition.y() + ep + 1)
  };
}
} // namespace

auto Text::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Rectangle {
  if (strength < 1 or strength > 2) {
    throw range_error(format("Invalid strength: {}", strength));
  }
  return (line.orientation == Horizontal? drawHorizontalLine: drawVerticalLine)(
    *this,
    line.position,
    line.endPosition,
    strength,
    dash,
    roundedCorners,
    line.extendBegin,
    line.extendEnd
  );
}

auto Text::withLine(const Line &line_, u1 strength, u1 dash, bool roundedCorners) -> Text & {
  line(line_, strength, dash, roundedCorners);
  return *this;
}

namespace {
///
/// Limit value
///
/// If value is negative it denotes "from the right".
///
/// @param[in]  v     Value
/// @param[in]  m     Maximum
///
/// @return     Value clipped to maximum
auto limit(Dim v, Dim m) -> Dim {
  return toDim(v < 0? v + m: min(v, toDim(m - 1)));
}
} // namespace

auto Text::box(const Box &box) -> vector<Rectangle> {
  vector<Rectangle> result;
  const Rectangle area = box.area.defaultTo(Rectangle(0, 0, width(), height()));
  return vector<Rectangle>{
    // top
    line(
      Line{
        Vector(area.x1(), area.y1()),
        limit(area.x2(), toDim(area.width() + 1)),
        Horizontal,
        false,
        false
      },
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // bottom
    line(
      Line{
        Vector(area.x1(), toDim(area.y2() - 1)),
        limit(area.x2(), toDim(area.width() + 1)),
        Horizontal,
        false,
        false
      },
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // left
    line(
      Line{
        Vector(area.x1(), area.y1()),
        limit(area.y2(), toDim(area.height() + 1)),
        Vertical,
        false,
        false
      },
      box.strength,
      box.dash,
      box.roundedCorners
    ),
    // right
    line(
      Line{
        Vector(toDim(area.x2() - 1), area.y1()),
        limit(area.y2(), toDim(area.height() + 1)),
        Vertical,
        false,
        false
      },
      box.strength,
      box.dash,
      box.roundedCorners
    )
  };
}

auto Text::withBox(const Box &box_) -> Text & {
  box(box_);
  return *this;
}

} // namespace jwezel
