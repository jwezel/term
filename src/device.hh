#pragma once

#include "update.hh"

namespace jwezel {

struct Device {

  Device() = default;

  Device(const Device &) = default;

  Device(Device &&) = delete;

  auto operator=(const Device &) -> Device & = default;

  auto operator=(Device &&) -> Device & = delete;

  virtual ~Device() = default;

  virtual void update(const jwezel::Updates &updates) = 0;
};

} // namespace jwezel
