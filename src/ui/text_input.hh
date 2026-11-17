#pragma once

#include "text_buffer.hh"
#include "ui_event.hh"
#include "widget.hh"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace jwezel::ui {

///
/// Base class for text input widgets
///
/// Provides the editing engine: a content buffer, the cursor, key handling
/// (characters, Backspace, Delete, arrows, Home/End, Enter) and scrolling.
/// It renders the visible content and places the terminal's hardware cursor
/// (the caret) at the editing position while it has the focus. The whole
/// widget gets the colors of the role `input`, or `input.focus` while it gets
/// the keyboard input, so the field can be seen where it is empty. There are no
/// glyphs that could show which field has the focus, so by default the role
/// `input.focus` tints the background.
/// Subclasses supply the policy: LineEdit accepts on Enter and scrolls one
/// row; TextArea inserts a newline on Enter and scrolls vertically, and Page Up and
/// Page Down move the cursor by the lines of a page.
///
/// The text is kept in a `TextBuffer`, apart from the display: the widget edits the
/// buffer, and draws what is of it in the area. Nothing is read from the display.
///
/// An entry was accepted: Enter was pressed in a widget where Enter does not start a new line
struct AcceptEvent: UiEvent {
  AcceptEvent(const Cause &cause, std::string text):
  UiEvent{cause},
  text_{std::move(text)}
  {}

  ///
  /// The text, in UTF-8, as it was when it was accepted
  [[nodiscard]] auto text() const -> const std::string & {return text_;}

  private:
  std::string text_;

  CLASS_ID(AcceptEvent);
};

struct TextInput: Widget {
  ///
  /// Create TextInput with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The initial text, in UTF-8; the cursor is behind it
  /// @param      background  The background
  explicit TextInput(Container &parent, const Rectangle &area, std::string_view text = {}, const Char &background = Space);

  ///
  /// Create TextInput with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The initial text, in UTF-8; the cursor is behind it
  /// @param      background  The background
  explicit TextInput(Container &parent, std::string_view text = {}, const Char &background = Space);

  TextInput(const TextInput &) = delete;

  TextInput(TextInput &&) = delete;

  auto operator=(const TextInput &) -> TextInput & = delete;

  auto operator=(TextInput &&) -> TextInput & = delete;

  ~TextInput() override = default;

  [[nodiscard]] auto caret() const -> std::optional<Vector> override;

  ///
  /// Get the text, in UTF-8, with a line feed between the lines
  ///
  /// The text is the document the widget edits; what is on the display is made from it.
  [[nodiscard]] auto text() const -> std::string {return buffer_.text();}

  ///
  /// Replace the text; the cursor is behind it, and the widget shows the beginning of the text or the end of it, where
  /// the cursor is
  ///
  /// @param[in]  text  The text, in UTF-8
  void text(std::string_view text);

  ///
  /// Get the lines of the text, as characters
  [[nodiscard]] auto lines() const -> const std::vector<std::u32string> & {return buffer_.lines();}

  [[nodiscard]] auto buffer() const -> const TextBuffer & {return buffer_;}

  ///
  /// Get the position of the cursor in the text: {column, line}
  [[nodiscard]] auto cursor() const -> const Vector & {return cursor_;}

  ///
  /// Get the position in the text of what is shown at the top left of the widget: {column, line}
  [[nodiscard]] auto scroll() const -> Vector {return Vector{scrollX_, scrollY_};}

  ///
  /// Register an accept handler (invoked on Enter when Enter is not inserted)
  ///
  /// If no handler returns true the Enter key is not used up, so it goes on to the actions of the widget and of
  /// the window: an entry without handlers lets the default action of its window happen.
  ///
  /// @param[in]  handler  The handler, which is given an `AcceptEvent`: `event.as<AcceptEvent>()`
  ///
  /// @return     Index of the handler
  auto onAccept(const Handler &handler) -> int;

  protected:
  void render() override;

  ///
  /// Whether Enter inserts a newline (TextArea) or accepts (LineEdit)
  [[nodiscard]] virtual auto acceptsEnter() const -> bool {return false;}

  private:
  void start();

  void registerHandlers();

  auto handleKey(const Event &event) -> bool;

  void insertChar(Unicode key);

  void insertNewline();

  void backspace();

  void del();

  void moveLeft();

  void moveRight();

  void moveUp();

  void moveDown();

  ///
  /// Move the cursor, and what is shown, by a page, which is as many lines as the widget has
  void movePage(int direction);

  auto accept(const Event &event) -> bool;

  void scrollIntoView();

  TextBuffer buffer_;
  Vector cursor_; // {column, line}
  Dim scrollX_ = 0;
  Dim scrollY_ = 0;
};

} // namespace jwezel::ui
