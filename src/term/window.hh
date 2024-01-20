#pragma once

#include "geometry.hh"
#include "surface.hh"
#include "term/event.hh"
#include "term_interface.hh"
#include "text.hh"

namespace jwezel {

///
/// A virtually infinite window, all blank with no buffer, used to represent
/// the display in a clear state.
struct Backdrop: public Surface::Element {

  explicit Backdrop(Surface *surface);

  ///
  /// Destroy Backdrop
  ~Backdrop() override = default;

  Backdrop(const Backdrop &) = delete;

  Backdrop(Backdrop &&) = delete;

  auto operator=(const Backdrop &) -> Backdrop & = default;

  auto operator=(Backdrop &&) -> Backdrop & = delete;

  ///
  /// Get backdrop area
  ///
  /// @return     RectangleMax
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area=RectangleMax) const -> Text override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &/*area*/) override {}
};

///
/// Window
struct Window: TextElement {
  explicit Window(struct TerminalInterface *terminal, const Rectangle &area, const Char &background = Space, Window *below = 0);

  ~Window() override;

  Window(const Window &) = delete;

  Window(Window &&) = delete;

  auto operator=(const Window &) -> Window & = delete;

  auto operator=(Window &&) -> Window & = delete;

  explicit operator string() const;

  void move(const Rectangle &area) override;

  virtual auto event(const Event &event) -> bool;

  [[nodiscard]] inline auto terminal() const {return terminal_;}

  private:
  struct TerminalInterface *terminal_;
};

} // namespace jwezel
