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

  ///
  /// Get the widget that handles the mouse input directed at this widget
  ///
  /// A widget that is only a part of another one (a popup of a drop down, say)
  /// hands its mouse input to the widget it belongs to, which owns it and can
  /// therefore safely change or even remove it while handling the input.
  ///
  /// @return     The widget itself by default
  [[nodiscard]] virtual auto eventTarget() -> Widget * {return this;}

  ///
  /// The widget started or stopped being the one that gets the keyboard input
  ///
  /// This is the case when the focus moves to or from the widget within its
  /// root and when the window of the root gains or loses the terminal focus.
  ///
  /// @param[in]  focused  True if the widget gets the keyboard input now
  virtual void focusChanged(bool /*focused*/) {}

  ///
  /// A mouse button went down somewhere else than on this widget, while it has the focus
  virtual void pressedElsewhere() {}

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
