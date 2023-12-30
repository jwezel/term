#pragma once

#include "basic.hh"
#include "geometry.hh"
#include "surface.hh"
#include "text.hh"

#include <memory>

namespace jwezel {

namespace impl
{
  struct Window;

  struct Backdrop;
}

struct BaseWindow {
  virtual auto text(const Rectangle &area=RectangleMax) const -> Text = 0;

  [[nodiscard]] virtual auto area() const -> Rectangle = 0;

  [[nodiscard]] virtual auto element() const -> Element * = 0;

  [[nodiscard]] virtual auto fragments() const -> const vector<Fragment> & = 0;
};

struct Window: BaseWindow {
  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  explicit Window(
    struct Terminal &terminal,
    const Rectangle &area,
    const Char &background=Space,
    const Window &below={}
  );

  Window() {}

  explicit Window(impl::Window *window);

  Window(const Window &) = default;

  Window(Window &&) = default;

  auto operator=(const Window &) -> Window & = default;

  auto operator=(Window &&) -> Window & = default;

  ///
  /// Destroy Window
  ~Window() = default;

  /// String representation of Window
  [[nodiscard]] explicit operator string() const;

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
  auto box(const Box &box = Box{}) -> Window &;

  ///
  /// Get window size
  ///
  /// @return     Window size
  // [[nodiscard]] auto size() const -> Vector;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  auto move(const Rectangle &area) -> Window &;

  bool above(Window &window);

  bool above(int position=-1);

  bool below(Window &window);

  bool below(int position=0);

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
  [[nodiscard]] virtual auto text(const Rectangle &area=RectangleMax) const -> Text override;

  [[nodiscard]] auto ptr() const -> auto {return p_.get();}

  [[nodiscard]] auto element() const -> Element * override;

  [[nodiscard]] auto fragments() const -> const vector<Fragment> & override;

  private:
  std::shared_ptr<impl::Window> p_;
};

//~Backdrop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct Backdrop: BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  explicit Backdrop(struct jwezel::Terminal &terminal, const Char &background=Space);

  Backdrop(const Backdrop &) = default;

  Backdrop(Backdrop &&) = default;

  auto operator=(const Backdrop &) -> Backdrop & = default;

  auto operator=(Backdrop &&) -> Backdrop & = default;

  ///
  /// Destroy Backdrop
  ~Backdrop() = default;

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

  [[nodiscard]] auto fragments() const -> const vector<Fragment> & override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area);

  [[nodiscard]] auto element() const -> Element * override;

  private:
  std::shared_ptr<impl::Backdrop> p_;
};

} // namespace jwezel
