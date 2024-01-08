#pragma once

#include "container.hh"
#include "element.hh"
#include "frame_style.hh"

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

  Window(Window &&)  noexcept = delete;

  auto operator=(const Window &) -> Window & = delete;

  auto operator=(Window &&) -> Window & = delete;

  ~Window() override = default;

  [[nodiscard]] inline auto window() -> Window * override {return this;}

  private:
  struct Device: jwezel::Device {
    explicit Device(Window *window);
    void update(const jwezel::Updates &updates) override;

    private:
    Window *window_;
  };

  std::unique_ptr<FrameStyle> style_;
  Device device_;
  Surface surface_;
  Text title_;
};

} // namespace jwezel::ui
