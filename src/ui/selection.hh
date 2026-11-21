#pragma once

#include "list_source.hh"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace jwezel::ui {

///
/// A set of selected list indices
///
/// The indices are kept sorted and without duplicates, in a `std::vector`. That is compact for a small index type
/// like `i1` and sparse for a source that has far more items than are selected, which is what a list box needs. The
/// operations are silent: they change the selection and leave it to the widget to tell anyone.
///
/// @tparam     Index  The index type
template<std::signed_integral Index = ListIndex>
class Selection {
  public:
  using value_type = Index;
  using const_iterator = typename std::vector<Index>::const_iterator;

  ///
  /// Get the number of selected items
  [[nodiscard]] auto size() const -> std::size_t {return indices_.size();}

  ///
  /// Whether nothing is selected
  [[nodiscard]] auto empty() const -> bool {return indices_.empty();}

  ///
  /// Get the n-th selected index, in ascending order
  [[nodiscard]] auto operator[](std::size_t position) const -> Index {return indices_[position];}

  ///
  /// Whether an index is selected
  [[nodiscard]] auto contains(Index index) const -> bool {
    const auto found{std::lower_bound(indices_.begin(), indices_.end(), index)};
    return found != indices_.end() and *found == index;
  }

  [[nodiscard]] auto begin() const -> const_iterator {return indices_.begin();}

  [[nodiscard]] auto end() const -> const_iterator {return indices_.end();}

  ///
  /// Select an index
  ///
  /// @return     Whether the index was not selected before
  auto add(Index index) -> bool {
    const auto found{std::lower_bound(indices_.begin(), indices_.end(), index)};
    if (found != indices_.end() and *found == index) {
      return false;
    }
    indices_.insert(found, index);
    return true;
  }

  ///
  /// Deselect an index
  ///
  /// @return     Whether the index was selected before
  auto remove(Index index) -> bool {
    const auto found{std::lower_bound(indices_.begin(), indices_.end(), index)};
    if (found == indices_.end() or *found != index) {
      return false;
    }
    indices_.erase(found);
    return true;
  }

  ///
  /// Select an index if it is not selected, and deselect it otherwise
  ///
  /// @return     Whether the index is selected now
  auto toggle(Index index) -> bool {
    if (remove(index)) {
      return false;
    }
    add(index);
    return true;
  }

  ///
  /// Deselect everything
  void clear() {indices_.clear();}

  ///
  /// Deselect every index that is not below a size
  ///
  /// This is what a list does when its source shrinks: the items that are gone cannot stay selected.
  ///
  /// @param[in]  size  The new number of items
  void truncate(Index size) {
    if (size <= 0) {
      indices_.clear();
      return;
    }
    const auto first{std::lower_bound(indices_.begin(), indices_.end(), size)};
    indices_.erase(first, indices_.end());
  }

  private:
  std::vector<Index> indices_;
};

} // namespace jwezel::ui
