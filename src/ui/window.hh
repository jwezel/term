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

  [[nodiscard]] auto window() -> Window * override {return this;}

  void move(const Rectangle &area) override;

  auto event(const jwezel::Event &event) -> bool override;

  private:
  struct Device: jwezel::Device {
    explicit Device(Window *window);
    void update(const jwezel::Updates &updates) override;

    private:
    Window *window_;
  };

  std::unique_ptr<FrameStyle> style_; // internal, never visible in the public API
  Device device_;
  Surface surface_;
  Text title_;
  Widget *focus_ = nullptr; // weak reference to the focused widget
};

} // namespace jwezel::ui
