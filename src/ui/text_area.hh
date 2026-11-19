#pragma once

#include "text_input.hh"

namespace jwezel::ui {

///
/// Multi-line text input
///
/// Enter inserts a newline; the content scrolls vertically.
struct TextArea: TextInput {
  TextArea(Container &parent, const Rectangle &area, std::string_view text = {}, const Char &background = InheritBackground):
  TextInput{parent, area, text, background}
  {
    layoutChanged(); // a text area prefers as many rows as it has lines, which Enter makes different
  }

  explicit TextArea(Container &parent, std::string_view text = {}, const Char &background = InheritBackground):
  TextInput{parent, text, background}
  {
    layoutChanged();
  }

  TextArea(const TextArea &) = delete;

  TextArea(TextArea &&) = delete;

  auto operator=(const TextArea &) -> TextArea & = delete;

  auto operator=(TextArea &&) -> TextArea & = delete;

  ~TextArea() override = default;

  protected:
  [[nodiscard]] auto acceptsEnter() const -> bool override {return true;}
};

} // namespace jwezel::ui
