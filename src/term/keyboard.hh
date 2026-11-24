#pragma once

#include <term/event.hh>
#include <term/geometry.hh>
#include <term/text.hh>
#include <util/basic.hh>

#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <termios.h>

namespace jwezel {

using std::tuple, std::unique_ptr;

/// Logical key values returned by the keyboard decoder.
///
/// Printable keys are their Unicode code points. Named keys use values beginning at `None` and cover unmodified,
/// Shift-, Ctrl-, and Alt-modified navigation/function keys. Escape is a named key, distinct from an escape-sequence
/// byte; `Mouse` identifies mouse input in legacy key-oriented APIs.
enum Key: Unicode {
  None = 0x0fff0000, ///< Sentinel indicating that no logical key was decoded.
  Left,       ///< Left arrow.
  Right,      ///< Right arrow.
  Up,         ///< Up arrow.
  Down,       ///< Down arrow.
  Insert,     ///< Insert key.
  Delete,     ///< Delete key.
  Home,       ///< Home key.
  End,        ///< End key.
  PageUp,     ///< Page Up key.
  PageDown,   ///< Page Down key.
  Backspace,  ///< Backspace key.
  Enter,      ///< Enter/Return key.
  Tab,        ///< Tab key.
  Escape, ///< The Escape key, not a raw escape-sequence byte.
  System,     ///< System-request key reported by the terminal.
  F1,         ///< Function key F1.
  F2,         ///< Function key F2.
  F3,         ///< Function key F3.
  F4,         ///< Function key F4.
  F5,         ///< Function key F5.
  F6,         ///< Function key F6.
  F7,         ///< Function key F7.
  F8,         ///< Function key F8.
  F9,         ///< Function key F9.
  F10,        ///< Function key F10.
  F11,        ///< Function key F11.
  F12,        ///< Function key F12.
  ShiftLeft,       ///< Shift + Left arrow.
  ShiftRight,      ///< Shift + Right arrow.
  ShiftUp,         ///< Shift + Up arrow.
  ShiftDown,       ///< Shift + Down arrow.
  ShiftInsert,     ///< Shift + Insert.
  ShiftDelete,     ///< Shift + Delete.
  ShiftHome,       ///< Shift + Home.
  ShiftEnd,        ///< Shift + End.
  ShiftPageUp,     ///< Shift + Page Up.
  ShiftPageDown,   ///< Shift + Page Down.
  ShiftBackspace,  ///< Shift + Backspace.
  ShiftEnter,      ///< Shift + Enter.
  ShiftTab,        ///< Shift + Tab.
  ShiftF1,         ///< Shift + F1.
  ShiftF2,         ///< Shift + F2.
  ShiftF3,         ///< Shift + F3.
  ShiftF4,         ///< Shift + F4.
  ShiftF5,         ///< Shift + F5.
  ShiftF6,         ///< Shift + F6.
  ShiftF7,         ///< Shift + F7.
  ShiftF8,         ///< Shift + F8.
  ShiftF9,         ///< Shift + F9.
  ShiftF10,        ///< Shift + F10.
  ShiftF11,        ///< Shift + F11.
  ShiftF12,        ///< Shift + F12.
  CtrlLeft,        ///< Ctrl + Left arrow.
  CtrlRight,       ///< Ctrl + Right arrow.
  CtrlUp,          ///< Ctrl + Up arrow.
  CtrlDown,        ///< Ctrl + Down arrow.
  CtrlInsert,      ///< Ctrl + Insert.
  CtrlDelete,      ///< Ctrl + Delete.
  CtrlHome,        ///< Ctrl + Home.
  CtrlEnd,         ///< Ctrl + End.
  CtrlPageUp,      ///< Ctrl + Page Up.
  CtrlPageDown,    ///< Ctrl + Page Down.
  CtrlBackspace,   ///< Ctrl + Backspace.
  CtrlEnter,       ///< Ctrl + Enter.
  CtrlTab,         ///< Ctrl + Tab.
  CtrlF1,          ///< Ctrl + F1.
  CtrlF2,          ///< Ctrl + F2.
  CtrlF3,          ///< Ctrl + F3.
  CtrlF4,          ///< Ctrl + F4.
  CtrlF5,          ///< Ctrl + F5.
  CtrlF6,          ///< Ctrl + F6.
  CtrlF7,          ///< Ctrl + F7.
  CtrlF8,          ///< Ctrl + F8.
  CtrlF9,          ///< Ctrl + F9.
  CtrlF10,         ///< Ctrl + F10.
  CtrlF11,         ///< Ctrl + F11.
  CtrlF12,         ///< Ctrl + F12.
  CtrlShiftLeft,      ///< Ctrl + Shift + Left arrow.
  CtrlShiftRight,     ///< Ctrl + Shift + Right arrow.
  CtrlShiftUp,        ///< Ctrl + Shift + Up arrow.
  CtrlShiftDown,      ///< Ctrl + Shift + Down arrow.
  CtrlShiftInsert,    ///< Ctrl + Shift + Insert.
  CtrlShiftDelete,    ///< Ctrl + Shift + Delete.
  CtrlShiftHome,      ///< Ctrl + Shift + Home.
  CtrlShiftEnd,       ///< Ctrl + Shift + End.
  CtrlShiftPageUp,    ///< Ctrl + Shift + Page Up.
  CtrlShiftPageDown,  ///< Ctrl + Shift + Page Down.
  CtrlShiftBackspace, ///< Ctrl + Shift + Backspace.
  CtrlShiftEnter,     ///< Ctrl + Shift + Enter.
  CtrlShiftTab,       ///< Ctrl + Shift + Tab.
  CtrlShiftF1,        ///< Ctrl + Shift + F1.
  CtrlShiftF2,        ///< Ctrl + Shift + F2.
  CtrlShiftF3,        ///< Ctrl + Shift + F3.
  CtrlShiftF4,        ///< Ctrl + Shift + F4.
  CtrlShiftF5,        ///< Ctrl + Shift + F5.
  CtrlShiftF6,        ///< Ctrl + Shift + F6.
  CtrlShiftF7,        ///< Ctrl + Shift + F7.
  CtrlShiftF8,        ///< Ctrl + Shift + F8.
  CtrlShiftF9,        ///< Ctrl + Shift + F9.
  CtrlShiftF10,       ///< Ctrl + Shift + F10.
  CtrlShiftF11,       ///< Ctrl + Shift + F11.
  CtrlShiftF12,       ///< Ctrl + Shift + F12.
  AltLeft,         ///< Alt + Left arrow.
  AltRight,        ///< Alt + Right arrow.
  AltUp,           ///< Alt + Up arrow.
  AltDown,         ///< Alt + Down arrow.
  AltInsert,       ///< Alt + Insert.
  AltDelete,       ///< Alt + Delete.
  AltHome,         ///< Alt + Home.
  AltEnd,          ///< Alt + End.
  AltPageUp,       ///< Alt + Page Up.
  AltPageDown,     ///< Alt + Page Down.
  AltBackspace,    ///< Alt + Backspace.
  AltEnter,        ///< Alt + Enter.
  AltTab,          ///< Alt + Tab.
  AltSystem,       ///< Alt + system-request key.
  AltA,            ///< Alt + A.
  AltB,            ///< Alt + B.
  AltC,            ///< Alt + C.
  AltD,            ///< Alt + D.
  AltE,            ///< Alt + E.
  AltF,            ///< Alt + F.
  AltG,            ///< Alt + G.
  AltH,            ///< Alt + H.
  AltI,            ///< Alt + I.
  AltJ,            ///< Alt + J.
  AltK,            ///< Alt + K.
  AltL,            ///< Alt + L.
  AltM,            ///< Alt + M.
  AltN,            ///< Alt + N.
  AltO,            ///< Alt + O.
  AltP,            ///< Alt + P.
  AltQ,            ///< Alt + Q.
  AltR,            ///< Alt + R.
  AltS,            ///< Alt + S.
  AltT,            ///< Alt + T.
  AltU,            ///< Alt + U.
  AltV,            ///< Alt + V.
  AltW,            ///< Alt + W.
  AltX,            ///< Alt + X.
  AltY,            ///< Alt + Y.
  AltZ,            ///< Alt + Z.
  AltF1,           ///< Alt + F1.
  AltF2,           ///< Alt + F2.
  AltF3,           ///< Alt + F3.
  AltF4,           ///< Alt + F4.
  AltF5,           ///< Alt + F5.
  AltF6,           ///< Alt + F6.
  AltF7,           ///< Alt + F7.
  AltF8,           ///< Alt + F8.
  AltF9,           ///< Alt + F9.
  AltF10,          ///< Alt + F10.
  AltF11,          ///< Alt + F11.
  AltF12,          ///< Alt + F12.
  Mouse ///< Mouse input marker for legacy APIs; normal mouse input uses typed mouse events.
};

///
/// Get a logical key by its name, like "Escape", "Enter", "CtrlShiftF5" or "AltLeft"
///
/// @param[in]  name  The name; it is the name of the constant in `Key`
///
/// @return     The key, nothing if there is no such key
auto keyFromName(std::string_view name) -> std::optional<Unicode>;

///
/// Get the name of a logical key
///
/// @param[in]  key  The key
///
/// @return     The name, nothing if the key is not a logical key (but a character, say)
auto nameOfKey(Unicode key) -> std::optional<std::string_view>;

/// Broad category of terminal input events.
enum class EventType: u1 {
  Key,   ///< Keyboard input.
  Mouse  ///< Mouse input.
};

/// Whether a mouse button was pressed or released.
enum class MouseAction: u1 {
  Up,    ///< Button was released.
  Down   ///< Button was pressed.
};

/// Terminal mouse button number.
enum class MouseButton: u1 {
  Button1, ///< Usually the left button.
  Button2, ///< Usually the middle button.
  Button3, ///< Usually the right button.
  Button4, ///< Commonly the wheel up event.
  Button5, ///< Commonly the wheel down event.
  Button6, ///< Additional terminal mouse button.
  Button7, ///< Additional terminal mouse button.
  Button8  ///< Additional terminal mouse button.
};

/// Modifier state reported with a mouse event.
struct MouseModifiers {
  u1 shift: 1;   ///< Shift was held.
  u1 control: 1; ///< Control was held.
  u1 alt: 1;     ///< Alt was held.
  u1 mod4: 1;    ///< Terminal-specific modifier 4 was held.
  u1 mod5: 1;    ///< Terminal-specific modifier 5 was held.
  u1 mod6: 1;    ///< Terminal-specific modifier 6 was held.
};

/// Base type of terminal keyboard and mouse events.
struct InputEvent: public BaseEvent {
  CLASS_ID(InputEvent);
};

/// A decoded logical key or printable Unicode code point.
struct KeyEvent: public InputEvent {
  [[nodiscard]] explicit KeyEvent(Unicode key):
    key_(key) {}

  [[nodiscard]] inline auto key() const -> auto {return key_;}

  private:
  Unicode key_;

  CLASS_ID(KeyEvent);
};

/// Base mouse event with a position relative to the terminal display.
struct MouseEvent: public InputEvent {
  explicit MouseEvent(const  Vector &position):
  position_{position}
  {}

  [[nodiscard]] inline auto position() const {return position_;}

  virtual auto translated(const Vector &shift) -> MouseEvent;

  private:
  Vector position_;

  CLASS_ID(MouseEvent);
};

/// Mouse button press or release, including button and modifier state.
struct MouseButtonEvent: MouseEvent {
  MouseButtonEvent(MouseButton button, const MouseModifiers &modifiers, const Vector &position, MouseAction action):
  MouseEvent{position},
  button_(button), modifiers_(modifiers), action_(action)
  {}

  [[nodiscard]] inline auto button() const {return button_;}

  [[nodiscard]] inline auto modifiers() const {return modifiers_;}

  [[nodiscard]] inline auto action() const {return action_;}

  private:
  MouseButton button_;
  MouseModifiers modifiers_;
  MouseAction action_;

  CLASS_ID(MouseButtonEvent);
};

/// Mouse pointer movement reported by the terminal.
struct MouseMoveEvent: MouseEvent {
  explicit MouseMoveEvent(const Vector &position):
  MouseEvent{position}
  {}

  CLASS_ID(MouseMoveEvent);
};

/// Reads terminal input and decodes key and mouse events.
struct Keyboard {

  ///
  /// Key prefix tree node
  struct PrefixNode {
    Unicode key = Key::None;
    std::map<char, unique_ptr<PrefixNode>> nodes{};
  };

  ///
  /// Constructor
  ///
  /// @param[in]  device  Terminal input file descriptor
  /// @param[in]  offset  Position offset applied to mouse input
  explicit Keyboard(int device=0, const Vector &offset=Vector{0, 0});

  Keyboard(const Keyboard &) = default;

  Keyboard(Keyboard &&) = delete;

  auto operator=(const Keyboard &) -> Keyboard & = default;

  auto operator=(Keyboard &&) -> Keyboard & = delete;

  ///
  /// Destructor
  ~Keyboard();

  ///
  /// Set terminal to raw mode
  void raw();

  ///
  /// Reset terminal to original state
  void reset();

  ///
  /// Get key
  ///
  /// @return     key
  [[nodiscard]] auto key() -> Unicode;

  ///
  /// Mouse report
  ///
  /// @return     Terminal mouse event data
  auto mouseReport() -> tuple<MouseButton, MouseModifiers, Vector, MouseAction>;

  [[nodiscard]] auto keyPrefixes() const -> const PrefixNode & {return keyPrefixes_;}
  ///
  /// Get input event
  ///
  /// @return     The input event.
  [[nodiscard]] auto event() -> Event;

  inline void displayOffset(const Vector &offset) {
    displayOffset_ = offset;
  }

  private:
  ///
  /// A byte of input and how long it took to arrive
  struct Byte {
    char value;
    std::chrono::steady_clock::duration waited;
  };

  ///
  /// Get the next byte of input
  ///
  /// Bytes that were put back are the first ones. If the input has to be waited for, the byte is waited for until
  /// it comes; otherwise it is only waited for a moment, which is what a sequence of keys that is sent by the
  /// terminal gets to arrive.
  ///
  /// @param[in]  wait  Whether to wait until there is a byte
  ///
  /// @return     The byte, nothing if there is none (in time or at all)
  auto readByte(bool wait) -> std::optional<Byte>;

  std::deque<char> pending_; //< Bytes that were read, but are not used yet
  PrefixNode keyPrefixes_; //< Key prefix tree
  int fd_; //< Terminal file descriptor
  std::optional<termios> originalState_; //< Original terminal state
  Vector displayOffset_;
};

} // namespace jwezel
