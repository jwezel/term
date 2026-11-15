#pragma once

#include "text_input.hh"

namespace jwezel::ui {

///
/// Single-line text input
///
/// Enter invokes the accept handlers instead of inserting a newline; the
/// content scrolls horizontally around the cursor.
struct LineEdit: TextInput {
  using TextInput::TextInput;

  LineEdit(const LineEdit &) = delete;

  LineEdit(LineEdit &&) = delete;

  auto operator=(const LineEdit &) -> LineEdit & = delete;

  auto operator=(LineEdit &&) -> LineEdit & = delete;

  ~LineEdit() override = default;

  protected:
  [[nodiscard]] auto acceptsEnter() const -> bool override {return false;}
};

} // namespace jwezel::ui
