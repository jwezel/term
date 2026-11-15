#pragma once

#include <term/geometry.hh>

#include "container.hh"
#include "frame_style.hh"
#include "widget.hh"

namespace jwezel::ui {

struct Frame: Widget {

  Frame() = delete;

  ///
  /// Create Frame
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      background  The background
  /// @param[in]  style       Optional frame style (weak reference; must outlive the frame)
  /// @param      content     Optional content element (weak reference; must outlive the frame)
  explicit Frame(
    Container &parent,
    const Rectangle &area=RectangleDefault,
    const Char &background=Char{},
    FrameStyle *style=nullptr,
    ui::Element *content=nullptr
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
