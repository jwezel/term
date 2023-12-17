#pragma once

#include "keyboard.hh"
#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"

namespace jwezel {

///
/// This class describes a display.
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
  /// @param[in]  expandTo  Max size (VectorMin=start size, VectorMax=screen
  /// size)
  explicit Display(
    Keyboard &keyboard,
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
  ~Display() override;

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
  auto terminalSize() -> Vector;

  [[nodiscard]] auto maxSize() const {return maxSize_;}

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Keyboard &keyboard_;                //< Keyboard
  int output_;                        //< Output file descriptor
  Vector cursor_;                     //< Current cursor position
  Vector terminalSize_;               //< Terminal size (cache)
  Rgb foreground_;                    //< Current foreground color
  Rgb background_;                    //< Current background color
  Attributes attributes_;             //< Current character attributes
  Vector position_;                   //< Display position
  Vector maxSize_;                    //< Maximum display size
  Text text_;                         //< Display text
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace jwezel
