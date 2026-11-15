#pragma once

#include "ui/element.hh"

#include <term/geometry.hh>
#include <term/surface.hh>

#include <optional>

namespace jwezel::ui {

struct Widget: TextElement, ui::Element
{
  explicit Widget(const Rectangle &area, const Char &background, Container &parent);

  void move(const Rectangle &area) override;

  [[nodiscard]] auto bounds() -> Rectangle override {return bounds_;}

  [[nodiscard]] auto inputWindow() -> jwezel::Window * override;

  auto moveEvent(const Rectangle &area) -> bool override;

  Widget(const Widget &) = delete;

  Widget(Widget &&) = delete;

  auto operator=(const Widget &) -> Widget & = delete;

  auto operator=(Widget &&) -> Widget & = delete;

  ~Widget() override;

  ///
  /// Where the text caret (hardware cursor) of the widget is, when it has the focus
  ///
  /// @return     Display position of the caret, or nothing for no caret
  [[nodiscard]] virtual auto caret() const -> std::optional<Vector> {return std::nullopt;}

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


  private:
  Rectangle bounds_; // area relative to the root
};

} // namespace jwezel::ui
