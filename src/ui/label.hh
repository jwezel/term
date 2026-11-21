#pragma once

#include "widget.hh"

#include <term/text.hh>

namespace jwezel::ui {

/// A passive text widget that does not take keyboard focus.
///
/// Content follows the widget's horizontal and vertical alignment settings. A label can name a non-owning buddy;
/// parallel boxes then use the buddy's preferred size along their shared row or column axis.
struct Label: Widget {

  ///
  /// Create Label with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The text to display
  /// @param      background  The background
  Label(Container &parent, const Rectangle &area, const Text &text = {}, const Char &background = InheritBackground);

  ///
  /// Create Label with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The text to display
  /// @param      background  The background
  explicit Label(Container &parent, const Text &text = {}, const Char &background = InheritBackground);

  Label(const Label &) = delete;

  Label(Label &&) = delete;

  auto operator=(const Label &) -> Label & = delete;

  auto operator=(Label &&) -> Label & = delete;

  ~Label() override;

  /// Get the text currently displayed by the label.
  [[nodiscard]] auto content() const -> const Text & {return text_;}

  ///
  /// Change the text, which is displayed in the same area
  ///
  /// @param[in]  text  The text
  void content(const Text &text);

  /// Get the widget this label describes, if any.
  [[nodiscard]] auto buddy() const -> Widget * {return buddy_;}

  /// Associate this label with its buddy without changing either widget's position.
  ///
  /// When the label and buddy are in separate boxes with the same orientation, the label's preferred size follows the
  /// buddy on that box's main axis. The buddy is a weak reference and must outlive the label.
  void buddy(Widget &widget);

  ///
  /// The size of the text, which is what a layout that sizes the label to its content uses
  [[nodiscard]] auto preferredSize() -> Vector override;

  protected:
  void render() override;

  Text text_;
  Widget *buddy_{nullptr}; // documented weak reference
};

} // namespace jwezel::ui
