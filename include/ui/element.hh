#pragma once

namespace jwezel::ui {

struct Element {
  Element(struct Container *parent=0): parent{parent} {}

  virtual ~Element() = default;

  struct Container *parent;
};

} // namespace
