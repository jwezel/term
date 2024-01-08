#pragma once

#include <term/geometry.hh>

#include "container.hh"
#include "frame_style.hh"
#include "widget.hh"

namespace jwezel::ui {

struct Frame: Widget {

  Frame() = delete;

  explicit Frame(
    FrameStyle *style=0,
    Container *parent=0,
    ui::Element *content=0,
    const Rectangle &area=RectangleDefault,
    const Char &background=Char{}
  );

  ~Frame() override = default;

  Frame(const Frame &) = delete;

  Frame(Frame &&) = delete;

  auto operator=(const Frame &) -> Frame & = delete;

  auto operator=(Frame &&) -> Frame & = delete;

  private:
  FrameStyle *style_;
  ui::Element *content_;
};

} // namespace jwezel::ui
