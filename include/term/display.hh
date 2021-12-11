#pragma once

#include <string_view>
#include "fmt/core.h"

#include "geometry.hh"
#include "text.hh"
#include "keyboard.hh"
#include "update.hh"

using namespace jwezel;

///
/// This class describes a display.
///
/// Responsibility:
///
/// Render content on a terminal device
struct Display {

  ///
  /// Create Display
  ///
  /// @param      keyboard  The keyboard
  /// @param[in]  output    Output file descriptor
  /// @param[in]  position  The position (VectorMin=current)
  /// @param[in]  size      Start size
  /// @param[in]  expandTo  Max size (VectorMin=start size, VectorMax=screen size)
  Display(
    Keyboard &keyboard,
    int output=1,
    const Vector &position={VectorMin},
    const Vector &size={1, 1},
    const Vector &expandTo={VectorMax}
  );

  ///
  /// Destroy Display
  ~Display() = default;

  ///
  /// Write string
  ///
  /// @param[in]  str   The string
  void write(const string_view &str) const;

  ///
  /// Get cursor position from terminal
  ///
  /// @return     cursor position
  Vector cursor();

  ///
  /// Move cursor
  ///
  /// @param[in]  x     0-based column
  /// @param[in]  y     0-based line
  ///
  /// @return     string with ANSI sequence
  void cursor(int x, int y);

  ///
  /// Turn cursor on/off
  ///
  /// @param[in]  mode  The mode
  ///
  /// @return     string with ANSI sequence
  void cursor(bool mode);

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
  void update(const Updates &updates);

  ///
  /// Sets terminal size
  Vector terminalSize();

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
};
