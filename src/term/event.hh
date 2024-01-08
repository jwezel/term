#pragma once

#include <util/xxh64.hpp>

#include <util/basic.hh>

namespace jwezel {

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_ID(NAME) \
public: \
static auto id() -> u8 {return xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED);}; \
virtual auto vid() const -> u8 {return id();} \
static auto classname() -> const char * {return #NAME;} \
virtual auto vclassname() const -> const char * {return classname();}

struct Event
{
  Event() = default;

  Event(const Event &) = default;

  Event(Event &&) = default;

  auto operator=(const Event &) -> Event & = delete;

  auto operator=(Event &&) -> Event & = delete;

  inline virtual ~Event() = default;

  CLASS_ID(Event);
};


} // namespace jwezel
