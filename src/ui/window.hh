#pragma once

#include "frame_style.hh"
#include "root.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

#include <memory>

namespace jwezel::ui {

///
/// A framed window that has a UI
///
/// It is a window like any other on the terminal and the root of its UI at the
/// same time. Applications that do not need windows place their UI on the
/// desktop instead (see `Ui::desktop`).
struct Window: jwezel::Window, Root {
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

  ///
  /// Move window on the display; children keep their window-relative areas
  /// and follow
  void move(const Rectangle &area) override;

  private:
  std::unique_ptr<FrameStyle> style_; // internal, never visible in the public API
  Text title_;
};

} // namespace jwezel::ui
