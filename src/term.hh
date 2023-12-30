#pragma once

#include "display.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "surface.hh"
#include "term_interface.hh"
#include "text.hh"
#include "window.hh"

#include <optional>

namespace jwezel {

using std::nullopt;

namespace impl
{
  struct Terminal;
}

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

  Terminal(const Terminal &) = default;

  Terminal(Terminal &&) = default;

  auto operator=(const Terminal &) -> Terminal & = default;

  auto operator=(Terminal &&) -> Terminal & = default;

  ~Terminal() = default;

  ///
  /// Create window
  ///
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  /// @param[in]  below       The below
  ///
  /// @return     new window
  void registerWindow(jwezel::BaseWindow *window) override;

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(BaseWindow &window, const Rectangle &area) override;

  ///
  /// Get event
  ///
  /// @return     Event
  [[nodiscard]] auto event() const -> Event *;

  ///
  /// Run loop
  void run();

  [[nodiscard]] auto screen() -> Surface & override;

  [[nodiscard]] auto display() -> Display &;

  [[nodiscard]] auto desktop() -> struct Window &;

  [[nodiscard]] auto backdrop() -> struct Backdrop &;

  [[nodiscard]] auto keyboard() -> Keyboard &;

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

  inline auto ptr() const -> impl::Terminal * {return p_.get();}

  inline auto ref() const -> impl::Terminal & {return *p_.get();}

  private:
  std::shared_ptr<impl::Terminal> p_;
};

} // namespace jwezel
