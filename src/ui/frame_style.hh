#pragma once

#include <term/geometry.hh>

namespace jwezel::ui {

struct FrameStyle {
  FrameStyle() = default;

  FrameStyle(const FrameStyle &) = delete;

  FrameStyle(FrameStyle &&) = delete;

  auto operator=(const FrameStyle &) -> FrameStyle & = delete;

  auto operator=(FrameStyle &&) -> FrameStyle & = delete;

  virtual ~FrameStyle() = default;

  virtual void draw() = 0;

  ///
  /// Width of the frame, which the content of the framed window has to keep clear of
  [[nodiscard]] virtual auto inset() const -> Dim {return 0;}
};

}  // namespace jwezel::ui
