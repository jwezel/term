
#include "device.hh"
#include "geometry.hh"
#include "string.hh"
#include "surface.hh"
#include "text.hh"
#include "update.hh"
#include "window.hh"

#include <algorithm>
#include <format>
#include <iterator>
#include <ranges>
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

struct Device: jwezel::Device {
  void update(const Updates &updates) override {
    copy(updates.begin(), updates.end(), back_inserter(updates_));
  }
  Updates updates_;
};

struct Terminal: jwezel::TerminalInterface {
  explicit Terminal(Device &device):
  device_{device},
  backdrop_{this},
  screen_{&device, {&backdrop_}}
  {}
  ~Terminal() override = default;
  Terminal(const Terminal &) = delete;
  Terminal(Terminal &&) = delete;
  auto operator=(const Terminal &) -> Terminal & = delete;
  auto operator=(Terminal &&) -> Terminal & = delete;
  void registerWindow(struct Window */*window*/) override {}
  auto screen() -> Surface & override { return screen_;}
  auto expand(const Vector &/*size*/) -> bool override {return false;}
  auto contract() -> bool override {return false;}
  void moveWindow(Window &/*window*/, const Rectangle & /*area*/) override {}

  Device device_;
  Backdrop backdrop_;
  Surface screen_;
};

// Test rtree: check whether all corners of all fragments can be found in rtree
void TestRtree(const Surface &screen) {
  for (const auto *element: screen.zorder()) {
    for (const auto &f: element->fragments()) {
      const auto &a{f.area};
      CHECK_EQ(screen.find(a.position()), &f);
      if (a.x2() > a.x1()) {
        CHECK_EQ(screen.find(Vector{Dim(a.x2() - 1), a.y1()}), &f);
        if (a.y2() > a.y1()) {
          CHECK_EQ(screen.find(Vector{Dim(a.x2() - 1), Dim(a.y2() - 1)}), &f);
        }
      } else {
        if (a.y2() > a.y1()) {
          CHECK_EQ(screen.find(Vector{a.x1(), Dim(a.y2() - 1)}), &f);
        }
      }
    }
  }
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
  Surface &scr{term.screen()};
  SUBCASE("Constructor") {
    CHECK_EQ(scr.zorder().size(), 1);
    TestRtree(scr);
  }
  SUBCASE("AddWindow") {
    dev.updates_.clear();
    Window win1{&term, Rectangle{1, 1, 9, 5}};
    SUBCASE("Window") {
      TestRtree(scr);
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
      TestRtree(scr);
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
        TestRtree(scr);
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
        TestRtree(scr);
        dev.updates_.clear();
      }
      TestRtree(scr);
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
      TestRtree(scr);
      dev.updates_.clear();
      scr.above(&win2, &term.backdrop_);
      TestRtree(scr);
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
      TestRtree(scr);
      dev.updates_.clear();
      scr.below(&win1);
      TestRtree(scr);
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
