#include <exception>
#include <geometry.hh>
#include <tuple>
#include <window.hh>

#pragma once

namespace jwezel {

struct Fragment {
  Rectangle area;
  Window *window;

  operator string() const;
};

struct ScreenError: public exception {};

struct Screen {
  Window *screen;
  vector<Window *> zorder;
  Window *focusWindow;

  struct update {
    Vector position;
    Text text;

    operator string() const {
      return string(position) + ": " + text.repr();
    }

    bool operator ==(const update &other) const {
      return position == other.position and text == other.text;
    }

    bool operator !=(const update &other) const {
      return !(*this == other);
    }

    bool operator <(const update &other) const {
      return make_tuple(position.x, position.y) < make_tuple(other.position.x, other.position.y);
    }

    bool operator >(const update &other) const {
      return other < *this;
    }

    bool operator <=(const update &other) const {
      return !(*this < other);
    }

    bool operator >=(const update &other) const {
      return !(other < *this);
    }
  };

  using updates = vector<update>;

  ///
  /// Constructor
  ///
  /// @param[in]  area  Screen area
  Screen(const Rectangle &area);

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
  updates screenUpdates(const vector<Fragment> &fragments);

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
  tuple<Window *, updates> addWindow(const Rectangle &area={-1, -1, -1, -1}, Window *below=0, Window *parent=0);

  ///
  /// Delete window
  ///
  /// @param      window  The window
  ///
  /// @return     Screen updates
  updates deleteWindow(Window *window);

  ///
  /// Move or reshape window
  ///
  /// @param      window  The window
  /// @param[in]  area    The area
  ///
  /// @return     Screen updates
  updates reshapeWindow(Window *window, const Rectangle &area);

  ///
  /// Fill window with character
  ///
  /// @param      window    The window
  /// @param[in]  fillChar  The fill character
  ///
  /// @return     Screen updates
  ///
  updates fill(Window *window, const Char &fillChar=Space);

  ///
  /// Write text to window
  ///
  /// @param      window    The window
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  ///
  /// @return     Screen updates
  updates text(Window *window, const Vector &position, const Text &text);

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
  updates line(Window * window, const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false);

  updates box(Window *window, const Box &box);

  Vector relative(Window *window, const Vector &position);
};

} // namespace
