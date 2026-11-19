#pragma once

#include "text_buffer.hh"
#include "ui_event.hh"
#include "widget.hh"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace jwezel::ui {

/// Notification that a single-line text entry accepted its current contents with Enter.
struct AcceptEvent: UiEvent {
  /// Create an acceptance event with a snapshot of the accepted contents.
  /// @param cause Input event that caused acceptance.
  /// @param text Accepted text in UTF-8.
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

/// Base editing engine for single-line and multi-line text widgets.
///
/// The editable document lives in `TextBuffer`, independently of rendered cells. `LineEdit` accepts on Enter and
/// scrolls horizontally; `TextArea` inserts a newline, scrolls vertically, and supports Page Up/Down. The widget
/// manages cursor movement, editing keys, viewport scrolling, rendering, and the focused hardware caret.
struct TextInput: Widget {
  ///
  /// Create TextInput with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The initial text, in UTF-8; the cursor is behind it
  /// @param      background  The background
  TextInput(Container &parent, const Rectangle &area, std::string_view text = {}, const Char &background = InheritBackground);

  ///
  /// Create TextInput with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The initial text, in UTF-8; the cursor is behind it
  /// @param      background  The background
  explicit TextInput(Container &parent, std::string_view text = {}, const Char &background = InheritBackground);

  TextInput(const TextInput &) = delete;

  TextInput(TextInput &&) = delete;

  auto operator=(const TextInput &) -> TextInput & = delete;

  auto operator=(TextInput &&) -> TextInput & = delete;

  ~TextInput() override = default;

  [[nodiscard]] auto caret() const -> std::optional<Vector> override;

  ///
  /// Get the text, in UTF-8, with a line feed between the lines
  ///
  /// The text is the document the widget edits; what is on the display is made from it. `text(area)` is what the
  /// widget shows, which is that, laid out.
  [[nodiscard]] auto contents() const -> std::string {return buffer_.text();}

  ///
  /// Replace the text; the cursor is behind it, and the widget shows the beginning of the text or the end of it, where
  /// the cursor is
  ///
  /// @param[in]  text  The text, in UTF-8
  void contents(std::string_view text);

  ///
  /// Get the lines of the text, as characters
  [[nodiscard]] auto lines() const -> const std::vector<std::u32string> & {return buffer_.lines();}

  /// Get a read-only view of the underlying editable document.
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

  ///
  /// The size of the text, with room for the caret behind it; a line edit is one row high
  [[nodiscard]] auto preferredSize() -> Vector override;

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
