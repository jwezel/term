#pragma once

#include <term/geometry.hh>
#include <term/text.hh>

#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace jwezel::ui {

///
/// Index type of a list source
///
/// A list source chooses its own index width through the return type of its `size()`: a small list can use `i1`, a
/// database-backed one `i8`. `ListIndex` is the default, wide enough for anything a terminal can show.
using ListIndex = i8;

///
/// The index type a source uses, which is the type its `size()` returns
///
/// A source with `size()` returning `i2` is a source of `i2` indices; `ListView` and `ListSource` are then
/// instantiated with `i2` as well.
template<class S>
using SourceIndex = std::remove_cvref_t<decltype(std::declval<const S &>().size())>;

///
/// Whether a type is a list source
///
/// A source has a `size()` that returns a signed integer, and either `item(index)` or `operator[](index)` that
/// returns something convertible to `Text`. `item` wins when a source has both.
template<class S>
concept ListSourceLike =
  std::signed_integral<SourceIndex<S>> &&
  (requires(const S &source, SourceIndex<S> index) {
     { source.item(index) } -> std::convertible_to<Text>;
   } ||
   requires(const S &source, SourceIndex<S> index) {
     { source[index] } -> std::convertible_to<Text>;
   });

///
/// Read an item from a duck-typed source
///
/// Uses `item(index)` if the source has it, and `operator[](index)` otherwise.
template<class S>
auto sourceItem(const S &source, SourceIndex<S> index) -> Text {
  if constexpr (requires { { source.item(index) } -> std::convertible_to<Text>; }) {
    return source.item(index);
  } else {
    return source[index];
  }
}

///
/// Convert a count to a list index, checking that it fits the chosen index type
///
/// @throws     std::range_error if the count does not fit
template<std::signed_integral Index>
auto toListIndex(std::size_t value) -> Index {
  if (value > static_cast<std::size_t>(std::numeric_limits<Index>::max())) {
    throw std::range_error(std::format("List index {} is out of range", value));
  }
  return static_cast<Index>(value);
}

///
/// A list source as a value: the two functions a list needs, owned by the widget
///
/// This is the layer that `ListView` stores. It can wrap a duck-typed source, a `ListSourceInterface`, a vector, or
/// two lambdas. The adapters that take a reference keep a weak reference: the source must outlive the list.
///
/// @tparam     Index  The index type
template<std::signed_integral Index = ListIndex>
class ListSource {
  public:
  /// Function that reports the number of items.
  using SizeFunction = std::function<Index()>;

  /// Function that returns one item.
  using ItemFunction = std::function<Text(Index)>;

  ListSource() = default;

  ///
  /// Create ListSource
  ///
  /// @param[in]  size  The function that reports the number of items
  /// @param[in]  item  The function that returns an item
  ListSource(SizeFunction size, ItemFunction item):
  size_{std::move(size)},
  item_{std::move(item)}
  {}

  ///
  /// Get the number of items
  [[nodiscard]] auto size() const -> Index {
    return size_? size_(): Index{0};
  }

  ///
  /// Get an item
  ///
  /// The index must be in `[0, size())`; the source defines what happens otherwise.
  [[nodiscard]] auto item(Index index) const -> Text {
    return item_? item_(index): Text{};
  }

  ///
  /// Get an item; the same as `item`
  [[nodiscard]] auto operator[](Index index) const -> Text {
    return item(index);
  }

  private:
  SizeFunction size_;
  ItemFunction item_;
};

///
/// A list source that is an object with virtual functions, for run-time polymorphism
///
/// Adapt it with `listSource(source)`, which keeps a weak reference; the source must outlive the list.
///
/// @tparam     Index  The index type
template<std::signed_integral Index = ListIndex>
struct ListSourceInterface {
  virtual ~ListSourceInterface() = default;

  [[nodiscard]] virtual auto size() const -> Index = 0;

  [[nodiscard]] virtual auto item(Index index) const -> Text = 0;
};

///
/// Adapt a duck-typed source
///
/// The source must outlive the list. Its `size()` return type selects the index type.
///
/// @param[in]  source  The source
template<class S>
  requires ListSourceLike<S>
auto listSource(const S &source) -> ListSource<SourceIndex<S>> {
  using Index = SourceIndex<S>;
  return ListSource<Index>{
    [&source] { return source.size(); },
    [&source](Index index) { return sourceItem(source, index); }
  };
}

///
/// Make a non-owning source view of a vector of items
///
/// The vector must outlive the list. The default index is `ListIndex`; pass another index type explicitly for a
/// smaller list, as in `listSource<i2>(items)`.
///
/// @param[in]  items  The items
template<std::signed_integral Index = ListIndex>
auto listSource(const std::vector<Text> &items) -> ListSource<Index> {
  return ListSource<Index>{
    [&items] { return toListIndex<Index>(items.size()); },
    [&items](Index index) { return items[static_cast<std::size_t>(index)]; }
  };
}

///
/// Make a source that owns its items
///
/// @param[in]  items  The items
template<std::signed_integral Index = ListIndex>
auto ownedListSource(std::vector<Text> items) -> ListSource<Index> {
  auto owned{std::make_shared<const std::vector<Text>>(std::move(items))};
  return ListSource<Index>{
    [owned] { return toListIndex<Index>(owned->size()); },
    [owned](Index index) { return (*owned)[static_cast<std::size_t>(index)]; }
  };
}

///
/// Make a source from a size function and an item function
///
/// The index type is the result type of the size function.
///
/// @param[in]  size  The size function
/// @param[in]  item  The item function
template<class Size, class Item>
  requires std::signed_integral<std::invoke_result_t<Size>> &&
           requires(Item item, std::invoke_result_t<Size> index) {
             { item(index) } -> std::convertible_to<Text>;
           }
auto listSource(Size size, Item item) -> ListSource<std::invoke_result_t<Size>> {
  using Index = std::invoke_result_t<Size>;
  return ListSource<Index>{
    std::function<Index()>{std::move(size)},
    std::function<Text(Index)>{std::move(item)}
  };
}

} // namespace jwezel::ui
