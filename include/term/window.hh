#pragma once

#include "basic.hh"
#include "geometry.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"

namespace jwezel {

///
/// Base Window
struct BaseWindow: Surface::Element {

  ///
  /// Constructor
  ///
  /// @param      terminal    The terminal
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  explicit BaseWindow(struct TerminalInterface *terminal, const Rectangle &area, const Char &background=Space);

  BaseWindow(const BaseWindow &) = default;

  BaseWindow(BaseWindow &&) = default;

  auto operator=(const BaseWindow &) -> BaseWindow & = default;

  auto operator=(BaseWindow &&) -> BaseWindow & = delete;

  ~BaseWindow() override = default;

  [[nodiscard]] inline auto terminal() const {return terminal_;}

  [[nodiscard]] inline auto position() const {return position_;}

  void position(const Rectangle &area);

  virtual inline bool moveEvent(const Rectangle &) override {return true;}

  virtual inline bool deleteEvent() {return true;}

  [[nodiscard]] inline auto background() const {return background_;}

  private:
  struct TerminalInterface *terminal_;
  Vector position_;
  Char background_;
};

///
/// A virtually infinite window, all blank with no buffer, used to represent
/// the display in a clear state.
struct Backdrop: public BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  explicit Backdrop(struct TerminalInterface *terminal, const Char &background=Space);

  Backdrop(const Backdrop &) = delete;

  Backdrop(Backdrop &&) = delete;

  auto operator=(const Backdrop &) -> Backdrop & = default;

  auto operator=(Backdrop &&) -> Backdrop & = delete;

  ///
  /// Destroy Backdrop
  ~Backdrop() override;

  ///
  /// Get backdrop area
  ///
  /// @return     RectangleMax
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area=RectangleMax) const -> Text override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &/*area*/) {}
};

///
/// Window
struct Window: public BaseWindow {
  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  explicit Window(struct TerminalInterface *terminal, const Rectangle &area, const Char &background=Space, Window *below=0);

  Window() = delete;

  Window(const Window &) = default;

  Window(Window &&) = default;

  auto operator=(const Window &) -> Window & = default;

  auto operator=(Window &&) -> Window & = delete;

  ///
  /// Destroy Window
  ~Window() override;

  /// String representation of Window
  explicit operator string() const;

  ///
  /// Write string to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  auto write(const Vector &position, const string_view &str) -> Window &;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  auto write(const Vector &position, const Text &txt_) -> Window &;

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  auto fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) -> Window &;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> Window &;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  auto box(const Box &box) -> Window &;

  ///
  /// Get window size
  ///
  /// @return     Window size
  // [[nodiscard]] auto size() const -> Vector override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area);

  bool moveEvent(const Rectangle &) override;

  ///
  /// Get window area
  ///
  /// @return     Window area
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area = RectangleMax) const -> Text override;

  private:
  Text text_;
};

} // namespace jwezel
