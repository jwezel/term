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

  ///
  /// Destroy BaseWindow
  virtual ~BaseWindow() {}

  /// String representation of WindowObj
  virtual operator string() const;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  virtual void write(const Vector &position, const string_view &str);

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  virtual void write(const Vector &position, const Text &text_);

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  virtual void fill(const Char &fillChar=Space);

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  virtual Rectangle line(
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
  virtual vector<Rectangle> box(const Box &box);

  ///
  /// Get window size
  ///
  /// @return     Window size
  virtual Vector size() const;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  virtual void move(const Rectangle &area);

  ///
  /// Get window area
  ///
  /// @return     Window area
  virtual Rectangle area() const;

  ///
  /// Get window text
  ///
  /// @return     Text
  virtual const Text &text() const;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  virtual Text text(const Rectangle &area) const;

  Vector position;
  Char background;
  vector<Rectangle> fragments;
};

struct Backdrop: public BaseWindow {

  ///
  /// Constructor
  ///
  /// @param[in]  id    The identifier
  /// @param[in]  area  The area
  Backdrop(const Char &background=Space);

  ///
  /// Destroy Backdrop
  ~Backdrop() override {}

  ///
  /// Get backdrop area
  ///
  /// @return     RectangleMax
  Rectangle area() const override;

  ///
  /// Get backdrop size
  ///
  /// @return     VectorMax
  Vector size() const override;

  ///
  /// Get window text
  ///
  /// @return     Text
  const Text &text() const override;

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  Text text(const Rectangle &area) const override;
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

  ///
  /// Destroy Window
   ~Window() override {}

  /// String representation of Window
  operator string() const override;

  ///
  /// Write string to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  void write(const Vector &position, const string_view &str) override;

  ///
  /// Write text to window
  ///
  /// @param[in]  position  The position
  /// @param[in]  text      The text
  void write(const Vector &position, const Text &text_) override;

  ///
  /// Fill window with Char
  ///
  /// @param[in]  fillChar  The fill character
  void fill(const Char &fillChar=Space) override;

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
  ) override;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  vector<Rectangle> box(const Box &box) override;

  ///
  /// Get window size
  ///
  /// @return     Window size
  Vector size() const override;

  ///
  /// Move window
  ///
  /// @param[in]  area  The area
  void move(const Rectangle &area) override;

  ///
  /// Get window area
  ///
  /// @return     Window area
  Rectangle area() const override;

  ///
  /// Get window text
  ///
  /// @return     Text
  const Text &text() const override {
    return text_;
  }

  ///
  /// Get area of window text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  Text text(const Rectangle &area) const override {
    return text_[area];
  }

  Text text_;
};

} // namespace
