#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <system_error>
#include <regex>
#include <unistd.h>

#include <fmt/core.h>
#include <utility>

#include "display.hh"
#include "fmt/format.h"
#include "geometry.hh"
#include "screen.hh"
#include "text.hh"
#include "update.hh"
#include "string.hh"

using namespace jwezel;

using fmt::format, fmt::print;
using std::array;

Display::Display(
  Keyboard &keyboard,
  int output,
  const Vector &position__,
  const Vector &size__,
  const Vector &expandTo
):
keyboard_(keyboard),
output_{output},
cursor_(cursor()),
terminalSize_{terminalSize()},
foreground_{RgbWhite},
background_{RgbNone},
attributes_{},
position_{
  position__ == VectorMin?
    cursor_
  :
    position__
},
maxSize_{min(expandTo == VectorMax? terminalSize_: expandTo, terminalSize_ - position_)},
text_(Null, size__ == VectorMin? Vector{1, 1}: min(size__, maxSize_))
{
  cursor(false);
}

Display::~Display() {
  cursor(true);
  foreground(RgbNone);
  background(RgbNone);
  attributes({});
  cursor(0, size().y + position_.y - 1);
  write("\n");
}

void Display::write(const string_view &str) const {
  if (!str.empty()) {
    string outstr{str};
    auto wp{outstr.c_str()};
    size_t wt{0};
    size_t ws{outstr.size()};
    while (wt < outstr.size()) {
      auto wc{::write(output_, wp + wt, ws - wt)};
      if (wc < 0) {
        throw system_error(errno, system_category(), format("Could not write to display {}", output_));
      }
      wt += wc;
    }
  }
}

Vector Display::cursor() {
  static const basic_regex reportPattern("\x1b\\[(\\d+);(\\d+)R");
  string report;
  report.reserve(10);
  for (int i = 0; i < 10; ++i) {
    write("\x1b[6n");
    Unicode key;
    do {
      key = keyboard_.key();
      report += key;
    } while (key != 'R');
    smatch subMatch;
    if (regex_match(report, subMatch, reportPattern)) {
      return cursor_ = Vector(Dim(stoi(subMatch[2].str()) - 1), Dim(stoi(subMatch[1].str()) - 1));
    }
  }
  throw runtime_error("Terminal did not report cursor position");
}

void Display::cursor(int x, int y) {
  if (cursor_.x != x or cursor_.y != y) {
    cursor_ = Vector(x, y);
    write(format("\x1b[{};{}H", y + 1, x + 1));
  }
}

void Display::cursor(bool mode) {
  static const array<string, 2> sequence{"\x1b[?25l", "\x1b[?25h"};
  write(sequence[int(mode)]);
}

void Display::foreground(const Rgb &color) {
  if (foreground_ != color) {
    foreground_ = color;
    if (color == RgbNone) {
      write("\x1b[39m");
    } else {
      write(format("\x1b[38;2;{};{};{}m", int(255 * color.r), int(255 * color.g), int(255 * color.b)));
    }
  }
}

void Display::background(const Rgb &color) {
  if (background_ != color) {
    background_ = color;
    if (color == RgbNone) {
      write("\x1b[49m");
    } else {
      write(format("\x1b[48;2;{};{};{}m", int(255 * color.r), int(255 * color.g), int(255 * color.b)));
    }
  }
}

void Display::attributes(const Attributes &attributes) {
  static const vector<array<string, 2>> sequence{
    {"22", "1"},
    {"24", "4"},
    {"27", "7"},
    {"25", "5"}
  };
  static const vector<string> sepString{"\x1b[", ";"};
  string output;
  output.reserve(16);
  for (unsigned b = 0, sep = 0; b < sequence.size(); ++b, sep = 1) {
    const unsigned a = 1 << b;
    if ((attributes & a) != (attributes_ & a)) {
      output += sepString[sep] + sequence[b][bool(attributes & a) - bool(attributes_ & a) + 1 >> 1];
    }
  }
  if (!output.empty()) {
    attributes_ = attributes;
    output += "m";
  }
  write(output);
}

Vector Display::terminalSize() {
  // Maybe also consider COLUMNS & LINES and ioctl-based terminal size
  const auto current{cursor()};
  cursor(9999, 9999); // Will set the cursor to max col, max line
  auto c{cursor()};
  Vector result{c + 1};
  cursor(current.x, current.y);
  return result;
}


void Display::update(const Vector &position, const Text &text) {
  auto area{Rectangle{position, position + text.size()} & Rectangle{0, size()}};
  if (!area) {
    return;
  }
  auto textArea{area.value() - position};
  // text_.extend(min(position + text.size(), maxSize_));
  for (Dim line = textArea.y1, dline = area.value().y1; line < textArea.y2; ++line, ++dline) {
    assert(line < (int)text.data.size());
    while (line + position.y >= (int)text_.data.size()) {
      text_.data.push_back(String(text.data[line].size(), Null));
    }
    for (Dim column = textArea.x1, dcolumn = area.value().x1; column < textArea.x2; ++column, ++dcolumn) {
      const auto &ch{text.data[line][column]};
      if (text_.data[dline][dcolumn] != ch) {
        cursor(dcolumn + position_.x, dline + position_.y);
        foreground(ch.attributes.fg);
        background(ch.attributes.bg);
        attributes(ch.attributes.attr);
        write(ch.utf8());
        cursor_.x += 1;
        if (cursor_.x > maxSize_.x) {
          cursor_.x = 0;
          cursor_.y += 1;
        }
        assert(dcolumn < (int)text_.data[dline].size());
        text_.data[dline][dcolumn] = ch;
      }
    }
  }
}

void Display::update(const Updates &updates) {
  for (const auto &update_: updates) {
    update(update_.position, update_.text);
  }
}

Vector Display::size() const {
  return text_.size();
}

void Display::resize(const Vector &size) {
  text_.resize(min(maxSize_, size), Null);
}
