#pragma once

#include <xxh64.hpp>

#include "basic.hh"

namespace jwezel {

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_ID(NAME) \
static u8 id() {return xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED);}; \
virtual u8 vid() const {return id();} \
static const char *classname() {return #NAME;} \
virtual const char *vclassname() const {return classname();}

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
