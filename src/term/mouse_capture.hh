#pragma once

#include "keyboard.hh"

namespace jwezel {

///
/// Tracks which mouse buttons are down
///
/// Mouse input that begins with a button press belongs together: the motion
/// while the button is down and the release must go to whatever received the
/// press, wherever the mouse is by then. Whoever routes mouse input feeds the
/// button events to this tracker to learn where such a sequence begins and
/// ends, and remembers the target of the sequence itself.
///
/// Wheel events are reported as button presses without a release, so they
/// are not tracked.
struct MouseCapture {
  enum class Change: u1 {
    none,   ///< The tracked press-and-release sequence remains in the same state.
    began,  ///< A button press began a sequence, or restarted it after a repeated press.
    ended   ///< The last held button was released, ending the sequence.
  };

  ///
  /// Account for a button event
  ///
  /// @param[in]  event  The event
  ///
  /// @return     What the event means for the sequence
  auto track(const MouseButtonEvent &event) -> Change {
    if (event.modifiers().mod5) {
      return Change::none;
    }
    const auto bit{static_cast<u1>(1U << static_cast<u1>(event.button()))};
    if (event.action() == MouseAction::Down) {
      // A button that is already down went down again: its release got lost, start over
      const auto began{pressed_ == 0 or (pressed_ & bit) != 0};
      pressed_ = began? bit: static_cast<u1>(pressed_ | bit);
      return began? Change::began: Change::none;
    }
    pressed_ = static_cast<u1>(pressed_ & ~bit);
    return pressed_ == 0? Change::ended: Change::none;
  }

  ///
  /// Whether a button is down
  [[nodiscard]] auto active() const -> bool {return pressed_ != 0;}

  private:
  u1 pressed_{0};
};

} // namespace jwezel
