#pragma once

#include "geometry.hh"
#include "surface.hh"
#include "term.hh"
#include "ui/container.hh"
#include "ui/visible_element.hh"

namespace jwezel::ui {

struct Window: Container, VisibleElement, Surface {
  explicit Window(struct Ui &ui, const Char &background = Space, const Rectangle &area = RectangleDefault);

  Window(const Window &) = default;

  Window(Window &&) = default;

  auto operator=(const Window &) -> Window & = delete;

  auto operator=(Window &&) -> Window & = delete;

  ~Window() override = default;

  [[nodiscard]] auto area() const -> Rectangle;

  void area(const Rectangle &area);

  auto size() const -> Vector;

  auto width() const -> Dim;

  auto height() const -> Dim;

  [[nodiscard]] inline auto window() -> ui::Element * override {return this;}

  private:
  jwezel::Window window_;
};

} // namespace jwezel::ui
