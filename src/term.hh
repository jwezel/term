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

  virtual ~Terminal() = default;

  Terminal(const Terminal &) = default;

  Terminal(Terminal &&) = delete;

  auto operator=(const Terminal &) -> Terminal & = default;

  auto operator=(Terminal &&) -> Terminal & = delete;

  ///
  /// Create window
  ///
  /// @param[in]  area        The area
  /// @param[in]  background  The background
  /// @param[in]  below       The below
  ///
  /// @return     new window
  void registerWindow(Window *window) override;

  ///
  /// Move window
  ///
  /// @param[in]  windowId  The window identifier
  /// @param[in]  area      The area
  void moveWindow(Window &window, const Rectangle &area) override;

  ///
  /// Get event
  ///
  /// @return     Event
  auto event() -> Event *;

  ///
  /// Run loop
  void run();

  auto screen() -> Surface::ref & override {return screen_;}

  auto display() -> auto & {return display_;}

  auto desktop() -> Window & {return desktop_;}

  auto keyboard() -> auto & {return keyboard_;}

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

  struct ref
  {
    explicit ref(
      const Char &background=Space,
      const Vector &initialPosition=VectorMin,
      const Vector &initialSize=VectorMin,
      const Vector &maxSize=VectorMax,
      int terminalFd=1,
      int keyboardFd=0,
      bool expand=true,
      bool contract=true
    ):
    ptr_{new Terminal{background, initialPosition, initialSize, maxSize, terminalFd, keyboardFd, expand, contract}}
    {}

    virtual ~ref() = default;

    ref(const ref &) = default;

    ref(ref &&) = default;

    auto operator=(const ref &) -> ref & = default;

    auto operator=(ref &&) -> ref & = default;

    ///
    /// Create window
    ///
    /// @param[in]  area        The area
    /// @param[in]  background  The background
    /// @param[in]  below       The below
    ///
    /// @return     new window
    void registerWindow(Window *window) {ptr_->registerWindow(window);}

    ///
    /// Move window
    ///
    /// @param[in]  windowId  The window identifier
    /// @param[in]  area      The area
    void moveWindow(Window &window, const Rectangle &area) {ptr_->moveWindow(window, area);}

    ///
    /// Get event
    ///
    /// @return     Event
    [[nodiscard]] auto event() -> Event * {return ptr_->event();}

    ///
    /// Run loop
    void run() {ptr_->run();}

    auto screen() -> Surface::ref & {return ptr_->screen();}

    auto display() -> auto & {return ptr_->display();}

    auto desktop() -> Window & {return ptr_->desktop();}

    auto keyboard() -> auto & {return ptr_->keyboard();}

    ///
    /// Possibly expand display and screen
    ///
    /// @param[in]  size  The size
    auto expand(const Vector &size) -> bool {return ptr_->expand(size);}

    ///
    /// Possibly contract display and screen
    ///
    /// @param[in]  size  The size
    auto contract() -> bool {return ptr_->contract();}

    [[nodiscard]] auto ptr() -> Terminal * {return ptr_.get();}

    [[nodiscard]] auto reference() -> Terminal & {return *ptr_;}

    private:
    std::shared_ptr<Terminal> ptr_;
  };

  private:
  Keyboard::ref keyboard_;
  Display::ref display_;
  Backdrop backdrop_;
  Surface::ref screen_;
  Window desktop_;
  Window *focusWindow_;
  Vector minimumSize_;
  bool expand_;
  bool contract_;
  bool running_;
};

} // namespace jwezel
