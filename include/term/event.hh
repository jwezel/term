#pragma once

#include <xxh64.hpp>

#include "basic.hh"

using namespace jwezel;

constexpr u8 EVENT_ID_SEED = xxh64::hash("EVENT_ID_SEED", sizeof "EVENT_ID_SEED", 0);

#define CLASS_ID(NAME) \
static constexpr u8 id_{xxh64::hash(#NAME, sizeof #NAME, EVENT_ID_SEED)}; \
static constexpr const char *class_{#NAME}

struct Event
{
  inline virtual ~Event() = default;

  CLASS_ID(Event);
};
