#pragma once

#include "update.hh"

namespace jwezel {

///
/// Interface for a device
struct Device {

  Device() = default;

  Device(const Device &) = default;

  Device(Device &&) = delete;

  auto operator=(Device &&) -> Device & = delete;

  ///
  /// Assignment operator
  ///
  /// @param[in]  source  The source
  ///
  /// @return     The result of the assignment
  auto operator=(const Device &source) -> Device & = default;


  ///
  /// Destroy the device
  virtual ~Device() = default;

  ///
  /// Updates the given updates.
  ///
  /// Write the updates to the device
  ///
  /// @param[in]  updates  The updates
  virtual void update(const jwezel::Updates &updates) = 0;
};

} // namespace jwezel
