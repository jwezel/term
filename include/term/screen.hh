#pragma once

#include "geometry.hh"
#include "_screen_window.hh"
#include "surface.hh"
#include "update.hh"

#include <algorithm>
#include <exception>
#include <memory>
#include <tuple>
#include <iostream>
#include <set>
#include <functional>

namespace jwezel {

using namespace screen;

// struct FragmentRef {
//   FragmentRef(const Rectangle *area, const BaseWindow *window): area(area), window(window) {}
//   const Rectangle *area;
//   const BaseWindow *window;
// };

struct ScreenError: public exception {};

struct Screen: Surface {

  ///
  /// Constructor
  ///
  /// @param[in]  area  Screen area
  Screen();

  ///
  /// Set window focus
  ///
  /// @param      window  The window
  void focus(Window *window);

  ///
  /// Get focus window
  ///
  /// @return     Focus window
  BaseWindow *focus() const;

  ///
  /// Move focus away from window
  ///
  /// The focus is moved up the zorder, unless at the end in which case it's moved down.
  ///
  /// @param      window  The window
  ///
  /// @return     false if the window had the focus or true otherwise
  ///
  bool unfocus(Window *window);

  ///
  /// Add a window
  ///
  /// @param[in]  area    The area
  /// @param      below   Window below which new one should be inserted
  /// @param      parent  The parent
  ///
  /// @return     Tuple with window pointer and screen updates
  tuple<Window *, Updates> addWindow(
    const Rectangle &area={-1, -1, -1, -1},
    const Char &background=Space,
    Window *below=0,
    Window *parent=0
  );

  ///
  /// Delete window
  ///
  /// @param      window  The window
  ///
  /// @return     Screen updates
  Updates deleteWindow(Window *window);

  ///
  /// Move or reshape window
  ///
  /// @param      window  The window
  /// @param[in]  area    The area
  ///
  /// @return     Screen updates
  Updates reshapeWindow(Window *window, const Rectangle &area);

  ///
  /// Fill window with character
  ///
  /// @param      window    The window
  /// @param[in]  fillChar  The fill character
  ///
  /// @return     Screen updates
  ///
  Updates fill(Window *window, const Char &fillChar=Space, const Rectangle &area=RectangleMax);

  ///
  /// Write text to window
  ///
  /// @param      window    The window
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  ///
  /// @return     Screen updates
  Updates text(Window *window, const Vector &position, const Text &text);

  ///
  /// Draw line
  ///
  /// @param      window          The window
  /// @param[in]  line            The line
  /// @param[in]  strength        The strength
  /// @param[in]  dash            The dash
  /// @param[in]  roundedCorners  The rounded corners
  ///
  /// @return     Screen updates
  ///
  Updates line(Window * window, const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false);

  ///
  /// Draw box
  ///
  /// @param      window  The window
  /// @param[in]  box     The box
  ///
  /// @return     Screen updates
  Updates box(Window *window, const Box &box);

  Vector relative(Window *window, const Vector &position) const;

  ///
  /// Get minimum size to accomodate all windows
  ///
  /// @param      exclude  Window to exclude
  ///
  /// @return     Minimum size
  Vector minSize(Window *exclude=0) const;

  Backdrop backdrop;
  BaseWindow *focusWindow;
};

} // namespace
