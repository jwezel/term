#pragma once

#include <functional>
#include <taitank_node.h>
#include <term/event.hh>
#include <term/keyboard.hh>
#include <term/surface.hh>


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

  auto event(const jwezel::Event &event) -> bool;

  auto mouseMoveEvent(const Event &event) -> bool;

  auto mouseButtonEvent(const Event &event) -> bool;

  auto keyEvent(const Event &event) -> bool;

  auto onMouseMove(const std::function<bool(Element &element, const Event &)>& handler) -> int;

  auto onMouseButton(const std::function<bool(Element &element, const Event &)>& handler) -> int;

  auto onKey(const std::function<bool(Element &element, const Event &)>& handler) -> int;

  private:
  struct Container *parent_;
  TaitankNodeRef node_;
  vector<pair<decltype(BaseEvent::type_), std::function<bool(Element &element, const Event &)>>> eventHandlers_;
};

} // namespace jwezel::ui
