#pragma once

#include "widget.hh"

#include <term/text.hh>

namespace jwezel::ui {

/// A passive text widget that does not take keyboard focus.
///
/// Text is anchored at the top-left corner and clipped to the assigned area. A label's preferred size is its text
/// content size.
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

  ~Label() override = default;

  /// Get the text currently displayed by the label.
  [[nodiscard]] auto content() const -> const Text & {return text_;}

  ///
  /// Change the text, which is displayed in the same area
  ///
  /// @param[in]  text  The text
  void content(const Text &text);

  ///
  /// The size of the text, which is what a layout that sizes the label to its content uses
  [[nodiscard]] auto preferredSize() -> Vector override {return text_.size();}

  protected:
  void render() override;

  Text text_;
};

} // namespace jwezel::ui
