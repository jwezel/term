#pragma once

#include "element.hh"

#include <unordered_set>

namespace jwezel::ui {

using std::unordered_set;

/// An element that tracks non-owning references to its children.
///
/// A container never owns or deletes its children. Each child must be destroyed before the container; the child's
/// destructor unregisters it from its parent.
struct Container: ui::Element {

  /// Create a container, optionally nested under another container.
  /// @param parent The parent container, or null for a detached container.
  explicit Container(Container *parent=nullptr);

  /// Register a child without taking ownership.
  virtual auto add(Element &child) -> Container & {
    children_.insert(&child);
    return *this;
  }

  ///
  /// Remove a child; called when the child is destroyed
  virtual void remove(Element &child) {
    children_.erase(&child);
  }

  ///
  /// A child may have a different preferred size now; hand this up to the container that lays this one out
  void layoutChanged() override {
    if (auto *container{parent()}; container) {
      container->layoutChanged();
    }
  }

  /// Get the non-owning references to the children currently in this container.
  [[nodiscard]] auto children() const -> const unordered_set<Element *> & {return children_;}

  private:
  unordered_set<Element *> children_;
};
} // namespace jwezel::ui
