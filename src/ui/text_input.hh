#pragma once

#include "widget.hh"

#include <functional>
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
/// row; TextArea inserts a newline on Enter and scrolls vertically.
struct TextInput: Widget {
  using Handler = std::function<bool(const ui::Element &element, const Event &event)>;

  ///
  /// Create TextInput with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      content     The initial content
  /// @param      background  The background
  explicit TextInput(Container &parent, const Rectangle &area, const Text &content = {}, const Char &background = Space);

  ///
  /// Create TextInput with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      content     The initial content
  /// @param      background  The background
  explicit TextInput(Container &parent, const Text &content = {}, const Char &background = Space);

  TextInput(const TextInput &) = delete;

  TextInput(TextInput &&) = delete;

  auto operator=(const TextInput &) -> TextInput & = delete;

  auto operator=(TextInput &&) -> TextInput & = delete;

  ~TextInput() override = default;

  [[nodiscard]] auto caret() const -> std::optional<Vector> override;

  [[nodiscard]] auto content() const -> const Text & {return content_;}

  ///
  /// Register an accept handler (invoked on Enter when Enter is not inserted)
  ///
  /// If no handler returns true the Enter key is not used up, so it goes on to the actions of the widget and of
  /// the window: an entry without handlers lets the default action of its window happen.
  ///
  /// @param[in]  handler  The handler
  ///
  /// @return     Index of the handler
  auto onAccept(const Handler &handler) -> int;

  protected:
  void render() override;

  ///
  /// Whether Enter inserts a newline (TextArea) or accepts (LineEdit)
  [[nodiscard]] virtual auto acceptsEnter() const -> bool {return false;}

  private:
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

  auto accept(const Event &event) -> bool;

  void scrollIntoView();

  [[nodiscard]] auto lineWidth(Dim line) const -> Dim;

  Text content_;
  Vector cursor_; // {column, line}
  Dim scrollX_ = 0;
  Dim scrollY_ = 0;
  std::vector<Handler> acceptHandlers_;
};

} // namespace jwezel::ui
