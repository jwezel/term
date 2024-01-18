#pragma once

#include <term/surface.hh>

#include "taitank_node.h"

namespace jwezel::ui {

using
  taitank::TaitankNodeRef;

struct Element
{

  ///
  /// Create Element
  ///
  /// @param      parent  The parent
  explicit Element(struct Container *parent=0);

  Element(const Element &) = delete;

  Element(Element &&) = delete;

  auto operator=(const Element &) -> Element & = delete;

  auto operator=(Element &&) -> Element & = delete;

  virtual ~Element();

  [[nodiscard]] virtual auto window() /*NOLINT(misc-no-recursion)*/-> struct Window *;

  auto parent() {return parent_;}

  private:
  struct Container *parent_;
  TaitankNodeRef node_;
};

} // namespace jwezel::ui
