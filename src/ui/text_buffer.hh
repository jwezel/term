#pragma once

#include <term/geometry.hh>
#include <term/text.hh>

#include <string>
#include <string_view>
#include <vector>

namespace jwezel::ui {

///
/// Text that is edited: lines of characters
///
/// The buffer is the document, apart from how it is shown. It knows nothing about colors, attributes, areas or
/// scrolling: a line is as long as it has characters, and lines may have any length. There is always at least one
/// line, which may be empty. The characters are Unicode code points; text goes in and out as UTF-8, with lines
/// separated by line feeds.
///
/// Positions are `Vector{column, line}`, counted from 0. A column can be any in 0 .. line length, where the
/// last one is behind the last character, where a character is added.
struct TextBuffer {
  ///
  /// Create a buffer with a single empty line
  TextBuffer() = default;

  ///
  /// Create a buffer with a text
  ///
  /// @param[in]  text  The text in UTF-8; lines end with a line feed (or with a carriage return and a line feed),
  ///                   and a line feed at the end of the text starts another, empty line
  explicit TextBuffer(std::string_view text);

  ///
  /// Get the text, with a line feed between the lines
  [[nodiscard]] auto text() const -> std::string;

  ///
  /// Replace the text; see the constructor. Bytes that are not UTF-8 become U+FFFD.
  void text(std::string_view text);

  [[nodiscard]] auto lines() const -> const std::vector<std::u32string> & {return lines_;}

  [[nodiscard]] auto lineCount() const -> Dim {return toDim(lines_.size());}

  ///
  /// Get a line
  ///
  /// @throws     std::out_of_range if there is no such line
  [[nodiscard]] auto line(Dim line) const -> const std::u32string &;

  ///
  /// Get the number of characters of a line
  [[nodiscard]] auto lineLength(Dim line) const -> Dim {return toDim(this->line(line).size());}

  ///
  /// Get the position behind the last character
  [[nodiscard]] auto end() const -> Vector;

  ///
  /// Whether a position is in the text: the line exists, and the column is not behind the end of it
  [[nodiscard]] auto contains(const Vector &position) const -> bool;

  ///
  /// Add a character at a position
  ///
  /// @throws     std::out_of_range if the position is not in the text
  void insert(const Vector &position, Unicode character);

  ///
  /// Start a new line at a position: what is behind it moves to the new line
  ///
  /// @throws     std::out_of_range if the position is not in the text
  void split(const Vector &position);

  ///
  /// Delete the character at a position; at the end of a line, the next line is joined to it
  ///
  /// @throws     std::out_of_range if the position is not in the text
  void erase(const Vector &position);

  ///
  /// Delete the character before a position; at the beginning of a line, the line is joined to the previous one
  ///
  /// @return     The position that the text before the deleted character ends at, which is the same position if there
  ///             was nothing before it
  ///
  /// @throws     std::out_of_range if the position is not in the text
  auto eraseBefore(const Vector &position) -> Vector;

  private:
  std::vector<std::u32string> lines_{std::u32string{}};
};

} // namespace jwezel::ui
