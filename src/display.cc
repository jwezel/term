#include "display.hh"
#include "geometry.hh"
#include "text.hh"

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

namespace impl {

/// This class describes a display
///
/// Responsibility:
///
/// Render content on a terminal device
struct Display: Device {
  ///
  /// Create Display
  ///
  /// @param      keyboard  The keyboard
  /// @param[in]  output    Output file descriptor
  /// @param[in]  position  The position (VectorMin=current)
  /// @param[in]  size      Start size
  /// @param[in]  expandTo  Max size (VectorMin=start size, VectorMax=screen size)
  explicit Display(
    jwezel::Keyboard &keyboard,
    int output=1,
    const Vector &position=VectorMin,
    const Vector &size=VectorMin,
    const Vector &expandTo=VectorMax
  );

  Display(const Display &) = default;

  Display(Display &&) = delete;

  auto operator=(const Display &) -> Display & = delete;

  auto operator=(Display &&) -> Display & = delete;

  ///
  /// Destroy Display
  ~Display();

  ///
  /// Write string
  ///
  /// @param[in]  str   The string
  void write(const string_view &str) const;

  ///
  /// Get cursor position from terminal
  ///
  /// @return     cursor position
  auto cursor() -> Vector;

  ///
  /// Move cursor
  ///
  /// @param[in]  x     0-based column
  /// @param[in]  y     0-based line
  ///
  /// @return     string with ANSI sequence
  void cursor(Dim column, Dim line);

  ///
  /// Turn cursor on/off
  ///
  /// @param[in]  mode  The mode
  ///
  /// @return     string with ANSI sequence
  void cursor(bool mode) const;

  ///
  /// Set foreground
  ///
  /// @param[in]  color  The color
  void foreground(const Rgb &color);

  ///
  /// Set background
  ///
  /// @param[in]  color  The color
  void background(const Rgb &color);

  ///
  /// Set attributes
  ///
  /// @param[in]  attributes  Attribute bitmap
  void attributes(const Attributes &attributes);

  ///
  /// Write text to screen
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void update(const Vector &position, const Text &text);

  ///
  /// Update screen
  ///
  /// @param[in]  updates  The updates
  void update(const Updates &updates) override;

  ///
  /// Get terminal size
  ///
  /// @return     Terminal size
  ///
  [[nodiscard]] auto size() const -> Vector;

  ///
  /// Set terminal size
  ///
  /// @param[in]  size  The size
  void resize(const Vector &size);

  ///
  /// Get "physical" terminal size
  [[nodiscard]] auto terminalSize() -> Vector;

  void mouseMode(MouseMode mode);

  [[nodiscard]] auto maxSize() const {return maxSize_;}

  jwezel::Keyboard &keyboard_;        //< Keyboard
  int output_;                        //< Output file descriptor
  Vector cursor_;                     //< Current cursor position
  Vector terminalSize_;               //< Terminal size (cache)
  Rgb foreground_;                    //< Current foreground color
  Rgb background_;                    //< Current background color
  Attributes attributes_;             //< Current character attributes
  Vector position_;                   //< Display position
  Vector maxSize_;                    //< Maximum display size
  Text text_;                         //< Display text
};

Display::Display(
  jwezel::Keyboard &keyboard,
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

void impl::Display::mouseMode(MouseMode mode) {
  static const array<string, 5> sequence{
    "\x1b[?9l\x1b[?1000l\x1b[?1002l\x1b[?1003l",
    "\x1b[?9h\x1b[?1006h",
    "\x1b[?1000h\x1b[?1006h",
    "\x1b[?1002h\x1b[?1006h",
    "\x1b[?1003h\x1b[?1006h"
  };
  write(sequence[static_cast<int>(mode)]);
}

} // namespace impl

Display::Display(Keyboard &keyboard, int output, const Vector &position, const Vector &size, const Vector &expandTo):
p_{new impl::Display(keyboard, output, position, size, expandTo)}
{}

///
/// Write string
///
/// @param[in]  str   The string
void Display::write(const string_view &str) const {
  p_->write(str);
}

///
/// Get cursor position from terminal
///
/// @return     cursor position
[[nodiscard]] auto Display::cursor() -> Vector {
  return p_->cursor();
}

///
/// Move cursor
///
/// @param[in]  x     0-based column
/// @param[in]  y     0-based line
///
/// @return     string with ANSI sequence
void Display::cursor(Dim column, Dim line) {
  p_->cursor(column, line);
}

///
/// Turn cursor on/off
///
/// @param[in]  mode  The mode
///
/// @return     string with ANSI sequence
void Display::cursor(bool mode) const {
  p_->cursor(mode);
}

///
/// Set foreground
///
/// @param[in]  color  The color
void Display::foreground(const Rgb &color) {
  p_->foreground(color);
}

///
/// Set background
///
/// @param[in]  color  The color
void Display::background(const Rgb &color) {
  p_->background(color);
}

///
/// Set attributes
///
/// @param[in]  attributes  Attribute bitmap
void Display::attributes(const Attributes &attributes) {
  p_->attributes(attributes);
}

///
/// Write text to screen
///
/// @param[in]  position  The position
/// @param[in]  text      The text
void Display::update(const Vector &position, const Text &text) {
  p_->update(position, text);
}

///
/// Update screen
///
/// @param[in]  updates  The updates
void Display::update(const Updates &updates) {
  p_->update(updates);
}

///
/// Get terminal size
///
/// @return     Terminal size
///
[[nodiscard]] auto Display::size() const -> Vector {
  return p_->size();
}

///
/// Set terminal size
///
/// @param[in]  size  The size
void Display::resize(const Vector &size) {
  p_->resize(size);
}

///
/// Get "physical" terminal size
[[nodiscard]] auto Display::terminalSize() -> Vector {
  return p_->terminalSize();
}

void Display::mouseMode(MouseMode mode) {
  p_->mouseMode(mode);
}

[[nodiscard]] auto Display::maxSize() const -> Vector {
  return p_->maxSize();
}

[[nodiscard]] auto Display::text() const -> Text & {
  return p_->text_;
}

} // namespace jwezel
