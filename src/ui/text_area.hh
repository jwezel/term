#pragma once

#include "text_input.hh"

namespace jwezel::ui {

///
/// Multi-line text input
///
/// Enter inserts a newline; the content scrolls vertically.
struct TextArea: TextInput {
  using TextInput::TextInput;

  TextArea(const TextArea &) = delete;

  TextArea(TextArea &&) = delete;

  auto operator=(const TextArea &) -> TextArea & = delete;

  auto operator=(TextArea &&) -> TextArea & = delete;

  ~TextArea() override = default;

  protected:
  [[nodiscard]] auto acceptsEnter() const -> bool override {return true;}
};

} // namespace jwezel::ui
