#pragma once

#include "color_style.hh"
#include "decoration.hh"
#include "root.hh"

#include <term/geometry.hh>
#include <term/surface.hh>
#include <term/window.hh>

namespace jwezel::ui {

///
/// A decorated window that has a UI
///
/// It is a window like any other on the terminal and the root of its UI at the same
/// time. Its frame is drawn by a decoration, which looks different when the window has the
/// focus and when it has not, and gets its colors from a color style. Applications that do not
/// need windows place their UI on the desktop instead (see `Ui::desktop`).
///
/// The decoration and the color style are optional weak references that must outlive the window.
/// Those that are not given are the defaults of `defaultDecoration()` and `defaultColorStyle()`,
/// which are consulted whenever the window is drawn.
struct Window: jwezel::Window, Root {
  explicit Window(
    struct Ui &ui,
    const Char &background = Space,
    const Rectangle &area = RectangleDefault,
    const Text &title = {},
    const Decoration *decoration = nullptr,
    const ColorStyle *colors = nullptr
  );

  Window(const Window &) = delete;

  Window(Window &&) noexcept = delete;

  auto operator =(const Window &) -> Window & = delete;

  auto operator =(Window &&) -> Window & = delete;

  ~Window() override = default;

  ///
  /// Move window on the display; children keep their window-relative areas
  /// and follow
  void move(const Rectangle &area) override;

  ///
  /// The area inside the frame
  [[nodiscard]] auto content() const -> Rectangle override;

  [[nodiscard]] auto title() const -> const Text & {return title_;}

  void title(const Text &title);

  ///
  /// Get the decoration that draws the frame
  [[nodiscard]] auto decoration() const -> const Decoration &;

  ///
  /// Set the decoration (weak reference, nullptr for the default); the content is laid out again
  void decoration(const Decoration *decoration);

  ///
  /// Get the colors of the frame
  [[nodiscard]] auto colors() const -> const ColorStyle &;

  ///
  /// Set the color style (weak reference, nullptr for the default)
  void colors(const ColorStyle *colors);

  ///
  /// Get the state, which is active while the window has the focus
  [[nodiscard]] auto state() const -> DecorationState {return state_;}

  void handleGeometry(const Rectangle &previous) override;

  void handleFocus() override;

  void handleBlur() override;

  private:
  void state(DecorationState state);

  void decorate();

  Text title_;
  const Decoration *decoration_; // weak reference, nullptr for the default
  const ColorStyle *colors_;     // weak reference, nullptr for the default
  DecorationState state_;
};

} // namespace jwezel::ui
