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
  /// A handler of events; it is told whether it was `handled`, which for input events keeps the others from getting it
  using Handler = std::function<bool(const Element &element, const Event &event)>;

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

  ///
  /// The size the element would like to have when a layout sizes it to its content
  ///
  /// A box that lays out an element with `LayoutSize::fit` gives it this size along its main axis and uses the
  /// cross component of it when the element is aligned instead of stretched. The default is zero: an element
  /// without content of its own has no preference.
  ///
  /// @return     The preferred size
  [[nodiscard]] virtual auto preferredSize() -> Vector {return Vector{0, 0};}

  ///
  /// Tell the layout that the preferred size of the element may have changed
  ///
  /// Containers hand this up to the box that lays them out, which lays its children out again.
  virtual void layoutChanged();

  auto parent() {return parent_;}

  ///
  /// Whether the area of the element is left to its parent
  ///
  /// This is the case for elements created without an area (`RectangleDefault`): a box
  /// assigns them their slice, and a root lets them fill its content area, also when the
  /// window is resized.
  [[nodiscard]] auto layoutManaged() const -> bool {return layoutManaged_;}

  auto event(const jwezel::Event &event) -> bool;

  auto mouseMoveEvent(const Event &event) -> bool;

  auto mouseButtonEvent(const Event &event) -> bool;

  auto keyEvent(const Event &event) -> bool;

  auto onMouseMove(const Handler &handler) -> int;

  auto onMouseButton(const Handler &handler) -> int;

  auto onKey(const Handler &handler) -> int;

  ///
  /// Register a handler for a kind of events
  ///
  /// The handler is given events of exactly that class, as `Event`s; `event.as<E>()` gives the event as what it is.
  ///
  /// @tparam     E        The event class
  /// @param[in]  handler  The handler
  ///
  /// @return     Index of the handler
  template<class E>
  auto on(const Handler &handler) -> int {
    eventHandlers_.emplace_back(E::type_, handler);
    return static_cast<int>(eventHandlers_.size() - 1);
  }

  protected:
  ///
  /// Tell the handlers of a kind of event what happened
  ///
  /// Unlike input events, which stop with the handler that handles them, all the handlers of the event are called;
  /// those that are registered while it is emitted are not.
  ///
  /// @param[in]  event  The event
  ///
  /// @return     Whether any handler handled the event
  template<class E>
  auto emit(E event) -> bool {
    const Event wrapped{new E{std::move(event)}};
    std::vector<Handler> handlers;
    for (const auto &handler: eventHandlers_) {
      if (handler.first == E::type_) {
        handlers.push_back(handler.second);
      }
    }
    auto handled{false};
    for (const auto &handler: handlers) {
      handled = handler(*this, wrapped) or handled;
    }
    return handled;
  }

  void layoutManaged(bool managed) {layoutManaged_ = managed;}

  private:
  struct Container *parent_;
  bool layoutManaged_{false};
  vector<pair<decltype(BaseEvent::type_), Handler>> eventHandlers_;
};

} // namespace jwezel::ui
