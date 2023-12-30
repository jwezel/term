#pragma once

#include "basic.hh"
#include "geometry.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"
#include <memory>

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

  inline auto moveEvent(const Rectangle &/*area*/) -> bool override {return true;}

  virtual inline auto deleteEvent() -> bool {return true;}

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

  Backdrop(const Backdrop &) = default;

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
  auto box(const Box &box = Box{}) -> Window &;

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

  auto moveEvent(const Rectangle &/*area*/) -> bool override;

  auto above(Window *window=0) -> bool;

  auto below(Window *window=0) -> bool;

  auto above(int position=0) -> bool;

  auto below(int position=-1) -> bool;

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

  struct ref
  {
    explicit ref(Terminal::ref &terminal, const Rectangle &area, ref &below, const Char &background=Space):
    ptr_{new Window{dynamic_cast<TerminalInterface *>(terminal.ptr()), area, background, below.ptr()}}
    {}

    ref() = delete;

    ref(const ref &) = default;

    ref(ref &&) = default;

    auto operator=(const ref &) -> ref & = default;

    auto operator=(ref &&) -> ref & = default;

    ///
    /// Destroy ref
    ~ref() = default;

    /// String representation of Window
    explicit operator string() const {return string(*ptr_);}

    ///
    /// Write string to window
    ///
    /// @param[in]  position  The position
    /// @param[in]  str       The text
    auto write(const Vector &position, const string_view &str) -> ref & {ptr_->write(position, str);}

    ///
    /// Write text to window
    ///
    /// @param[in]  position  The position
    /// @param[in]  text      The text
    auto write(const Vector &position, const Text &txt_) -> ref & {ptr_->write(position, txt_);}

    ///
    /// Fill window with Char
    ///
    /// @param[in]  fillChar  The fill character
    /// @param[in]  area      The area
    auto fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) -> ref & {ptr_->fill(fillChar, area);}

    ///
    /// Draw line
    ///
    /// @param[in]  line            Line
    /// @param[in]  strength        Line strength
    /// @param[in]  dash            Dash mode
    /// @param[in]  roundedCorners  Whether to round corners
    ///
    /// @return     Rectangle
    auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> ref & {
      ptr_->line(line, strength, dash, roundedCorners);
    }

    ///
    /// Draw box
    ///
    /// @param[in]  box   The box
    ///
    /// @return     vector of rectangles
    auto box(const Box &box = Box{}) -> ref & {ptr_->box(box);}

    ///
    /// Get window size
    ///
    /// @return     Window size
    // [[nodiscard]] auto size() const -> Vector override;

    ///
    /// Move window
    ///
    /// @param[in]  area  The area
    void move(const Rectangle &area) {ptr_->move(area);}

    auto moveEvent(const Rectangle &area) -> bool {ptr_->moveEvent(area);}

    auto above(ref &window) -> bool {ptr_->above(window.ptr());}

    auto below(ref &window) -> bool {ptr_->below(window.ptr());}

    auto above(int position=0) -> bool {ptr_->above(position);}

    auto below(int position=-1) -> bool {ptr_->below(position);}

    ///
    /// Get window area
    ///
    /// @return     Window area
    [[nodiscard]] auto area() const -> Rectangle {return ptr_->area();}

    ///
    /// Get area of window text
    ///
    /// @param[in]  area  The area
    ///
    /// @return     text
    [[nodiscard]] auto text(const Rectangle &area = RectangleMax) const -> Text {return ptr_->text(area);}

    [[nodiscard]] auto ptr() -> Window * {return ptr_.get();}

    [[nodiscard]] auto reference() -> Window & {return *ptr_;}

    private:
    std::shared_ptr<Window> ptr_;
  };

  private:
  Text text_;
};

} // namespace jwezel
