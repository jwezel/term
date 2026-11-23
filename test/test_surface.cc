
#include <term/device.hh>
#include <term/element_device.hh>
#include <term/geometry.hh>
#include <util/string.hh>
#include <term/surface.hh>
#include <term/text.hh>
#include <term/update.hh>
#include <term/window.hh>

#include <algorithm>
#include <format>
#include <iterator>
#include <memory>
#include <random>
#include <ranges>
#include <string>
#include <vector>

#include <doctest/doctest.h>

using
  jwezel::Char,
  jwezel::Dim,
  jwezel::DimHigh,
  jwezel::Rectangle,
  jwezel::str,
  jwezel::string,
  jwezel::Text,
  jwezel::Update,
  jwezel::Updates,
  jwezel::Vector,
  jwezel::operator""_C;

using jwezel::Surface, jwezel::Window, jwezel::Backdrop;
using std::vector, std::copy, std::back_inserter;
using std::ranges::views::transform, std::ranges::views::join_with;

namespace doctest {
  template<> struct StringMaker<Text> {
    static auto convert(const Text& value) -> String {
      return value.repr().c_str();
    }
  };

  template<> struct StringMaker<Vector> {
    static auto convert(const Vector& value) -> String {
      return string(value).c_str();
    }
  };

  template<> struct StringMaker<Rectangle> {
    static auto convert(const Rectangle& value) -> String {
      return string(value).c_str();
    }
  };

  template<> struct StringMaker<Updates> {
    static auto convert(const Updates& value) -> String {
      static auto delimiter{string(", ")};
      string lstr;
      std::ranges::copy(value | transform(str<Update>) | join_with(delimiter), back_inserter(lstr));
      return format("[{}]", lstr).c_str();
    }
  };
} // namespace doctest

namespace jwezel {
  auto toString(const Surface::Fragment &f) -> doctest::String {
    return {string(f).c_str()};
  }
} // namespace jwezel

namespace jwezel {
  auto toString(const vector<Surface::Fragment> &f) -> doctest::String {
    static auto delimiter{string(", ")};
    string lstr;
    std::ranges::copy(f | transform(str<Surface::Fragment>) | join_with(delimiter), back_inserter(lstr));
    return format("[{}]", lstr).c_str();
  }
} // namespace jwezel

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)

struct Device: jwezel::Device {
  void update(const Updates &updates) override {
    copy(updates.begin(), updates.end(), back_inserter(updates_));
  }
  Updates updates_;
};

struct Terminal: Surface, jwezel::TerminalInterface {
  explicit Terminal(Device &device):
  Surface{&device},
  device_{device},
  backdrop_{this}
  {}
  ~Terminal() override = default;
  Terminal(const Terminal &) = delete;
  Terminal(Terminal &&) = delete;
  auto operator=(const Terminal &) -> Terminal & = delete;
  auto operator=(Terminal &&) -> Terminal & = delete;
  void focus(struct Window */*window*/) override {}
  auto surface() -> Surface * override { return this;}

  [[nodiscard]] auto size() const -> Vector override {return jwezel::VectorMax;}
  auto expand(const Vector &/*size*/) -> bool override {return false;}
  auto contract() -> bool override {return false;}
  void moveWindow(Window &/*window*/, const Rectangle & /*area*/) override {}

  Device device_;
  Backdrop backdrop_;
};

namespace {

auto inside(const Rectangle &area, const Vector &position) -> bool {
  return position.x() >= area.x1() and position.x() < area.x2() and position.y() >= area.y1() and position.y() < area.y2();
}

// The element visible at a position is the topmost whose area contains it
auto topmost(const Surface &screen, const Vector &position) -> const Surface::Element * {
  const Surface::Element *result{nullptr};
  for (const auto *element: screen.zorder()) {
    if (inside(element->area(), position)) {
      result = element;
    }
  }
  return result;
}

// The topmost element of all cells of a region, by painting the elements bottom to top
auto topmostCells(const Surface &screen, Dim width, Dim height) -> vector<const Surface::Element *> {
  vector<const Surface::Element *> result(static_cast<size_t>(width) * static_cast<size_t>(height), nullptr);
  for (const auto *element: screen.zorder()) {
    const auto area{element->area() & Rectangle{0, 0, width, height}};
    if (!area) {
      continue;
    }
    for (Dim y = area->y1(); y < area->y2(); ++y) {
      for (Dim x = area->x1(); x < area->x2(); ++x) {
        result[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = element;
      }
    }
  }
  return result;
}

} // namespace

// Test fragments: compare the surface with the definition of visibility. Every cell in the region is in exactly
// one fragment of the surface, which belongs to the topmost element whose area has the cell, and find() gets that
// fragment. Fragments must be inside the area of their element.
void TestFragments(const Surface &screen, Dim width = 40, Dim height = 24) {
  // One assertion for all cells: a check per cell makes the tests very slow
  string problem;
  const auto cells{static_cast<size_t>(width) * static_cast<size_t>(height)};
  vector<int> counts(cells, 0);
  vector<const Surface::Fragment *> owners(cells, nullptr);
  for (const auto *element: screen.zorder()) {
    for (const auto &fragment: element->fragments()) {
      if (fragment.element != element or (fragment.area & element->area()) != std::optional{fragment.area}) {
        problem = format("Fragment {} is not inside its element {}", string(fragment), string(element->area()));
      }
      if (const auto area{fragment.area & Rectangle{0, 0, width, height}}; area) {
        for (Dim y = area->y1(); y < area->y2(); ++y) {
          for (Dim x = area->x1(); x < area->x2(); ++x) {
            const auto cell{static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)};
            ++counts[cell];
            owners[cell] = &fragment;
          }
        }
      }
    }
  }
  const auto tops{topmostCells(screen, width, height)};
  for (Dim y = 0; y < height and problem.empty(); ++y) {
    for (Dim x = 0; x < width and problem.empty(); ++x) {
      const Vector position{x, y};
      const auto cell{static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)};
      if (counts[cell] != (tops[cell]? 1: 0)) {
        problem = format("{} fragments at {}, expected {}", counts[cell], string(position), tops[cell]? 1: 0);
      } else if (owners[cell] and owners[cell]->element != tops[cell]) {
        problem = format("Fragment at {} belongs to the wrong element", string(position));
      } else if (screen.find(position) != owners[cell]) {
        problem = format("find() is wrong at {}", string(position));
      }
    }
  }
  CHECK_EQ(problem, "");
}

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTBEGIN(readability-function-cognitive-complexity)
// NOLINTBEGIN(hicpp-no-assembler)
// NOLINTBEGIN(hicpp-avoid-c-arrays)
// NOLINTBEGIN(hicpp-no-array-decay)
// NOLINTBEGIN(modernize-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(misc-use-anonymous-namespace)

TEST_CASE("Surface") {
  Device dev;
  Terminal term{dev};
  Surface &scr{*term.surface()};
  SUBCASE("Constructor") {
    CHECK_EQ(scr.zorder().size(), 1);
    TestFragments(scr);
  }
  SUBCASE("AddWindow") {
    dev.updates_.clear();
    Window win1{&term, Rectangle{1, 1, 9, 5}};
    SUBCASE("Window") {
      TestFragments(scr);
      auto expupdates{Updates{{Vector{1, 1}, Text("        \n        \n        \n        ")}}};
      CHECK_EQ(&win1, scr.zorder()[1]);
      CHECK_EQ(dev.updates_, expupdates);
      CHECK_EQ(scr.zorder().size(), 2);
      REQUIRE_EQ(
        scr.zorder()[0]->fragments(), vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{0, 0, DimHigh, 1}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{0, 1, 1, 5}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{9, 1, DimHigh, 5}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{0, 5, DimHigh, DimHigh}, scr.zorder()[0]}
        }
      );
      CHECK_EQ(scr.zorder()[1]->fragments(), vector<Surface::Fragment>{Surface::Fragment{Rectangle{1, 1, 9, 5},scr.zorder()[1]}});
      CHECK_EQ(string(scr.zorder()[1]->text()), "        \n        \n        \n        \n");
    }
    SUBCASE("AddWindow below") {
      dev.updates_.clear();
      Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C, &win1};
      TestFragments(scr);
      // MESSAGE(win1.fragments[0]);
      // for (const auto &f: term.backdrop_.fragments) {
      //   MESSAGE(f);
      // }
      REQUIRE_EQ(
        scr.zorder()[0]->fragments(), vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{0, 0, 2, 1}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{8, 0, DimHigh, 1}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{0, 1, 1, 5}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{9, 1, DimHigh, 5}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{0, 5, 2, 6}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{8, 5, DimHigh, 6}, scr.zorder()[0]},
          Surface::Fragment{Rectangle{0, 6, DimHigh, DimHigh}, scr.zorder()[0]}
        }
      );
      CHECK_EQ(scr.find(Vector{0, 0}), &term.backdrop_.fragments()[0]);
      CHECK_EQ(scr.find(Vector{1, 1}), &win1.fragments()[0]);
      CHECK_EQ(scr.find(Vector{8, 1}), &win1.fragments()[0]);
      CHECK_EQ(scr.find(Vector{2, 0}), &win2.fragments()[0]);
      CHECK_EQ(scr.find(Vector{2, 5}), &win2.fragments()[1]);
      auto expupdates2{Updates{{Vector{2, 0}, Text("......")}, {Vector{2, 5}, Text("......")}}};
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(scr.zorder().size(), 3);
      CHECK_EQ(scr.zorder()[1], &win2);
      CHECK_EQ(scr.zorder()[2], &win1);
      CHECK_EQ(
        scr.zorder()[1]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{2, 0, 8, 1}, scr.zorder()[1]}, Surface::Fragment{Rectangle{2, 5, 8, 6}, scr.zorder()[1]}
        }
      );
      CHECK_EQ(scr.zorder()[2]->fragments(), vector<Surface::Fragment>{Surface::Fragment{Rectangle{1, 1, 9, 5}, scr.zorder()[2]}});
      SUBCASE("ReshapeWindow") {
        dev.updates_.clear();
        scr.reshapeElement(&win2, Rectangle{4, 0, 10, 6});
        TestFragments(scr);
        std::ranges::sort(dev.updates_);
        CHECK_EQ(
          dev.updates_,
          Updates{
            {
              {Vector{2, 0}, Text("  ")},
              {Vector{2, 5}, Text("  ")},
              {Vector{4, 0}, Text("......")},
              {Vector{4, 5}, Text("......")},
              {Vector{9, 1}, Text(".\n.\n.\n.")}
            }
          }
        );
        CHECK_EQ(
          scr.zorder()[1]->fragments(),
          vector<Surface::Fragment>{
            Surface::Fragment{Rectangle{4, 0, 10, 1}, scr.zorder()[1]},
            Surface::Fragment{Rectangle{9, 1, 10, 5}, scr.zorder()[1]},
            Surface::Fragment{Rectangle{4, 5, 10, 6}, scr.zorder()[1]}
          }
        );
      }
      SUBCASE("Fill") {
        INFO("Window 1: ", string(win1));
        INFO("Window 2: ", string(win2));
        dev.updates_.clear();
        win1.fill(Char('1'));
        CHECK_EQ(
          dev.updates_,
          Updates{
            {
              {Vector{1, 1}, Text("11111111\n11111111\n11111111\n11111111")}
            }
          }
        );
      }
      SUBCASE("Fill covered window") {
        dev.updates_.clear();
        win2.fill(Char('2'));
        std::ranges::sort(dev.updates_);
        CHECK_EQ(
          dev.updates_,
          Updates{
            {
              {Vector{2, 0}, Text("222222")},
              {Vector{2, 5}, Text("222222")},
            }
          }
        );
      }
      dev.updates_.clear();
    }
    SUBCASE("Delete window") {
      {
        Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C, &win1};
        TestFragments(scr);
        dev.updates_.clear();
      }
      TestFragments(scr);
      auto expupdates2{
        Updates{
            {Vector{2, 0}, Text("      ")},
            {Vector{2, 5}, Text("      ")},
        }
      };
      std::ranges::sort(dev.updates_);
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(scr.zorder().size(), 2);
      CHECK_EQ(scr.zorder()[1], &win1);
      CHECK_EQ(
        scr.zorder()[1]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{1, 1, 9, 5}, scr.zorder()[1]}
        }
      );
    }
    SUBCASE("Shift down") {
      Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C};
      TestFragments(scr);
      dev.updates_.clear();
      scr.above(&win2, &term.backdrop_);
      TestFragments(scr);
      auto expupdates2{
        Updates{
            {Vector{2, 1}, Text("      \n      \n      \n      ")},
        }
      };
      std::ranges::sort(dev.updates_);
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(
        scr.zorder()[1]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{2, 0, 8, 1}, scr.zorder()[1]},
          Surface::Fragment{Rectangle{2, 5, 8, 6}, scr.zorder()[1]}
        }
      );
      CHECK_EQ(
        scr.zorder()[2]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{1, 1, 9, 5}, scr.zorder()[2]}
        }
      );
    }
    SUBCASE("Shift up") {
      Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C};
      TestFragments(scr);
      dev.updates_.clear();
      scr.below(&win1);
      TestFragments(scr);
      auto expupdates2{
        Updates{
            {Vector{2, 1}, Text("      \n      \n      \n      ")},
        }
      };
      std::ranges::sort(dev.updates_);
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(
        scr.zorder()[1]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{2, 0, 8, 1}, scr.zorder()[1]},
          Surface::Fragment{Rectangle{2, 5, 8, 6}, scr.zorder()[1]}
        }
      );
      CHECK_EQ(
        scr.zorder()[2]->fragments(),
        vector<Surface::Fragment>{
          Surface::Fragment{Rectangle{1, 1, 9, 5}, scr.zorder()[2]}
        }
      );
    }
    SUBCASE("Reordering at the ends") {
      // win1 {1, 1, 9, 5} is below win2 {2, 0, 8, 6}
      Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C};
      const auto order{
        [&] {
          return vector<const Surface::Element *>(scr.zorder().begin(), scr.zorder().end());
        }
      };
      const vector<const Surface::Element *> win1Below{&term.backdrop_, &win1, &win2};
      const vector<const Surface::Element *> win1Above{&term.backdrop_, &win2, &win1};
      const Updates win1Shows{{Vector{2, 1}, Text("      \n      \n      \n      ")}};
      dev.updates_.clear();
      SUBCASE("Above nothing is the top") {
        scr.above(&win1);
        TestFragments(scr);
        CHECK_EQ(order(), win1Above);
        CHECK_EQ(dev.updates_, win1Shows);
      }
      SUBCASE("Above nothing, already on top") {
        scr.above(&win2);
        TestFragments(scr);
        CHECK_EQ(order(), win1Below);
        CHECK(dev.updates_.empty());
      }
      SUBCASE("Below nothing is the top as well") {
        scr.below(&win1);
        TestFragments(scr);
        CHECK_EQ(order(), win1Above);
        CHECK_EQ(dev.updates_, win1Shows);
      }
      SUBCASE("Below the backdrop is the bottom") {
        scr.below(&win2, &term.backdrop_);
        TestFragments(scr);
        CHECK_EQ(order(), win1Above);
        CHECK_EQ(dev.updates_, win1Shows);
      }
      SUBCASE("Below the backdrop, already at the bottom") {
        scr.below(&win1, &term.backdrop_);
        TestFragments(scr);
        CHECK_EQ(order(), win1Below);
        CHECK(dev.updates_.empty());
      }
      SUBCASE("Above the element it is above already") {
        scr.above(&win2, &win1); // used to overwrite another place in the z-order
        TestFragments(scr);
        CHECK_EQ(order(), win1Below);
        CHECK(dev.updates_.empty());
      }
      SUBCASE("Below the element it is below already") {
        scr.below(&win1, &win2);
        TestFragments(scr);
        CHECK_EQ(order(), win1Below);
        CHECK(dev.updates_.empty());
      }
      SUBCASE("The backdrop does not move") {
        CHECK_THROWS_AS(scr.above(&term.backdrop_, &win2), std::logic_error);
        CHECK_THROWS_AS(scr.below(&term.backdrop_, &win1), std::logic_error);
        CHECK_THROWS_AS(scr.above(&term.backdrop_), std::logic_error);
        CHECK_EQ(order(), win1Below);
      }
      SUBCASE("One up and one down") {
        CHECK(win1.above(nullptr));
        CHECK_EQ(order(), win1Above);
        CHECK_FALSE(win1.above(nullptr)); // on top
        CHECK_EQ(order(), win1Above);
        CHECK(win1.below(nullptr));
        CHECK_EQ(order(), win1Below);
        CHECK_FALSE(win1.below(nullptr)); // at the bottom
        CHECK_EQ(order(), win1Below);
        TestFragments(scr);
      }
      SUBCASE("By position") {
        CHECK(win1.above(-1)); // above the topmost
        CHECK_EQ(order(), win1Above);
        CHECK(win1.below(0)); // below the backdrop is the bottom
        CHECK_EQ(order(), win1Below);
        CHECK(win1.above(2));
        CHECK_EQ(order(), win1Above);
        CHECK_THROWS_AS(win1.above(3), std::out_of_range);
        CHECK_THROWS_AS(win1.below(-4), std::out_of_range);
        CHECK_EQ(order(), win1Above);
        TestFragments(scr);
      }
    }
  }
}


// A device that paints the updates into a grid, like a display
struct PaintDevice: Device {
  static const Dim width{40};
  static const Dim height{24};

  PaintDevice():
  grid_(height, string(width, ' '))
  {}

  void update(const Updates &updates) override {
    for (const auto &update_: updates) {
      for (size_t row = 0; row < update_.text.data.size(); ++row) {
        for (size_t column = 0; column < update_.text.data[row].size(); ++column) {
          const auto x{update_.position.x() + static_cast<Dim>(column)}, y{update_.position.y() + static_cast<Dim>(row)};
          if (x >= 0 and x < width and y >= 0 and y < height) {
            grid_[y][x] = static_cast<char>(update_.text.data[row][column].rune);
          }
        }
      }
    }
  }

  // What the screen has to show: the character of the element on top of each cell
  [[nodiscard]] auto expected(const Surface &screen) const -> vector<string> {
    vector<string> result(height, string(width, ' '));
    const auto tops{topmostCells(screen, width, height)};
    for (Dim y = 0; y < height; ++y) {
      for (Dim x = 0; x < width; ++x) {
        if (const auto *element{tops[static_cast<size_t>(y) * width + static_cast<size_t>(x)]};
            element and element != screen.zorder()[0]) {
          result[y][x] = static_cast<char>(element->text(Rectangle{0, 0, 1, 1}).data[0][0].rune);
        }
      }
    }
    return result;
  }

  vector<string> grid_;
};

// An element that is nothing but an area, to build a surface from scratch
struct BlockElement: Surface::Element {
  BlockElement(Surface *surface, const Rectangle &area): Element{surface}, area_{area} {}
  [[nodiscard]] auto area() const -> Rectangle override {return area_;}
  [[nodiscard]] auto text(const Rectangle &area = jwezel::RectangleMax) const -> Text override {
    return Text{jwezel::Space, area.size()};
  }
  auto moveEvent(const Rectangle &area) -> bool override {
    area_ = area;
    return true;
  }
  void move(const Rectangle &area) override {surface()->reshapeElement(this, area);}
  Rectangle area_;
};

// Number of fragments if the surface were built from scratch with its elements in z-order
auto fragmentsFromScratch(const Surface &screen) -> size_t {
  Device dev;
  Surface fresh{&dev};
  vector<std::unique_ptr<BlockElement>> blocks;
  for (const auto *element: screen.zorder()) {
    blocks.push_back(std::make_unique<BlockElement>(&fresh, element->area()));
    fresh.addElement(blocks.back().get());
  }
  size_t result{0};
  for (const auto *element: fresh.zorder()) {
    result += element->fragments().size();
  }
  return result;
}

TEST_CASE("Surface fragments stay bounded") {
  // Elements that come, go, move and change their order for a long time must not make fragments multiply
  Device dev;
  Surface scr{&dev};
  Backdrop backdrop{&scr};
  std::mt19937 random{7};
  auto number = [&random](int low, int high) {return std::uniform_int_distribution<int>{low, high}(random);};
  auto area = [&] {
    const auto x{number(0, 70)}, y{number(0, 40)};
    return Rectangle{Dim(x), Dim(y), Dim(x + number(2, 30)), Dim(y + number(2, 12))};
  };
  vector<std::unique_ptr<BlockElement>> blocks;
  for (auto i = 0; i < 24; ++i) {
    blocks.push_back(std::make_unique<BlockElement>(&scr, area()));
    scr.addElement(blocks.back().get());
  }
  auto pick = [&] {return blocks[static_cast<size_t>(number(0, static_cast<int>(blocks.size()) - 1))].get();};
  auto fragments = [&] {
    size_t result{0};
    for (const auto *element: scr.zorder()) {
      result += element->fragments().size();
    }
    return result;
  };
  size_t peak{0};
  for (auto step = 1; step <= 4000; ++step) {
    auto *block{pick()};
    auto *other{pick()};
    switch (number(0, 3)) {
      case 0:
      case 1:
      block->move(area());
      break;

      case 2:
      scr.above(block, other);
      break;

      default:
      scr.below(block, other);
    }
    peak = std::max(peak, fragments());
    if (step % 500 == 0) {
      INFO("Step ", step);
      CHECK_LE(fragments(), 3 * fragmentsFromScratch(scr) + 20);
    }
  }
  MESSAGE("Peak ", peak, " fragments, from scratch now ", fragmentsFromScratch(scr), ", now ", fragments());
  CHECK_LE(peak, 400); // without tidying up, this walk goes beyond 430
  TestFragments(scr, 100, 60);
}

TEST_CASE("Surface model") {
  // Random changes to a surface; after each of them the fragments have to be right and the device must have been
  // told everything that changed on the screen
  for (auto seed = 1U; seed <= 8; ++seed) {
    INFO("Seed: ", seed);
    PaintDevice dev;
    Terminal term{dev};
    Surface &scr{*term.surface()};
    std::mt19937 random{seed};
    auto number = [&random](int low, int high) {return std::uniform_int_distribution<int>{low, high}(random);};
    vector<std::unique_ptr<jwezel::TextElement>> elements;
    string letters{"abcdefghijklmnopqrstuvwxyz"};
    auto area = [&] {
      const auto x{number(0, 34)}, y{number(0, 18)};
      return Rectangle{Dim(x), Dim(y), Dim(x + number(1, 40 - x)), Dim(y + number(1, 24 - y))};
    };
    auto pick = [&] {return elements[static_cast<size_t>(number(0, static_cast<int>(elements.size()) - 1))].get();};
    size_t maxFragments{0};
    double worstRatio{0.};
    for (auto step = 0; step < 250; ++step) {
      string operation;
      const auto choice{number(0, 9)};
      if (elements.size() < 3 or (choice < 3 and elements.size() < 16)) {
        operation = "add";
        const auto letter{letters.back()};
        letters.pop_back();
        auto *below{!elements.empty() and number(0, 2) == 0? pick(): nullptr};
        elements.push_back(std::make_unique<jwezel::TextElement>(&scr, area(), Char(letter), below));
      } else if (choice < 5) {
        operation = "delete";
        auto *element{pick()};
        letters.push_back(static_cast<char>(element->text(Rectangle{0, 0, 1, 1}).data[0][0].rune));
        std::erase_if(elements, [element](const auto &e) {return e.get() == element;});
      } else if (choice < 8) {
        operation = "move";
        auto *element{pick()};
        auto target{area()};
        if (number(0, 2) == 0) { // move only
          const auto current{element->area()};
          const auto dx{Dim(number(-3, 3))}, dy{Dim(number(-2, 2))};
          target = current + Vector{dx, dy};
          if (target.x1() < 0 or target.y1() < 0 or target.x2() > 40 or target.y2() > 24) {
            target = current;
          }
        }
        element->move(target);
      } else {
        auto *element{pick()};
        auto *other{pick()};
        // The destination is another element, or none, or the backdrop
        const auto kind{number(0, 3)};
        Surface::Element *destination{kind < 2? static_cast<Surface::Element *>(other): kind == 2? nullptr: scr.zorder()[0]};
        if (element != destination) {
          if (choice == 8) {
            operation = "above";
            scr.above(element, destination);
          } else {
            operation = "below";
            scr.below(element, destination);
          }
        }
      }
      INFO("Step ", step, ": ", operation);
      TestFragments(scr);
      REQUIRE_EQ(dev.grid_, dev.expected(scr));
      size_t fragments{0};
      for (const auto *element: scr.zorder()) {
        fragments += element->fragments().size();
      }
      maxFragments = std::max(maxFragments, fragments);
      if (step % 5 == 0) {
        worstRatio = std::max(worstRatio, static_cast<double>(fragments) / static_cast<double>(fragmentsFromScratch(scr)));
      }
    }
    MESSAGE("seed ", seed, ": most fragments ", maxFragments, ", at worst ", worstRatio, " times those from scratch");
  }
}

TEST_CASE("ElementDevice") {
  Device dev;
  Terminal term{dev};
  Surface &scr{*term.surface()};
  jwezel::ElementDevice device{&term, Rectangle{1, 1, 9, 5}, '.'_C};
  REQUIRE_EQ(device.area(), Rectangle{1, 1, 9, 5});
  CHECK_EQ(device.text(), Text{'.'_C, Vector{8, 4}});
  CHECK_EQ(device.ownText(), Text{'.'_C, Vector{8, 4}});

  SUBCASE("Own drawing") {
    dev.updates_.clear();
    device.write(Vector{1, 1}, "hello");
    auto expected{Text{'.'_C, Vector{8, 4}}};
    expected.patch(Text{"hello"}, Vector{1, 1});
    CHECK_EQ(device.text(), expected);
    CHECK_EQ(device.ownText(), expected);
    CHECK_EQ(dev.updates_, Updates{{Vector{2, 2}, Text{"hello"}}});
  }

  SUBCASE("Content") {
    dev.updates_.clear();
    jwezel::TextElement child{&device.content(), Rectangle{2, 1, 6, 3}, '#'_C};
    auto expected{Text{'.'_C, Vector{8, 4}}};
    expected.patch(Text{'#'_C, Vector{4, 2}}, Vector{2, 1});
    CHECK_EQ(device.text(), expected);
    CHECK_EQ(device.ownText(), Text{'.'_C, Vector{8, 4}});
    CHECK_EQ(dev.updates_, Updates{{Vector{3, 2}, Text{"####\n####"}}});
    SUBCASE("Clipping") {
      // The child is wider than the device; what is outside of the device is not shown
      CHECK_EQ(device.content().position(&child), 1);
      CHECK_EQ(child.surface(), &device.content());
      child.move(Rectangle{4, 1, 12, 3});
      CHECK_EQ(device.text().size(), Vector{8, 4});
      CHECK_EQ(device.text()[Vector{4, 1}].rune, '#'_C.rune);
      CHECK_EQ(device.text()[Vector{7, 2}].rune, '#'_C.rune);
      CHECK_EQ(device.text()[Vector{0, 1}].rune, '.'_C.rune);
      // and the device only passes on the part of the update that it shows
      dev.updates_.clear();
      child.write(Vector{0, 0}, Text{"abcdefgh"});
      CHECK_EQ(dev.updates_, Updates{{Vector{5, 2}, Text{"abcd"}}});
    }
    SUBCASE("Covered child") {
      jwezel::TextElement above{&device.content(), Rectangle{3, 1, 5, 2}, 'b'_C};
      CHECK_EQ(device.text()[Vector{2, 1}].rune, '#'_C.rune);
      CHECK_EQ(device.text()[Vector{3, 1}].rune, 'b'_C.rune);
      CHECK_EQ(device.text()[Vector{4, 1}].rune, 'b'_C.rune);
      CHECK_EQ(device.text()[Vector{5, 1}].rune, '#'_C.rune);
      SUBCASE("Shows") {
        CHECK(device.shows(Vector{1 + 3, 1 + 1}, &above));
        CHECK_FALSE(device.shows(Vector{1 + 3, 1 + 1}, &child));
        CHECK(device.shows(Vector{1 + 2, 1 + 1}, &child));
        CHECK_FALSE(device.shows(Vector{1 + 0, 1 + 0}, &child));
      }
    }
  }

  SUBCASE("Occlusion") {
    Window above{&term, Rectangle{3, 2, 7, 4}};
    CHECK_EQ(scr.find(Vector{3, 2}), &above.fragments()[0]);
    CHECK_EQ(scr.find(Vector{1, 1}), &device.fragments()[0]);
    CHECK_EQ(device.text(), Text{'.'_C, Vector{8, 4}});
  }

  SUBCASE("Move") {
    jwezel::TextElement child{&device.content(), Rectangle{2, 1, 6, 3}, '#'_C};
    auto expected{device.text()};
    device.move(Rectangle{2, 2, 10, 6});
    CHECK_EQ(device.area(), Rectangle{2, 2, 10, 6});
    CHECK_EQ(device.text(), expected);
    CHECK_EQ(scr.find(Vector{2, 2}), &device.fragments()[0]);
  }
}

// NOLINTEND(misc-use-anonymous-namespace)
// NOLINTEND(readability-magic-numbers)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
// NOLINTEND(cppcoreguidelines-avoid-c-arrays)
// NOLINTEND(modernize-avoid-c-arrays)
// NOLINTEND(hicpp-no-array-decay)
// NOLINTEND(hicpp-avoid-c-arrays)
// NOLINTEND(hicpp-no-assembler)
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// NOLINTEND(misc-non-private-member-variables-in-classes)
