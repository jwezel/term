#pragma once

namespace jwezel::ui {

struct FrameStyle {
  FrameStyle() = default;

  FrameStyle(const FrameStyle &) = delete;

  FrameStyle(FrameStyle &&) = delete;

  auto operator=(const FrameStyle &) -> FrameStyle & = delete;

  auto operator=(FrameStyle &&) -> FrameStyle & = delete;

  virtual ~FrameStyle() = default;

  virtual void draw() = 0;
};

}  // namespace jwezel::ui
