#pragma once

#include <memory>
#include <util/xxh64.hpp>

#include <util/basic.hh>

namespace jwezel {

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_ID(NAME) \
public: \
static const auto type_ = xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED); \
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

  [[nodiscard]] virtual auto type() const -> u8 {
    return event_->type();
  }

  [[nodiscard]] virtual auto typeName() const -> const char * {
    return event_->typeName();
  }

  private:
  std::unique_ptr<BaseEvent> event_;
};

} // namespace jwezel
