#pragma once

#include "geometry.hh"
#include "surface.hh"
#include "ui/visible_element.hh"

namespace jwezel::ui {

struct Widget: virtual VisibleElement, Surface::Element
{
  explicit Widget(const Char &background=Space, const Rectangle &area=RectangleDefault, struct ui::Element *parent=0);

  Widget(const Widget &) = default;

  Widget(Widget &&) = delete;

  auto operator=(const Widget &) -> Widget & = default;

  auto operator=(Widget &&) -> Widget & = delete;

  ~Widget() override = default;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  void write(const Vector &position, const string_view &str) override;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const Vector &position, const Text &txt_) override;

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  void fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) override;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false ) -> Rectangle override;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  auto box(const Box &box) -> vector<Rectangle> override;

  [[nodiscard]] auto text(const Rectangle & area) const -> Text override {
    return text_[area];
  }

  [[nodiscard]] auto area() const -> Rectangle override;

  void move(const Rectangle &area) override;

  [[nodiscard]] auto text() const -> auto & {
    return text_;
  }

  private:
  Vector position_;
  Text text_;
};

} // namespace jwezel::ui
