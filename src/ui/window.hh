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
/// The decoration, the color style and the UI style are optional weak references that must outlive the window.
/// The decoration is the one given, or that of the UI style, or `defaultDecoration()`. A color style or UI
/// style that is not given is the default one, which is consulted whenever the window is drawn. The styles
/// are those of everything in the window: its widgets get their glyphs and colors from them.
struct Window: jwezel::Window, Root {
  /// Create a decorated terminal window that is also the root of its UI tree.
  ///
  /// A null decoration, color style, or UI style selects the corresponding default. Non-null style pointers are
  /// non-owning and must remain alive for the lifetime of the window.
  /// @param ui The UI whose terminal hosts the window.
  /// @param background The window's explicit background cell.
  /// @param area Window area in display coordinates, or `RectangleDefault` for a default-sized window.
  /// @param title Text drawn in the window decoration.
  /// @param decoration Optional window decoration.
  /// @param colors Optional color-role table.
  /// @param style Optional glyph and decoration style.
  explicit Window(
    struct Ui &ui,
    const Char &background = Space,
    const Rectangle &area = RectangleDefault,
    const Text &title = {},
    const Decoration *decoration = nullptr,
    const ColorStyle *colors = nullptr,
    const UiStyle *style = nullptr
  );

  Window(const Window &) = delete;

  Window(Window &&) noexcept = delete;

  auto operator =(const Window &) -> Window & = delete;

  auto operator =(Window &&) -> Window & = delete;

  ~Window() override = default;

  ///
  /// The area inside the frame, in content-relative coordinates
  [[nodiscard]] auto content() const -> Rectangle override;

  ///
  /// Get the display position where the content area starts
  ///
  /// The content area starts there, which the frame of a window puts after its insets.
  ///
  /// @return     The position of the content area on the display
  [[nodiscard]] auto contentOrigin() const -> Vector override;

  ///
  /// Move window on the display; children keep their content-relative areas
  /// and follow
  void move(const Rectangle &area) override;

  [[nodiscard]] auto title() const -> const Text & {return framing_.title;}

  void title(const Text &title);

  ///
  /// Get the decoration that draws the frame
  [[nodiscard]] auto decoration() const -> const Decoration &;

  ///
  /// Set the decoration (weak reference, nullptr for the one of the UI style); the content is laid out again
  void decoration(const Decoration *decoration);

  ///
  /// Get the state, which is active while the window has the focus
  [[nodiscard]] auto state() const -> DecorationState {return framing_.state;}

  ///
  /// Draw the frame and all elements again
  void restyle() override;

  void handleGeometry(const Rectangle &previous) override;

  void handleFocus() override;

  void handleBlur() override;

  private:
  void state(DecorationState state);

  void decorate();

  Framing framing_;
  const Decoration *decoration_{}; // weak reference; nullptr for the one of the UI style
};

} // namespace jwezel::ui
