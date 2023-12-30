#include "display.hh"

#include <array>
#include <cassert>
#include <exception>
#include <iostream>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

#include <format>

namespace jwezel {

using std::format;
using
  std::array,
  std::basic_regex,
  std::cerr,
  std::exception,
  std::runtime_error,
  std::smatch,
  std::system_category,
  std::system_error;

Display::Display(
  Keyboard::ref &keyboard,
  int output,
  const Vector &position,
  const Vector &size,
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
  position == VectorMin?
    cursor_
  :
    position
},
maxSize_{min(expandTo == VectorMax? terminalSize_: expandTo, terminalSize_ - position_)},
text_(Null, size == VectorMin? Vector{1, 1}: min(size, maxSize_))
{
  cursor(false);
  keyboard.displayOffset(position_);
}

Display::~Display() {
  try {
    cursor(true);
    foreground(RgbNone);
    background(RgbNone);
    attributes({});
    cursor(0, toDim(size().y() + position_.y() - 1));
    write("\x1b[?9l\x1b[?1000l\x1b[?1002l\x1b[?1003l\n");
  } catch (exception & error) {
    cerr << "Error: " << error.what() << "\n";
  }
}

void Display::write(const string_view &str) const {
  if (!str.empty()) {
    string outstr{str};
    const auto *wp{outstr.c_str()};
    size_t wt{0};
    size_t ws{outstr.size()};
    while (wt < ws) {
      auto wc{::write(output_, wp + wt, ws - wt)}; // NOLINT
      if (wc < 0) {
        throw system_error(errno, system_category(), format("Could not write to display {}", output_));
      }
      wt += wc;
    }
  }
}

auto Display::cursor() -> Vector {
  static const basic_regex reportPattern("\x1b\\[(\\d+);(\\d+)R");
  static const size_t MaxChars{10};
  string report;
  for (size_t i = 0; i < MaxChars; ++i) {
    write("\x1b[6n");
    Unicode key{0};
    do {
      key = keyboard_.key();
      report += static_cast<string::value_type>(key);
    } while (key != 'R');
    smatch subMatch;
    if (regex_match(report, subMatch, reportPattern)) {
      return cursor_ = Vector(static_cast<Dim>(stoi(subMatch[2].str()) - 1), static_cast<Dim>(stoi(subMatch[1].str()) - 1));
    }
  }
  throw runtime_error("Terminal did not report cursor position");
}

void Display::cursor(Dim column, Dim line) {
  if (cursor_.x() != column or cursor_.y() != line) {
    cursor_ = Vector(column, line);
    write(format("\x1b[{};{}H", line + 1, column + 1));
  }
}

void Display::cursor(bool mode) const {
  static const array<string, 2> sequence{"\x1b[?25l", "\x1b[?25h"};
  write(sequence[mode? 1: 0]); // NOLINT
}

void Display::foreground(const Rgb &color) {
  if (foreground_ != color) {
    foreground_ = color;
    if (color == RgbNone) {
      write("\x1b[39m");
    } else {
      write(
        format(
          "\x1b[38;2;{};{};{}m",
          static_cast<int>(HighColor * color.r),
          static_cast<int>(HighColor * color.g),
          static_cast<int>(HighColor * color.b)
        )
      );
    }
  }
}

void Display::background(const Rgb &color) {
  if (background_ != color) {
    background_ = color;
    if (color == RgbNone) {
      write("\x1b[49m");
    } else {
      write(
        format(
          "\x1b[48;2;{};{};{}m",
          static_cast<int>(HighColor * color.r),
          static_cast<int>(HighColor * color.g),
          static_cast<int>(HighColor * color.b)
        )
      );
    }
  }
}

void Display::attributes(const Attributes &attributes) {
  static const array<array<string, 2>, 4> sequence{
    array<string, 2>{"22", "1"},
    array<string, 2>{"24", "4"},
    array<string, 2>{"27", "7"},
    array<string, 2>{"25", "5"}
  };
  static const array<string, 2> sepString{"\x1b[", ";"};
  string output;
  for (unsigned b = 0, sep = 0; b < sequence.size(); ++b, sep = 1) {
    const int
      nattr = (/*NOLINT*/attributes >> b) & 1,
      eattr = (/*NOLINT*/attributes_ >> b) & 1;
    if (nattr != eattr) {
      output += sepString.at(sep) + sequence.at(b).at((nattr - eattr) + 1U >> 1U);
    }
  }
  if (!output.empty()) {
    attributes_ = attributes;
    output += "m";
  }
  write(output);
}

auto Display::terminalSize() -> Vector {
  // Maybe also consider COLUMNS & LINES and ioctl-based terminal size
  const auto current{cursor()};
  static const int HighScreenDim{9999};
  cursor(HighScreenDim, HighScreenDim); // Will set the cursor to max col, max line
  auto cursorPos{cursor()};
  Vector result{cursorPos + 1};
  cursor(current.x(), current.y());
  return result;
}

void Display::update(const Vector &position, const Text &text) {
  auto area{Rectangle{position, position + text.size()} & Rectangle{Vector{0, 0}, size()}};
  if (!area) {
    return;
  }
  auto textArea{area.value() - position};
  for (Dim line = textArea.y1(), dline = area.value().y1(); line < textArea.y2(); ++line, ++dline) {
    assert(line < toDim(text.data.size()));
    while (toDim(line + position.y()) >= text_.height()) {
      text_.data.emplace_back(text.data[line].size(), Null);
    }
    for (Dim column = textArea.x1(), dcolumn = area.value().x1(); column < textArea.x2(); ++column, ++dcolumn) {
      const auto &ch{text.data[line][column]};
      if (text_.data[dline][dcolumn] != ch) {
        cursor(toDim(dcolumn + position_.x()), toDim(dline + position_.y()));
        foreground(ch.attributes.fg);
        background(ch.attributes.bg);
        attributes(ch.attributes.attr);
        write(ch.utf8());
        cursor_ = cursor_.right();
        if (cursor_.x() > maxSize_.x()) {
          cursor_ = Vector{0, cursor_.y()};
          cursor_ = cursor_.down();
        }
        assert(dcolumn < (int)text_.data[dline].size()); // NOLINT
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

auto Display::size() const -> Vector {
  return text_.size();
}

void Display::resize(const Vector &size) {
  text_.resize(min(maxSize_, size), Null);
}

void Display::mouseMode(MouseMode mode) const {
  static const array<string, 5> sequence{
    "\x1b[?9l\x1b[?1000l\x1b[?1002l\x1b[?1003l",
    "\x1b[?9h\x1b[?1006h",
    "\x1b[?1000h\x1b[?1006h",
    "\x1b[?1002h\x1b[?1006h",
    "\x1b[?1003h\x1b[?1006h"
  };
  write(sequence[static_cast<int>(mode)]); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

} // namespace jwezel
