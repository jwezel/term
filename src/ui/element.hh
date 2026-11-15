#pragma once

#include <functional>
#include <term/event.hh>
#include <term/keyboard.hh>
#include <term/surface.hh>


namespace jwezel::ui {

struct Element
{

  ///
  /// Create Element
  ///
  /// @param      parent  The parent, if any
  explicit Element(struct Container *parent=nullptr);

  Element(const Element &) = delete;

  Element(Element &&) = delete;

  auto operator=(const Element &) -> Element & = delete;

  auto operator=(Element &&) -> Element & = delete;

  virtual ~Element();

  [[nodiscard]] virtual auto window() /*NOLINT(misc-no-recursion)*/-> struct Window *;

  ///
  /// Move element to a new area
  ///
  /// @param[in]  area  The new area
  virtual void move(const Rectangle &area) = 0;

  auto parent() {return parent_;}

  auto event(const jwezel::Event &event) -> bool;

  auto mouseMoveEvent(const Event &event) -> bool;

  auto mouseButtonEvent(const Event &event) -> bool;

  auto keyEvent(const Event &event) -> bool;

  auto onMouseMove(const std::function<bool(const Element &element, const Event &)>& handler) -> int;

  auto onMouseButton(const std::function<bool(const Element &element, const Event &)>& handler) -> int;

  auto onKey(const std::function<bool(const Element &element, const Event &)>& handler) -> int;

  private:
  struct Container *parent_;
  vector<pair<decltype(BaseEvent::type_), std::function<bool(const Element &element, const Event &)>>> eventHandlers_;
};

} // namespace jwezel::ui
