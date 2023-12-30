#pragma once

#include "keyboard.hh"
#include "device.hh"
#include "geometry.hh"
#include "text.hh"
#include "update.hh"
#include <memory>

namespace jwezel {

///
/// This class describes a display.
///
/// Responsibility:
///
/// Render content on a terminal device
struct Display: Device {
  enum class MouseMode: u1 {
    off,
    x10,
    normal,
    buttons,
    anything
  };
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
    Keyboard::ref &keyboard,
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

  void mouseMode(MouseMode mode) const;

  [[nodiscard]] auto maxSize() const {return maxSize_;}

  [[nodiscard]] auto text() const {return text_;}

  struct ref: Device
  {
    inline explicit ref(
      Keyboard::ref &keyboard,
      int output=1,
      const Vector &position=VectorMin,
      const Vector &size=VectorMin,
      const Vector &expandTo=VectorMax
    ):
    ptr_{new Display{keyboard, output, position, size, expandTo}}
    {}

    ref(const ref &) = default;

    ref(ref &&) = delete;

    auto operator=(const ref &) -> ref & = default;

    auto operator=(ref &&) -> ref & = default;

    ///
    /// Destroy ref
    ~ref() override = default;

    ///
    /// Write string
    ///
    /// @param[in]  str   The string
    inline void write(const string_view &str) const {ptr_->write(str);}

    ///
    /// Get cursor position from terminal
    ///
    /// @return     cursor position
    [[nodiscard]] inline auto cursor() -> Vector {return ptr_->cursor();}

    ///
    /// Move cursor
    ///
    /// @param[in]  x     0-based column
    /// @param[in]  y     0-based line
    ///
    /// @return     string with ANSI sequence
    inline void cursor(Dim column, Dim line) {ptr_->cursor(column, line);}

    ///
    /// Turn cursor on/off
    ///
    /// @param[in]  mode  The mode
    ///
    /// @return     string with ANSI sequence
    inline void cursor(bool mode) const {ptr_->cursor(mode);}

    ///
    /// Set foreground
    ///
    /// @param[in]  color  The color
    inline void foreground(const Rgb &color) {ptr_->foreground(color);}

    ///
    /// Set background
    ///
    /// @param[in]  color  The color
    inline void background(const Rgb &color) {ptr_->background(color);}

    ///
    /// Set attributes
    ///
    /// @param[in]  attributes  Attribute bitmap
    inline void attributes(const Attributes &attributes) {ptr_->attributes(attributes);}

    ///
    /// Write text to screen
    ///
    /// @param[in]  position  The position
    /// @param[in]  text      The text
    inline void update(const Vector &position, const Text &text) {ptr_->update(position, text);}

    ///
    /// Update screen
    ///
    /// @param[in]  updates  The updates
    inline void update(const Updates &updates) override {ptr_->update(updates);}

    ///
    /// Get terminal size
    ///
    /// @return     Terminal size
    ///
    [[nodiscard]] inline auto size() const -> Vector {return ptr_->size();}

    ///
    /// Set terminal size
    ///
    /// @param[in]  size  The size
    inline void resize(const Vector &size) {ptr_->resize(size);}

    ///
    /// Get "physical" terminal size
    [[nodiscard]] auto terminalSize() -> Vector {return ptr_->terminalSize();}

    inline void mouseMode(MouseMode mode) const {ptr_->mouseMode(mode);}

    [[nodiscard]] inline auto maxSize() const {return ptr_->maxSize();}

    [[nodiscard]] inline auto text() const {return ptr_->text();}

    private:
    std::shared_ptr<Display> ptr_;
  };

  private:
  Keyboard::ref keyboard_;            //< Keyboard
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

} // namespace jwezel
