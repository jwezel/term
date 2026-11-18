#pragma once

#include "widget.hh"

#include <term/text.hh>

namespace jwezel::ui {

///
/// Widget that displays a text
///
/// The text is rendered anchored at the top-left corner of the widget and
/// clipped to the widget area. When the widget is moved (e.g. positioned by
/// a Box), the text is rendered again into the new area. The colors of the
/// role `label` are applied to the whole widget, and what the text says for
/// itself comes on top of that. A label does nothing, so it does not get the focus.
struct Label: Widget {

  ///
  /// Create Label with an explicit area
  ///
  /// @param      parent      The parent container
  /// @param      area        The area
  /// @param      text        The text to display
  /// @param      background  The background
  Label(Container &parent, const Rectangle &area, const Text &text = {}, const Char &background = Space);

  ///
  /// Create Label with its area assigned by the parent layout (e.g. a Box)
  ///
  /// @param      parent      The parent container
  /// @param      text        The text to display
  /// @param      background  The background
  explicit Label(Container &parent, const Text &text = {}, const Char &background = Space);

  Label(const Label &) = delete;

  Label(Label &&) = delete;

  auto operator=(const Label &) -> Label & = delete;

  auto operator=(Label &&) -> Label & = delete;

  ~Label() override = default;

  [[nodiscard]] auto content() const -> const Text & {return text_;}

  ///
  /// Change the text, which is displayed in the same area
  ///
  /// @param[in]  text  The text
  void content(const Text &text);

  protected:
  void render() override;

  Text text_;
};

} // namespace jwezel::ui
