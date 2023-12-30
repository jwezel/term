#pragma once

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <termios.h>

#include "basic.hh"
#include "event.hh"
#include "geometry.hh"
#include "text.hh"

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
  u1 mod4: 1;
  u1 mod5: 1;
  u1 mod6: 1;
};

struct InputEvent: public Event {
  CLASS_ID(InputEvent);
};

struct KeyEvent: public InputEvent {
  [[nodiscard]] explicit KeyEvent(Unicode key):
    key_(key) {}

  [[nodiscard]] inline auto key() const -> auto {return key_;}

  private:
  Unicode key_;

  CLASS_ID(KeyEvent);
};

struct MouseEvent: public InputEvent {
  CLASS_ID(MouseEvent);
};

struct MouseButtonEvent: MouseEvent {
  MouseButtonEvent(MouseButton button, const MouseModifiers &modifiers, u2 column, u2 line, MouseAction action):
  button_(button), modifiers_(modifiers), column_(column), line_(line), action_(action)
  {}

  [[nodiscard]] inline auto button() const {return button_;}

  [[nodiscard]] inline auto modifiers() const {return modifiers_;}

  [[nodiscard]] inline auto column() const {return column_;}

  [[nodiscard]] inline auto line() const {return line_;}

  [[nodiscard]] inline auto action() const {return action_;}

  private:
  MouseButton button_;
  MouseModifiers modifiers_;
  u2 column_;
  u2 line_;
  MouseAction action_;

  CLASS_ID(MouseButtonEvent);
};

struct MouseMoveEvent: MouseEvent {
  MouseMoveEvent(u2 column, u2 line): column_{column}, line_{line} {}

  [[nodiscard]] inline auto column() const {return column_;}

  [[nodiscard]] inline auto line() const {return line_;}

  private:
  u2 column_;
  u2 line_;

  CLASS_ID(MouseMoveEvent);
};

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
  /// @param[in]  device  Terminal device
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
  auto mouseReport() -> tuple<MouseButton, MouseModifiers, u2, u2, MouseAction>;

  [[nodiscard]] auto keyPrefixes() const -> const PrefixNode & {return keyPrefixes_;}
  ///
  /// Get input event
  ///
  /// @return     The input event.
  [[nodiscard]] auto event() -> InputEvent *;

  inline void displayOffset(const Vector &offset) {
    displayOffset_ = offset;
  }

  struct ref
  {
    inline explicit ref(int device=0, const Vector &offset=Vector{0, 0}):
    ptr_{new Keyboard{device, offset}}
    {}

    ref(const ref &) = default;

    ref(ref &&) = default;

    auto operator=(const ref &) -> ref & = default;

    auto operator=(ref &&) -> ref & = default;

    ///
    /// Destructor
    ~ref() = default;

    ///
    /// Set terminal to raw mode
    inline void raw() {ptr_->raw();}

    ///
    /// Reset terminal to original state
    inline void reset() {ptr_->reset();}

    ///
    /// Get key
    ///
    /// @return     key
    [[nodiscard]] inline auto key() -> Unicode {return ptr_->key();}

    ///
    /// Mouse report
    ///
    /// @return     Terminal mouse event data
    inline auto mouseReport() -> tuple<MouseButton, MouseModifiers, u2, u2, MouseAction> {return ptr_->mouseReport();}

    [[nodiscard]] auto keyPrefixes() const -> const PrefixNode & {return ptr_->keyPrefixes();}
    ///
    /// Get input event
    ///
    /// @return     The input event.
    [[nodiscard]] auto event() -> InputEvent * {return ptr_->event();}

    inline void displayOffset(const Vector &offset) {
      ptr_->displayOffset(offset);
    }

    private:
    std::shared_ptr<Keyboard> ptr_;
  };

  private:
  std::deque<Unicode> keyBuffer_; //< Key buffer
  PrefixNode keyPrefixes_; //< Key prefix tree
  int fd_; //< Terminal file descriptor
  std::optional<termios> originalState_; //< Original terminal state
  Vector displayOffset_;
};

} // namespace jwezel
