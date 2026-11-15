#pragma once

#include "container.hh"
#include "element.hh"
#include "frame_style.hh"
#include "widget.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/update.hh>
#include <term/window.hh>

namespace jwezel::ui {

struct Window: jwezel::Window, Container {
  explicit Window(
    struct Ui &ui,
    const Char &background = Space,
    const Rectangle &area = RectangleDefault,
    const Text& title={}
  );

  Window(const Window &) = delete;

  Window(Window &&) noexcept = delete;

  auto operator =(const Window &) -> Window & = delete;

  auto operator =(Window &&) -> Window & = delete;

  ~Window() override = default;

  void focus(Widget *widget);

  [[nodiscard]] auto focus() -> Widget *;

  ///
  /// Tell the terminal where the caret of the focused widget is (or hide it)
  void updateCaret();

  [[nodiscard]] auto window() -> Window * override {return this;}

  ///
  /// Move window on the display; children keep their window-relative areas
  /// and follow
  void move(const Rectangle &area) override;

  [[nodiscard]] auto bounds() -> Rectangle override {return area();}

  auto event(const jwezel::Event &event) -> bool override;

  private:
  std::unique_ptr<FrameStyle> style_; // internal, never visible in the public API
  Text title_;
  Widget *focus_ = nullptr; // weak reference to the focused widget
};

} // namespace jwezel::ui
