#pragma once

#include "color_style.hh"
#include "container.hh"
#include "decoration.hh"
#include "widget.hh"

#include <term/geometry.hh>

namespace jwezel::ui {

///
/// Frame around other elements
///
/// A frame is drawn by a decoration like the frame of a window and has a state, but it
/// is not a window and does not follow the focus: its state is set by whoever
/// knows when it should look active.
struct Frame: Widget {

  Frame() = delete;

  ///
  /// Create Frame
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      background  The background
  /// @param[in]  decoration  Optional decoration (weak reference; must outlive the frame)
  /// @param      content     Optional content element (weak reference; must outlive the frame)
  /// @param      title       The title
  /// @param[in]  colors      Optional color style (weak reference; must outlive the frame)
  explicit Frame(
    Container &parent,
    const Rectangle &area=RectangleDefault,
    const Char &background=InheritBackground,
    const Decoration *decoration=nullptr,
    ui::Element *content=nullptr,
    const Text &title={},
    const ColorStyle *colors=nullptr
  );

  ~Frame() override = default;

  Frame(const Frame &) = delete;

  Frame(Frame &&) = delete;

  auto operator=(const Frame &) -> Frame & = delete;

  auto operator=(Frame &&) -> Frame & = delete;

  [[nodiscard]] auto state() const -> DecorationState {return framing_.state;}

  void state(DecorationState state);

  [[nodiscard]] auto title() const -> const Text & {return framing_.title;}

  void title(const Text &title);

  protected:
  void render() override;

  private:
  Framing framing_;
  ui::Element *content_;         // weak reference
};

} // namespace jwezel::ui
