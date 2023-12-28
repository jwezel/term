#pragma once

#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"
#include "window.hh"

#include <memory>
#include <optional>


namespace jwezel {

using std::nullopt;

namespace impl {
///
/// Terminal
///
/// Responsibilites:
///   - Provide a:
///     - Display
///     - Keyboard
///   - Control display size
struct Terminal: TerminalInterface {
  ///
  /// Constructor
  ///
  /// @param[in]  background       The background
  /// @param[in]  initialPosition  The initial position
  /// @param[in]  initialSize      The initial size
  /// @param[in]  maxSize          The maximum size
  /// @param[in]  terminalFd       The terminal fd (default stdout)
  /// @param[in]  keyboardFd       The keyboard fd (default stdin)
  /// @param[in]  expand           Whether terminal can expand (default true)
  /// @param[in]  contract         Whether terminal can contract (default true)
  explicit Terminal(
    const Char &background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true
  );

  ///
  /// Create window
  ///
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  /// @param[in]  below       The below
  ///
  /// @return     new window
  void registerWindow(jwezel::Window *window) override;

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(jwezel::Window &window, const Rectangle &area) override;

  ///
  /// Get event
  ///
  /// @return     Event
  auto event() -> Event *;

  ///
  /// Run loop
  void run();

  auto screen() -> jwezel::Surface & override {return screen_;}

  auto display() -> jwezel::Display & {return display_;}

  auto desktop() -> jwezel::Window & {return desktop_;}

  auto keyboard() -> jwezel::Keyboard & {return keyboard_;}

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  auto expand(const Vector &size) -> bool override;

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  auto contract() -> bool override;

  private:
  bool expand_;
  bool contract_;
  jwezel::Keyboard keyboard_;
  jwezel::Display display_;
  Backdrop backdrop_;
  jwezel::Surface screen_;
  jwezel::Window desktop_;
  jwezel::Window *focusWindow_;
  Vector minimumSize_;
  bool running_;
};

} // namespace impl

///
/// Terminal
///
/// Responsibilites:
///   - Provide a:
///     - Display
///     - Keyboard
///   - Control display size
struct Terminal: TerminalInterface {
  ///
  /// Constructor
  ///
  /// @param[in]  background       The background
  /// @param[in]  initialPosition  The initial position
  /// @param[in]  initialSize      The initial size
  /// @param[in]  maxSize          The maximum size
  /// @param[in]  terminalFd       The terminal fd (default stdout)
  /// @param[in]  keyboardFd       The keyboard fd (default stdin)
  /// @param[in]  expand           Whether terminal can expand (default true)
  /// @param[in]  contract         Whether terminal can contract (default true)
  explicit Terminal(
    const Char &background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true
  ):
  p_{new impl::Terminal{background, initialPosition, initialSize, maxSize, terminalFd, keyboardFd, expand, contract}}
  {}

  ///
  /// Create window
  ///
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  /// @param[in]  below       The below
  ///
  /// @return     new window
  void registerWindow(Window *window) override {p_->registerWindow(window);}

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(Window &window, const Rectangle &area) override {p_->moveWindow(window, area);}

  ///
  /// Get event
  ///
  /// @return     Event
  [[nodiscard]] auto event() const -> Event * {return p_->event();}

  ///
  /// Run loop
  void run() {p_->run();}

  [[nodiscard]] auto screen() -> Surface & override {return p_->screen();}

  [[nodiscard]] auto display() -> Display & {return p_->display();}

  [[nodiscard]] auto desktop() -> Window & {return p_->desktop();}

  [[nodiscard]] auto keyboard() -> Keyboard & {return p_->keyboard();}

  ///
  /// Possibly expand display and screen
  ///
  /// @param[in]  size  The size
  auto expand(const Vector &size) -> bool override {return p_->expand(size);}

  ///
  /// Possibly contract display and screen
  ///
  /// @param[in]  size  The size
  auto contract() -> bool override {return p_->contract();}

  private:
  std::shared_ptr<impl::Terminal> p_;
};

} // namespace jwezel
