
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
  jwezel::Vector,
  jwezel::DimHigh,
  jwezel::Char,
  jwezel::Rectangle,
  jwezel::Text,
  jwezel::string,
  jwezel::str,
  jwezel::Update,
  jwezel::Updates,
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
  auto expand(const Vector &) -> bool override {return false;}
  auto contract() -> bool override {return false;}
  void moveWindow(Window &, const Rectangle &) override {}

  private:
  Device device_;
  Backdrop backdrop_;
  Surface screen_;
};

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
    CHECK_EQ(scr.zorder.size(), 1);
  }
  SUBCASE("AddWindow") {
    dev.updates_.clear();
    Window win1{&term, Rectangle{1, 1, 9, 5}};
    auto expupdates{Updates{{Vector{1, 1}, Text("        \n        \n        \n        ")}}};
    CHECK_EQ(&win1, scr.zorder[1]);
    CHECK_EQ(dev.updates_, expupdates);
    CHECK_EQ(scr.zorder.size(), 2);
    REQUIRE_EQ(
      scr.zorder[0]->fragments, vector<Rectangle>{
        Rectangle{0, 0, DimHigh, 1},
        Rectangle{0, 1, 1, 5},
        Rectangle{9, 1, DimHigh, 5},
        Rectangle{0, 5, DimHigh, DimHigh}
      }
    );
    CHECK_EQ(scr.zorder[1]->fragments, vector<Rectangle>{Rectangle{1, 1, 9, 5}});
    CHECK_EQ(string(scr.zorder[1]->text()), "        \n        \n        \n        \n");
    SUBCASE("AddWindow below") {
      dev.updates_.clear();
      Window win2{&term, Rectangle{2, 0, 8, 6}, '.'_C, &win1};
      auto expupdates2{Updates{{Vector{2, 0}, Text("......")}, {Vector{2, 5}, Text("......")}}};
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(scr.zorder.size(), 3);
      CHECK_EQ(scr.zorder[1], &win2);
      CHECK_EQ(scr.zorder[2], &win1);
      CHECK_EQ(scr.zorder[1]->fragments, vector<Rectangle>{Rectangle{2, 0, 8, 1}, Rectangle{2, 5, 8, 6}});
      CHECK_EQ(scr.zorder[2]->fragments, vector<Rectangle>{Rectangle{1, 1, 9, 5}});
      SUBCASE("ReshapeWindow") {
        dev.updates_.clear();
        scr.reshapeElement(&win2, Rectangle{4, 0, 10, 6});
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
        CHECK_EQ(scr.zorder[1]->fragments, vector<Rectangle>{Rectangle{4, 0, 10, 1}, Rectangle{9, 1, 10, 5}, Rectangle{4, 5, 10, 6}});
      }
      SUBCASE("Fill") {
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
        dev.updates_.clear();
      }
      auto expupdates2{
        Updates{
            {Vector{2, 0}, Text("      ")},
            {Vector{2, 5}, Text("      ")},
        }
      };
      std::ranges::sort(dev.updates_);
      CHECK_EQ(dev.updates_, expupdates2);
      CHECK_EQ(scr.zorder.size(), 2);
      CHECK_EQ(scr.zorder[1], &win1);
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
