#pragma once

#include <xxh64.hpp>

#include "basic.hh"

namespace jwezel {

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLASS_ID(NAME) \
static constexpr u8 id_{xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED)}; \
static constexpr const char *classname_{#NAME}

struct Event
{
  Event() = default;

  Event(const Event &) = default;

  Event(Event &&) = default;

  auto operator=(const Event &) -> Event & = delete;

  auto operator=(Event &&) -> Event & = delete;

  inline virtual ~Event() = default;

  [[nodiscard]] inline virtual auto id() const -> u8 {return id_;}

  [[nodiscard]] inline virtual auto classname() const -> const char * {return classname_;}

  private:
  CLASS_ID(Event);
};

} // namespace jwezel
