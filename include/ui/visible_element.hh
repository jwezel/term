#pragma once

#include "ui/element.hh"
#include "geometry.hh"
#include "text.hh"

#include <string_view>

namespace jwezel::ui {

struct VisibleElement: virtual Element
{
  explicit VisibleElement(const Char &background=Space, struct Element *parent=0);

  VisibleElement(const VisibleElement &) = default;

  VisibleElement(VisibleElement &&) = delete;

  auto operator=(const VisibleElement &) -> VisibleElement & = default;

  auto operator=(VisibleElement &&) -> VisibleElement & = delete;

  ~VisibleElement() override = default;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  virtual void write(const Vector &position, const string_view &str) = 0;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  virtual void write(const Vector &position, const Text &txt_) = 0;

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  virtual void fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) = 0;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  virtual auto line(
    const Line &line,
    u1 strength=1,
    u1 dash=0,
    bool roundedCorners=false
  ) -> Rectangle = 0;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  virtual auto box(const Box &box) -> vector<Rectangle> = 0;

  [[nodiscard]] auto background() const -> const auto & {return background_;}

  private:
  Char background_;
};

} // namespace jwezel::ui
