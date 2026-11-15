#pragma once

#include "element.hh"

#include <unordered_set>

namespace jwezel::ui {

using std::unordered_set;

struct Container: ui::Element {

  explicit Container(Container *parent=nullptr);

  virtual auto add(Element &child) -> Container & {
    children_.insert(&child);
    return *this;
  }

  ///
  /// Remove a child; called when the child is destroyed
  virtual void remove(Element &child) {
    children_.erase(&child);
  }

  auto children() const -> const unordered_set<Element *> & {return children_;}

  private:
  unordered_set<Element *> children_;
};
} // namespace jwezel::ui
