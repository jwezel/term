#pragma once

#include "update.hh"

namespace jwezel {

struct Device {

  virtual ~Device() = default;

  virtual void update(const jwezel::Updates &updates) = 0;
};

} // namespace jwezel
