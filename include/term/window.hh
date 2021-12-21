#pragma once

#include <string>
#include <basic.hh>
#include <geometry.hh>
#include <string_view>
#include <text.hh>

namespace jwezel {

///
/// Base Window
struct BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  BaseWindow(const Rectangle &area, const Char &background=Space);

  /// String representation of WindowObj
  operator string() const;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  void write(const Vector &position, const string_view &str);

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const Vector &position, const Text &text_);

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  void fill(const Char &fillChar=Space);

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  Rectangle line(
    const Line &line,
    u1 strength=1,
    u1 dash=0,
    bool roundedCorners=false
  );

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  vector<Rectangle> box(const Box &box);

  ///
  /// Get window size
  ///
  /// @return     Window size
  Vector size() const;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area);

  ///
  /// Get window area
  ///
  /// @return     Window area
  Rectangle area() const;

  ///
  /// Get window text
  ///
  /// @return     Text
  const Text &text() const;

  Vector position;
  Char background;
  vector<Rectangle> fragments;
};

struct DisplayBackground: BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  DisplayBackground(const Rectangle &area, const Char &background=Space);

  ///
  /// Get window size
  ///
  /// @return     Window size
  Vector size() const;

  ///
  /// Get window area
  ///
  /// @return     Window area
  Rectangle area() const;

  ///
  /// Get window text
  ///
  /// @return     Text
  const Text &text() const;

  Vector size_;
};

///
/// Window
struct Window: public BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  ///
  Window(const Rectangle &area, const Char &background=Space);

  /// String representation of Window
  operator string() const;

  ///
  /// Write string to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  void write(const Vector &position, const string_view &str);

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const Vector &position, const Text &text_);

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  void fill(const Char &fillChar=Space);

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  Rectangle line(
    const Line &line,
    u1 strength=1,
    u1 dash=0,
    bool roundedCorners=false
  );

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  vector<Rectangle> box(const Box &box);

  ///
  /// Get window size
  ///
  /// @return     Window size
  Vector size() const;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area);

  ///
  /// Get window area
  ///
  /// @return     Window area
  Rectangle area() const;

  ///
  /// Get window text
  ///
  /// @return     Text
  const Text &text() const {
    return text_;
  }

  Text text_;
};

} // namespace
