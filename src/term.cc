#include "term.hh"
#include "display.hh"
#include "event.hh"
#include "geometry.hh"
#include "geometry.hh"
#include "term_interface.hh"
#include "window.hh"
#include "text.hh"

#include <unistd.h>

namespace jwezel
{

using
  jwezel::Char,
  jwezel::Event,
  jwezel::Space,
  jwezel::Rectangle,
  jwezel::Vector,
  jwezel::VectorMin,
  jwezel::VectorMax;

namespace impl {

///
/// Terminal
///
/// Responsibilites:
///   - Provide a:
///     - Display
///     - Keyboard
///   - Control display size
struct Terminal: jwezel::TerminalInterface {
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
    struct jwezel::Terminal *api,
    const Char &background=Space,
    const Vector &initialPosition=VectorMin,
    const Vector &initialSize=VectorMin,
    const Vector &maxSize=VectorMax,
    int terminalFd=1,
    int keyboardFd=0,
    bool expand=true,
    bool contract=true
  );

  Terminal(const Terminal &) = delete;

  auto operator=(const Terminal &) -> Terminal & = delete;

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
  void moveWindow(jwezel::BaseWindow &window, const Rectangle &area) override;

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

  auto backdrop() -> jwezel::Backdrop & {return backdrop_;}

  auto desktop() -> jwezel::Window & {return desktop_;}

  auto keyboard() -> jwezel::Keyboard & {return keyboard_;}

  auto api() -> jwezel::Terminal * {return api_;}

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
  struct jwezel::Terminal *api_;
  bool expand_;
  bool contract_;
  jwezel::Keyboard keyboard_;
  jwezel::Display display_;
  jwezel::Backdrop backdrop_;
  jwezel::Surface screen_;
  jwezel::Window desktop_;
  jwezel::Window *focusWindow_;
  Vector minimumSize_;
  bool running_;
};

Terminal::Terminal(
  struct jwezel::Terminal *api,
  const Char &background,
  const Vector &initialPosition,
  const Vector &initialSize,
  const Vector &maxSize,
  int terminalFd,
  int keyboardFd,
  bool expand,
  bool contract
):
api_{api},
expand_{expand},
contract_{contract},
keyboard_{keyboardFd == -1? terminalFd: keyboardFd},
display_{keyboard_, terminalFd, initialPosition, initialSize == VectorMin? Vector{1, 1}: initialSize, maxSize},
backdrop_{*this->api()},
screen_{&display_, {backdrop_.element()}},
desktop_{*api, Rectangle{Vector{0, 0}, display_.size()}, background},
focusWindow_{&desktop_},
minimumSize_{display_.size()},
running_{false}
{}

void Terminal::registerWindow(jwezel::BaseWindow *window) {
  focusWindow_ = dynamic_cast<jwezel::Window *>(window);
  expand(Vector{window->area().x2(), window->area().y2()});
}

void Terminal::moveWindow(jwezel::BaseWindow &window, const Rectangle &area) {
  expand(Vector{area.x2(), area.y2()});
  screen_.reshapeElement(window.element(), area);
  contract();
}

auto Terminal::event() -> Event * {
  return keyboard_.event();
}

void Terminal::run() {
  while (running_) {
    keyboard_.event();
  }
}

auto Terminal::expand(const Vector &size) -> bool {
  if (!expand_) {
    return false;
  }
  const auto size_ = max(min(display_.maxSize(), size), display_.size());
  if (size_ != display_.size()) {
    display_.resize(size_);
    screen_.reshapeElement(desktop_.element(), Rectangle{Vector{0, 0}, size_});
    return true;
  }
  return false;
}

auto Terminal::contract() -> bool {
  if (!contract_) {
    return false;
  }
  const auto size_{max(max(screen_.minSize(desktop_.element()), Vector{1, 1}), minimumSize_)};
  if (size_ != display_.size()) {
    screen_.reshapeElement(desktop_.element(), Rectangle{Vector{0, 0}, size_});
    display_.resize(size_);
    return true;
  }
  return false;
}

} // namespace impl

//~Terminal~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Terminal::Terminal(
  const Char &background,
  const Vector &initialPosition,
  const Vector &initialSize,
  const Vector &maxSize,
  int terminalFd,
  int keyboardFd,
  bool expand,
  bool contract
):
p_{new impl::Terminal{this, background, initialPosition, initialSize, maxSize, terminalFd, keyboardFd, expand, contract}}
{}

///
/// Create window
///
/// @param[in]  area        The area
/// @param[in]  background  The background
/// @param[in]  below       The below
///
/// @return     new window
void Terminal::registerWindow(BaseWindow *window) {p_->registerWindow(window);}

///
/// Move window
///
/// @param[in]  windowId  The window identifier
/// @param[in]  area      The area
void Terminal::moveWindow(BaseWindow &window, const Rectangle &area) {p_->moveWindow(window, area);}

///
/// Get event
///
/// @return     Event
[[nodiscard]] auto Terminal::event() const -> Event * {return p_->event();}

///
/// Run loop
void Terminal::run() {p_->run();}

[[nodiscard]] auto Terminal::screen() -> Surface & {return p_->screen();}

[[nodiscard]] auto Terminal::display() -> Display & {return p_->display();}

[[nodiscard]] auto Terminal::backdrop() -> Backdrop & {return p_->backdrop();}

[[nodiscard]] auto Terminal::desktop() -> Window & {return p_->desktop();}

[[nodiscard]] auto Terminal::keyboard() -> Keyboard & {return p_->keyboard();}

///
/// Possibly expand display and screen
///
/// @param[in]  size  The size
auto Terminal::expand(const Vector &size) -> bool {return p_->expand(size);}

///
/// Possibly contract display and screen
///
/// @param[in]  size  The size
auto Terminal::contract() -> bool {return p_->contract();}

}
