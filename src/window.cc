#include "geometry.hh"
#include "surface.hh"
#include "text.hh"
#include "window.hh"
#include "term.hh"

#include <format>
#include <iostream>

namespace jwezel {

using std::format;

using std::cerr;

namespace impl
{

//~impl::BaseWindow~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// Base Window
struct BaseWindow: Element {

  ///
  /// Constructor
  ///
  /// @param      terminal    The terminal
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  explicit BaseWindow(
    struct jwezel::BaseWindow *api,
    struct jwezel::Terminal &terminal,
    const Rectangle &area,
    const Char &background=Space
  );

  ~BaseWindow() override = default;

  BaseWindow(const BaseWindow &) = default;

  BaseWindow(BaseWindow &&) = default;

  auto operator=(const BaseWindow &) -> BaseWindow & = delete;

  auto operator=(BaseWindow &&) -> BaseWindow & = delete;

  [[nodiscard]] inline auto terminal() const {return terminal_;}

  [[nodiscard]] inline auto position() const {return position_;}

  void position(const Rectangle &area);

  inline auto moveEvent(const Rectangle &/*area*/) -> bool override {return true;}

  virtual inline auto deleteEvent() -> bool {return true;}

  [[nodiscard]] inline auto background() const {return background_;}

  [[nodiscard]] inline auto api() const -> struct jwezel::BaseWindow * {return api_;}

  private:
  struct jwezel::BaseWindow * const api_;
  jwezel::Terminal terminal_;
  Vector position_;
  Char background_;
};

BaseWindow::BaseWindow(jwezel::BaseWindow *api, jwezel::Terminal &terminal, const Rectangle &area, const Char &background):
Element(terminal.screen(), area),
api_{api},
terminal_{terminal},
position_{area.position()},
background_{background}
{}

void BaseWindow::position(const Rectangle &area) {
  position_ = area.position();
}

//~impl::Backdrop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// A virtually infinite window, all blank with no buffer, used to represent
/// the display in a clear state.
struct Backdrop: public BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  explicit Backdrop(jwezel::Backdrop *api, struct jwezel::Terminal &terminal, const Char &background=Space);

  Backdrop(const Backdrop &) = delete;

  Backdrop(Backdrop &&) = delete;

  auto operator=(const Backdrop &) -> Backdrop & = delete;

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

Backdrop::Backdrop(jwezel::Backdrop *api, jwezel::Terminal &terminal, const Char &background):
BaseWindow(api, terminal, Rectangle{Vector{0, 0}, VectorMax}, background)
{}

Backdrop::~Backdrop() = default;

auto Backdrop::area() const -> Rectangle {
  return Rectangle{Vector{0, 0}, VectorMax};
}

auto Backdrop::text(const Rectangle &area) const -> Text {
  return Text{Space, area.size()};
}

//~impl::Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// Window
struct Window: public BaseWindow {
  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  explicit Window(
    jwezel::Window *api,
    struct jwezel::Terminal &terminal,
    const Rectangle &area,
    const Char &background=Space,
    Window *below=0
  );

  Window() = delete;

  Window(const Window &) = default;

  Window(Window &&) = default;

  auto operator=(const Window &) -> Window & = delete;

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

  ///
  /// Move event notification
  ///
  /// @param[in]  area  The area
  ///
  /// @return     Whether event was acknowledged
  ///
  auto moveEvent(const Rectangle &area) -> bool override;

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

Window::Window(jwezel::Window *api, jwezel::Terminal &terminal, const Rectangle &area, const Char &background, Window *below):
BaseWindow{api, terminal, area, background}
{
  text_.extend(area.size(), background);
  terminal.registerWindow(this->api());
  terminal.screen().addElement(this, below);
}

Window::~Window() {
  try {
    surface_.deleteElement(this);
    terminal().contract();
  } catch (std::exception & error) {
    cerr << error.what() << '\n';
  }
}

Window::operator string() const {
  return format("Window(id={}, area={})", long(this), string(area()));
}

auto Window::write(const Vector &position, const string_view &str) -> Window & {
  return write(position, Text(str, RgbNone, RgbNone, {}, AttributeMode::mix));
}

auto Window::write(const Vector &position, const Text &txt_) -> Window & {
  text_.patch(txt_, position);
  update({Rectangle{position, position + txt_.size()}});
  return *this;
}

auto Window::fill(const Char &fillChar, const Rectangle &area) -> Window & {
  update({text_.fill(fillChar, area)});
  return *this;
}

auto Window::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Window & {
  update({text_.line(line, strength, dash, roundedCorners)});
  return *this;
}

auto Window::box(const Box &box) -> Window & {
  update(text_.box(box));
  return *this;
}

void Window::move(const Rectangle &area) {
  terminal().moveWindow(*this->api(), area);
  surface_.reshapeElement(this, area);
}

auto Window::moveEvent(const Rectangle &area) -> bool {
  position(area);
  text_.resize(area.size(), background());
  return true;
}

auto Window::area() const -> Rectangle {
  return Rectangle{position(), position() + text_.size()};
}

auto Window::text(const Rectangle &area) const -> Text {
  return text_[area];
}

} // namespace impl

//~Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Window::Window(
  struct Terminal &terminal,
  const Rectangle &area,
  const Char &background,
  const Window &below
):
p_{new impl::Window{this, terminal, area, background, below.ptr()}}
{}

Window::Window(impl::Window *window):
p_{window}
{}

/// String representation of Window
[[nodiscard]] Window::operator string() const {
  return string(*p_);
}

///
/// Write string to window
///
/// @param[in]  position  The position
/// @param[in]  str       The text
auto Window::write(const Vector &position, const string_view &str) -> Window & {
  p_->write(position, str); return *this;
}

///
/// Write text to window
///
/// @param[in]  position  The position
/// @param[in]  text      The text
auto Window::write(const Vector &position, const Text &txt_) -> Window & {
  p_->write(position, txt_); return *this;
}

///
/// Fill window with Char
///
/// @param[in]  fillChar  The fill character
/// @param[in]  area      The area
auto Window::fill(const Char &fillChar, const Rectangle &area) -> Window & {
  p_->fill(fillChar, area); return *this;
}

///
/// Draw line
///
/// @param[in]  line            Line
/// @param[in]  strength        Line strength
/// @param[in]  dash            Dash mode
/// @param[in]  roundedCorners  Whether to round corners
///
/// @return     Rectangle
auto Window::line(const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Window & {
  p_->line(line, strength, dash, roundedCorners);
  return *this;
}

///
/// Draw box
///
/// @param[in]  box   The box
///
/// @return     vector of rectangles
auto Window::box(const Box &box) -> Window & {
  p_->box(box); return *this;
}

///
/// Get window size
///
/// @return     Window size
// [[nodiscard]] auto size() const -> Vector override;

///
/// Move window
///
/// @param[in]  area  The area
auto Window::move(const Rectangle &area) -> Window & {
  p_->move(area); return *this;
}

bool Window::above(Window &window) {
  return p_->above(window.ptr());
}

bool Window::above(int position) {
  return p_->above(position);
}

bool Window::below(Window &window) {
  return p_->below(window.ptr());
}

bool Window::below(int position) {
  return p_->below(position);
}

///
/// Get window area
///
/// @return     Window area
[[nodiscard]] auto Window::area() const -> Rectangle {
  return p_->area();
}

///
/// Get area of window text
///
/// @param[in]  area  The area
///
/// @return     text
[[nodiscard]] auto Window::text(const Rectangle &area) const -> Text {
  return p_->text(area);
}

[[nodiscard]] auto Window::fragments() const -> const vector<Fragment> & {
  return p_->fragments_;
}

[[nodiscard]] auto Window::element() const -> Element * {
  return dynamic_cast<Element *>(p_.get());
}

//~Backdrop~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Backdrop::Backdrop(struct jwezel::Terminal &terminal, const Char &background):
p_{new impl::Backdrop{this, terminal, background}}
{}

///
/// Get backdrop area
///
/// @return     RectangleMax
[[nodiscard]] auto Backdrop::area() const -> Rectangle {return p_->area();}

///
/// Get area of window text
///
/// @param[in]  area  The area
///
/// @return     text
[[nodiscard]] auto Backdrop::text(const Rectangle &area) const -> Text {return p_->text(area);}

[[nodiscard]] auto Backdrop::fragments() const -> const vector<Fragment> & {
  return p_->fragments_;
}

///
/// Move window
///
/// @param[in]  area  The area
void Backdrop::move(const Rectangle &area) {p_->move(area);}

[[nodiscard]] auto Backdrop::element() const -> Element * {
  return dynamic_cast<Element *>(p_.get());
}

}  // namespace jwezel
