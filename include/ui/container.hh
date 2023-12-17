#pragma once

#include "ui/element.hh"

#include <memory>
#include <unordered_map>

namespace jwezel::ui {

using std::unordered_map, std::unique_ptr;

struct Container: virtual ui::Element {

  auto add(Element *child) -> Container & {
    children_.emplace(child, unique_ptr<Element>(child));
    return *this;
  }
  auto children() -> unordered_map<Element *, unique_ptr<Element>> & {return children_;}

  private:
  unordered_map<Element *, unique_ptr<Element>> children_;
};
} // namespace jwezel::ui
