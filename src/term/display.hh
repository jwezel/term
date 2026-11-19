#pragma once

#include "keyboard.hh"
#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <optional>

namespace jwezel {

///
/// Renders cell updates and controls cursor, mouse-reporting, and screen state on a terminal device.
///
/// This is the library's terminal-output boundary. Higher layers submit text updates and do not emit terminal
/// control sequences themselves.
struct Display: Device {
  /// Amount of mouse input reported by a terminal.
  enum class MouseMode: u1 {
    off,      ///< Do not request mouse reports.
    x10,      ///< Report button presses only.
    normal,   ///< Report button presses and releases.
    buttons,  ///< Report presses, releases, and button motion.
    anything  ///< Report all mouse motion and button events.
  };
  ///
  /// The screen that the display is on
  ///
  /// The main screen is the one of the shell. The display is a rectangle on it, with its top left corner where the
  /// cursor was (or where it is told to be), growing downwards: if there is no more room below it, the terminal is
  /// scrolled up, so the display can grow to the height of the terminal, and what was on the screen before stays
  /// above it. It can be as wide as the terminal is from its left edge.
  ///
  /// The alternate screen is a separate screen of the size of the terminal, which is given up again when the
  /// display ends, leaving the shell screen as it was. It is fixed: there is nothing to scroll and nothing to
  /// stay clear of, so the display is in the top left corner and can be as large as the terminal.
  enum class Screen: u1 {
    main,      ///< Use the shell screen; the display may scroll terminal history to make room.
    alternate  ///< Use a temporary full-terminal screen restored when the display is destroyed.
  };

  ///
  /// Create Display
  ///
  /// @param      keyboard  The keyboard
  /// @param[in]  output    Output file descriptor
  /// @param[in]  position  The position (VectorMin=current); on the alternate screen it can only be the top left
  ///                       corner, and anything else is an error
  /// @param[in]  size      Start size
  /// @param[in]  expandTo  Max size (VectorMin=start size, VectorMax=screen size)
  /// @param[in]  screen    The screen to use
  ///
  /// @throws     std::invalid_argument if the position is not possible on the screen
  explicit Display(
    Keyboard &keyboard,
    int output=1,
    const Vector &position=VectorMin,
    const Vector &size=VectorMin,
    const Vector &expandTo=VectorMax,
    Screen screen=Screen::main
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
  /// @param[in]  column  0-based column
  /// @param[in]  line    0-based line
  void cursor(Dim column, Dim line);

  ///
  /// Turn cursor on/off
  ///
  /// @param[in]  mode  The mode
  void cursor(bool mode);

  ///
  /// Set the caret
  ///
  /// The caret is the hardware cursor, parked at the given position and shown
  /// after every update; without a caret the cursor is hidden.
  ///
  /// @param[in]  position  Position relative to the display, or nothing to hide
  void caret(const std::optional<Vector> &position);

  [[nodiscard]] auto caret() const -> const std::optional<Vector> & {return caret_;}

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
  /// Set display size
  ///
  /// The size is limited to the maximum size. A display on the main screen that gets higher than there is room
  /// for below it scrolls the terminal up by as much as is missing, so that it moves up on the screen: its position
  /// changes, and so does the offset of mouse reports, but nothing on the display moves relative to it.
  ///
  /// @param[in]  size  The size
  void resize(const Vector &size);

  ///
  /// Get "physical" terminal size
  auto terminalSize() -> Vector;

  void mouseMode(MouseMode mode) const;

  [[nodiscard]] auto maxSize() const {return maxSize_;}

  ///
  /// Get the position of the display on the screen
  [[nodiscard]] auto position() const -> const Vector & {return position_;}

  [[nodiscard]] auto screen() const -> Screen {return screen_;}

  [[nodiscard]] auto text() const {return text_;}

  private:
  ///
  /// Switch to the screen and get the cursor position
  auto enter(const Vector &position) -> Vector;

  ///
  /// Scroll the terminal up if the display would not fit on the screen with a height
  void makeRoom(Dim height);

  Keyboard &keyboard_;                //< Keyboard
  int output_;                        //< Output file descriptor
  Screen screen_;                     //< Screen
  Vector cursor_;                     //< Current cursor position
  bool cursorVisible_{true};          //< Whether the hardware cursor is shown
  std::optional<Vector> caret_;       //< Requested caret position
  Vector terminalSize_;               //< Terminal size (cache)
  Rgb foreground_;                    //< Current foreground color
  Rgb background_;                    //< Current background color
  Attributes attributes_;             //< Current character attributes
  Vector position_;                   //< Display position
  Vector maxSize_;                    //< Maximum display size
  Text text_;                         //< Display text
};

} // namespace jwezel
