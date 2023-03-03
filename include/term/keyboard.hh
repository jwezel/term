#pragma once

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <termios.h>

#include "basic.hh"
#include "event.hh"
#include "text.hh"
#include <typeinfo>

namespace jwezel {

using std::tuple, std::unique_ptr;

enum Key: Unicode {
  None = 0x0fff0000,
  Left,
  Right,
  Up,
  Down,
  Insert,
  Delete,
  Home,
  End,
  PageUp,
  PageDown,
  Backspace,
  Enter,
  Tab,
  System,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  ShiftLeft,
  ShiftRight,
  ShiftUp,
  ShiftDown,
  ShiftInsert,
  ShiftDelete,
  ShiftHome,
  ShiftEnd,
  ShiftPageUp,
  ShiftPageDown,
  ShiftBackspace,
  ShiftEnter,
  ShiftTab,
  ShiftF1,
  ShiftF2,
  ShiftF3,
  ShiftF4,
  ShiftF5,
  ShiftF6,
  ShiftF7,
  ShiftF8,
  ShiftF9,
  ShiftF10,
  ShiftF11,
  ShiftF12,
  CtrlLeft,
  CtrlRight,
  CtrlUp,
  CtrlDown,
  CtrlInsert,
  CtrlDelete,
  CtrlHome,
  CtrlEnd,
  CtrlPageUp,
  CtrlPageDown,
  CtrlBackspace,
  CtrlEnter,
  CtrlTab,
  CtrlF1,
  CtrlF2,
  CtrlF3,
  CtrlF4,
  CtrlF5,
  CtrlF6,
  CtrlF7,
  CtrlF8,
  CtrlF9,
  CtrlF10,
  CtrlF11,
  CtrlF12,
  CtrlShiftLeft,
  CtrlShiftRight,
  CtrlShiftUp,
  CtrlShiftDown,
  CtrlShiftInsert,
  CtrlShiftDelete,
  CtrlShiftHome,
  CtrlShiftEnd,
  CtrlShiftPageUp,
  CtrlShiftPageDown,
  CtrlShiftBackspace,
  CtrlShiftEnter,
  CtrlShiftTab,
  CtrlShiftF1,
  CtrlShiftF2,
  CtrlShiftF3,
  CtrlShiftF4,
  CtrlShiftF5,
  CtrlShiftF6,
  CtrlShiftF7,
  CtrlShiftF8,
  CtrlShiftF9,
  CtrlShiftF10,
  CtrlShiftF11,
  CtrlShiftF12,
  AltLeft,
  AltRight,
  AltUp,
  AltDown,
  AltInsert,
  AltDelete,
  AltHome,
  AltEnd,
  AltPageUp,
  AltPageDown,
  AltBackspace,
  AltEnter,
  AltTab,
  AltSystem,
  AltF1,
  AltF2,
  AltF3,
  AltF4,
  AltF5,
  AltF6,
  AltF7,
  AltF8,
  AltF9,
  AltF10,
  AltF11,
  AltF12,
  Mouse
};

enum class EventType: u1 {
  Key,
  Mouse
};

enum class MouseAction: u1 {
  Up,
  Down
};

enum class MouseButton: u1 {
  Button1,
  Button2,
  Button3,
  Button4,
  Button5,
  Button6,
  Button7,
  Button8
};

struct MouseModifiers {
  u1 shift: 1;
  u1 control: 1;
  u1 alt: 1;
};

struct InputEvent: public Event {
  CLASS_ID(InputEvent);
};

struct KeyEvent: public InputEvent {
  explicit KeyEvent(const Unicode &key): key{key} {}

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Unicode key;
  CLASS_ID(KeyEvent);
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

struct MouseEvent: public InputEvent {
  CLASS_ID(MouseEvent);
};

struct MouseButtonEvent: public MouseEvent {
  MouseButtonEvent(MouseButton button, MouseModifiers modifiers, u2 x, u2 y, MouseAction action):
  button(button), modifiers(modifiers), x(x), y(y), action(action)
  {}

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  MouseButton button;
  MouseModifiers modifiers;
  u2 x;
  u2 y;
  MouseAction action;
  CLASS_ID(MouseButtonEvent);
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

struct MouseMoveEvent: public MouseEvent {
  MouseMoveEvent(u2 x, u2 y): x{x}, y{y} {}
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  u2 x;
  u2 y;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
  CLASS_ID(MouseMoveEvent);
};

struct Keyboard {
  ///
  /// Constructor
  ///
  /// @param[in]  device  Terminal device
  explicit Keyboard(int device=0);

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
  auto key() -> Unicode;

  ///
  /// Mouse report
  ///
  /// @return     Terminal mouse event data
  auto mouseReport() -> tuple<MouseButton, MouseModifiers, u2, u2, MouseAction>;

  ///
  /// Get input event
  ///
  /// @return     The input event.
  auto event() -> InputEvent *;

  ///
  /// Key prefix tree node
  struct PrefixNode {
    Unicode key = Key::None;
    std::map<char, unique_ptr<PrefixNode>> nodes{};
  };

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  std::deque<Unicode> keyBuffer; //< Key buffer
  PrefixNode keyPrefixes; //< Key prefix tree
  int fd; //< Terminal file descriptor
  std::optional<termios> originalState; //< Original terminal state
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace jwezel
