#pragma once

#include "surface.hh"

#include <basic.hh>
#include <geometry.hh>
#include <string>
#include <string_view>
#include <text.hh>

namespace jwezel::screen {

///
/// Base Window
struct BaseWindow: Surface::Element {

  BaseWindow(const BaseWindow &) = default;

  BaseWindow(BaseWindow &&) = delete;

  auto operator=(const BaseWindow &) -> BaseWindow & = default;

  auto operator=(BaseWindow &&) -> BaseWindow & = delete;

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  explicit BaseWindow(const Rectangle &area, const Char &background=Space);

  ///
  /// Destroy BaseWindow
   ~BaseWindow() override = default;

  /// String representation of WindowObj
  virtual explicit operator string() const;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  virtual void write(const Vector &position, const string_view &str);

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  virtual void write(const Vector &position, const Text &txt_);

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  virtual void fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax);

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  virtual Rectangle line(
    const Line &line,
    u1 strength=1,
    u1 dash=0,
    bool roundedCorners=false
  );

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  virtual auto box(const Box &box) -> vector<Rectangle>;

  ///
  /// Get window size
  ///
  /// @return     Window size
  [[nodiscard]] virtual auto size() const -> Vector;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
   void move(const Rectangle &area) override;

  ///
  /// Get window area
  ///
  /// @return     Window area
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get window text
  ///
  /// @return     Text
  [[nodiscard]] virtual auto text() const -> const Text &;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area) const -> Text override;

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Vector position;
  Char background;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
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
  explicit Backdrop(const Char &background=Space);

  Backdrop(const Backdrop &) = default;

  Backdrop(Backdrop &&) = delete;

  auto operator=(const Backdrop &) -> Backdrop & = default;

  auto operator=(Backdrop &&) -> Backdrop & = delete;

  ///
  /// Destroy Backdrop
  ~Backdrop() override = default;

  ///
  /// Get backdrop area
  ///
  /// @return     RectangleMax
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get backdrop size
  ///
  /// @return     VectorMax
  [[nodiscard]] auto size() const -> Vector override;

  ///
  /// Get window text
  ///
  /// @return     Text
  [[nodiscard]] auto text() const -> const Text & override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area) const -> Text override;
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
  explicit Window(const Rectangle &area, const Char &background=Space);

  Window(const Window &) = default;

  Window(Window &&) = delete;

  auto operator=(const Window &) -> Window & = default;

  auto operator=(Window &&) -> Window & = delete;

  ///
  /// Destroy Window
   ~Window() override = default;

  /// String representation of Window
  explicit operator string() const override;

  ///
  /// Write string to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  void write(const Vector &position, const string_view &str) override;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const Vector &position, const Text &txt_) override;

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  void fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) override;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  auto line(
    const Line &line,
    u1 strength=1,
    u1 dash=0,
    bool roundedCorners=false
  ) -> Rectangle override;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  auto box(const Box &box) -> vector<Rectangle> override;

  ///
  /// Get window size
  ///
  /// @return     Window size
  [[nodiscard]] auto size() const -> Vector override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area) override;

  ///
  /// Get window area
  ///
  /// @return     Window area
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get window text
  ///
  /// @return     Text
  [[nodiscard]] auto text() const -> const Text & override {
    return text_;
  }

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area) const -> Text override {
    return text_[area];
  }

  private:
  Text text_;
};

} // namespace jwezel::screen
