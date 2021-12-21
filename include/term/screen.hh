#pragma once

#include <exception>
#include <memory>
#include <tuple>
#include <iostream>

#include "geometry.hh"
#include "window.hh"
#include "update.hh"

namespace jwezel {

struct Fragment {
  Rectangle area;
  Window *window;

  operator string() const;
};

struct ScreenError: public exception {};

struct Screen {

  ///
  /// Constructor
  ///
  /// @param[in]  area  Screen area
  Screen(const Vector &size, const Char &background=Space);

  ///
  /// Split window into fragments
  ///
  /// @param[in]  area        The area
  /// @param      fragments1  Result fragments
  /// @param[in]  fragments2  Source fragments
  void split(const Rectangle &area, vector<Rectangle> &fragments1, const vector<Rectangle> fragments2);

  ///
  /// Generate regions to update screen
  ///
  /// @param[in]  fragments  The fragments
  ///
  /// @return     vector with updates
  Updates screenUpdates(const vector<Fragment> &fragments);

  ///
  /// Set window focus
  ///
  /// @param      window  The window
  void focus(Window *window);

  ///
  /// Get focus window
  ///
  /// @return     Focus window
  Window *focus() const;

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
  Updates fill(Window *window, const Char &fillChar=Space);

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
  /// Get window *
  ///
  /// @param[in]  window  The window
  ///
  /// @return     Window *
  Window * operator[](int window) const;

  ///
  /// Get window ID
  ///
  /// @param[in]  window  The window
  ///
  /// @return     Window ID
  int operator[](const Window *window) const;

  ///
  /// Resize screen
  ///
  /// @param[in]  size  The size
  ///
  /// @return     Screen updates
  Updates resize(const Vector &size);

  ///
  /// Get minimum size to accomodate all windows
  ///
  /// @return     Minimum size
  ///
  Vector minSize() const;

  Window *screen;
  vector<Window *> zorder;
  vector<unique_ptr<Window>> windows;
  Window *focusWindow;
};

} // namespace
