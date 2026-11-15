#pragma once

#include <functional>
#include <term/event.hh>
#include <term/keyboard.hh>
#include <term/surface.hh>


namespace jwezel::ui {

struct Container;
struct Root;

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

  ///
  /// Destroy the element
  ///
  /// The element leaves its parent, which therefore has to outlive it.
  virtual ~Element();

  ///
  /// Get the root of the element tree this element is in
  ///
  /// @return     The root, which is the (framed) window or the desktop the UI is shown in
  [[nodiscard]] virtual auto root() /*NOLINT(misc-no-recursion)*/-> Root *;

  ///
  /// Move element to a new area
  ///
  /// @param[in]  area  The new area
  virtual void move(const Rectangle &area) = 0;

  ///
  /// Get the area of the element
  ///
  /// All areas of the ui layer (constructor areas, layouts, `move`, `bounds`)
  /// are relative to the window; only the window itself is positioned on the
  /// display.
  ///
  /// @return     The area relative to the window
  [[nodiscard]] virtual auto bounds() -> Rectangle = 0;

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
