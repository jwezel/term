#pragma once

#include "ui/element.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

namespace jwezel::ui {

struct Widget: TextElement, ui::Element
{
  explicit Widget(const Rectangle &area, const Char &background, Container &parent);

  void move(const Rectangle &area) override;

  auto moveEvent(const Rectangle &area) -> bool override;

  Widget(const Widget &) = delete;

  Widget(Widget &&) = delete;

  auto operator=(const Widget &) -> Widget & = delete;

  auto operator=(Widget &&) -> Widget & = delete;

  ~Widget() override = default;

  protected:
  ///
  /// Render the widget content into its text buffer
  ///
  /// Called whenever the widget is moved to a new area (via moveEvent, so
  /// term-layer moves re-render as well), so content follows geometry
  /// changes without the layout knowing anything about content. Subclasses
  /// with visual content override this and call it in their own
  /// constructor as well.
  virtual void render() {}

};

} // namespace jwezel::ui
