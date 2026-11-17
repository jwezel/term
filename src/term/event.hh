#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <util/xxh64.hpp>

#include <util/basic.hh>

namespace jwezel {

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_ID(NAME) \
public: \
constexpr static const auto type_ = xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED); \
constexpr static const auto typeName_ = #NAME; \
[[nodiscard]] virtual auto type() const -> u8 { \
  return type_; \
} \
[[nodiscard]] virtual auto typeName() const -> const char * { \
  return typeName_; \
}

struct BaseEvent
{
  BaseEvent() = default;

  virtual ~BaseEvent() = default;

  BaseEvent(const BaseEvent &) = default;

  BaseEvent(BaseEvent &&) = default;

  auto operator=(const BaseEvent &) -> BaseEvent & = delete;

  auto operator=(BaseEvent &&) -> BaseEvent & = delete;

  CLASS_ID(Event);
};

struct Event {
  explicit Event(BaseEvent *event):
  event_(event)
  {}

  Event() = default;

  virtual ~Event() = default;

  Event(const Event &) = delete;

  Event(Event &&) = default;

  auto operator =(const Event &) -> Event & = delete;

  auto operator =(Event &&) -> Event & = default;

  auto operator()() const -> BaseEvent * {
    return event_.get();
  }

  ///
  /// Get the event as a type of event, if it is one
  ///
  /// @tparam     E     The event class; an event of a class derived from it matches as well
  ///
  /// @return     The event, or nullptr if it is another kind of event
  template<class E>
  [[nodiscard]] auto get() const -> const E * {
    return dynamic_cast<const E *>(event_.get());
  }

  ///
  /// Whether the event is of a type; see `get`
  template<class E>
  [[nodiscard]] auto is() const -> bool {
    return get<E>() != nullptr;
  }

  ///
  /// Get the event as a type of event that it is expected to be
  ///
  /// @tparam     E     The event class; an event of a class derived from it matches as well
  ///
  /// @throws     std::logic_error if it is another kind of event: a handler was given an event that it has not
  ///             been made for
  template<class E>
  [[nodiscard]] auto as() const -> const E & {
    if (const auto *result{get<E>()}; result) {
      return *result;
    }
    throw std::logic_error(
      std::string{"The event is "} + typeName() + ", not " + E::typeName_
    );
  }

  ///
  /// Get the kind of event, which is 0 if there is no event
  [[nodiscard]] virtual auto type() const -> u8 {
    return event_? event_->type(): 0;
  }

  ///
  /// Get the name of the kind of event, which is "empty" if there is no event
  [[nodiscard]] virtual auto typeName() const -> const char * {
    return event_? event_->typeName(): "empty";
  }

  private:
  std::unique_ptr<BaseEvent> event_;
};

} // namespace jwezel
