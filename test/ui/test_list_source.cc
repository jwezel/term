#include <ui/list_source.hh>
#include <ui/selection.hh>

#include <doctest/doctest.h>

#include <string>
#include <vector>

using jwezel::Text;
using jwezel::i1;
using jwezel::i2;
using jwezel::ui::ListSource;
using jwezel::ui::ListSourceInterface;
using jwezel::ui::ListSourceLike;
using jwezel::ui::Selection;
using jwezel::ui::SourceIndex;
using jwezel::ui::listSource;
using jwezel::ui::ownedListSource;
using jwezel::ui::toListIndex;

namespace {

struct BracketSource {
  auto size() const -> i1 {return 2;}
  auto operator[](i1 index) const -> Text {return index == 0? Text{"first"}: Text{"second"};}
};

struct ItemSource {
  auto size() const -> i2 {return 1;}
  auto item(i2) const -> Text {return Text{"item"};}
};

struct BothSource {
  auto size() const -> i1 {return 1;}
  auto item(i1) const -> Text {return Text{"item wins"};}
  auto operator[](i1) const -> Text {return Text{"bracket loses"};}
};

struct VirtualSource: ListSourceInterface<i1> {
  auto size() const -> i1 override {return 1;}
  auto item(i1) const -> Text override {return Text{"virtual"};}
};

} // namespace

static_assert(ListSourceLike<BracketSource>);
static_assert(ListSourceLike<ItemSource>);
static_assert(ListSourceLike<BothSource>);
static_assert(not ListSourceLike<std::vector<Text>>);
static_assert(std::same_as<SourceIndex<BracketSource>, i1>);
static_assert(std::same_as<SourceIndex<ItemSource>, i2>);

TEST_CASE("List source adapters") {
  SUBCASE("A source can use operator[] and its size chooses the index type") {
    const BracketSource source;
    auto value{listSource(source)};
    static_assert(std::same_as<decltype(value), ListSource<i1>>);
    CHECK_EQ(value.size(), 2);
    CHECK_EQ(std::string(value[0]), "first\n");
    CHECK_EQ(std::string(value.item(1)), "second\n");
  }
  SUBCASE("A source can use item() and item() wins") {
    const ItemSource itemSource;
    CHECK_EQ(std::string(listSource(itemSource)[0]), "item\n");
    const BothSource both;
    CHECK_EQ(std::string(listSource(both)[0]), "item wins\n");
  }
  SUBCASE("The interface adapter keeps a weak reference") {
    const VirtualSource source;
    const auto adapted{listSource(source)};
    CHECK_EQ(adapted.size(), 1);
    CHECK_EQ(std::string(adapted[0]), "virtual\n");
  }
  SUBCASE("A vector view is non-owning and can choose a narrow index") {
    const std::vector<Text> items{Text{"one"}, Text{"two"}};
    const auto adapted{listSource<i1>(items)};
    CHECK_EQ(adapted.size(), 2);
    CHECK_EQ(std::string(adapted[1]), "two\n");
    CHECK_THROWS_AS(toListIndex<i1>(128), std::range_error);
  }
  SUBCASE("An owning vector source outlives the input") {
    const auto makeSource{[] {
      std::vector<Text> items{Text{"kept"}};
      return ownedListSource<i1>(std::move(items));
    }};
    const auto adapted{makeSource()};
    CHECK_EQ(adapted.size(), 1);
    CHECK_EQ(std::string(adapted[0]), "kept\n");
  }
  SUBCASE("Two functions make a source") {
    const auto adapted{listSource([]() -> i1 {return 1;}, [](i1) {return Text{"lambda"};})};
    static_assert(std::same_as<decltype(adapted), const ListSource<i1>>);
    CHECK_EQ(std::string(adapted[0]), "lambda\n");
  }
}

TEST_CASE("List selection") {
  Selection<i1> selection;
  CHECK(selection.empty());
  CHECK(selection.add(3));
  CHECK(selection.add(1));
  CHECK_FALSE(selection.add(3));
  CHECK_EQ(selection.size(), 2);
  CHECK_EQ(selection[0], 1);
  CHECK_EQ(selection[1], 3);
  CHECK(selection.contains(1));
  CHECK(selection.toggle(1) == false);
  CHECK_FALSE(selection.contains(1));
  CHECK(selection.toggle(2));
  CHECK(selection.remove(3));
  CHECK_FALSE(selection.remove(3));
  selection.add(5);
  selection.truncate(5);
  REQUIRE_EQ(selection.size(), 1);
  CHECK_EQ(selection[0], 2);
  selection.clear();
  CHECK(selection.empty());
}
