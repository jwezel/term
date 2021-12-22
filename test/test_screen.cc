#include <tuple>
#include <vector>
#include <algorithm>

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <range/v3/view/all.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/view/ref.hpp>

#include "geometry.hh"
#include "screen.hh"
#include "string.hh"
#include "text.hh"

using namespace jwezel;

namespace doctest {
  template<> struct StringMaker<Text> {
    static String convert(const Text& value) {
      return value.repr().c_str();
    }
  };

  template<> struct StringMaker<Vector> {
    static String convert(const Vector& value) {
      return string(value).c_str();
    }
  };

  template<> struct StringMaker<Rectangle> {
    static String convert(const Rectangle& value) {
      return string(value).c_str();
    }
  };

  template<> struct StringMaker<Updates> {
    static String convert(const Updates& value) {
      return fmt::format(
        "[{}]",
        fmt::join(value | ranges::views::transform(str<Update>) | ranges::_to_::to<vector<string>>(), ", ")
      ).c_str();
    }
  };
}

TEST_CASE("Screen") {
  Screen s(Vector{10, 6});
  SUBCASE("Constructor") {
    CHECK_EQ(s.zorder.size(), 1);
    CHECK_EQ(&s.screen, s.zorder[0]);
    CHECK_EQ(&s.screen, s.focusWindow);
    CHECK_EQ(string(s.screen.text()), "          \n          \n          \n          \n          \n          \n");
  }
  SUBCASE("AddWindow") {
    auto [win1, updates]{s.addWindow(Rectangle{1, 1, 9, 5})};
    auto [expwin1, expupdates]{make_tuple(s.zorder[1], Updates{{{1, 1}, Text("        \n        \n        \n        ")}})};
    CHECK_EQ(win1, expwin1);
    CHECK_EQ(updates, expupdates);
    CHECK_EQ(s.zorder.size(), 2);
    CHECK_EQ(s.zorder[1], s.focusWindow);
    CHECK_EQ(string(s.zorder[1]->text()), "        \n        \n        \n        \n");
    SUBCASE("AddWindow below") {
      auto [win2, updates]{s.addWindow(Rectangle{2, 0, 8, 6}, Space, win1)};
      auto [expwin, expupdates]{make_tuple(s.zorder[1], Updates{{{2, 0}, Text("      ")}, {{2, 5}, Text("      ")}})};
      CHECK_EQ(win2, expwin);
      CHECK_EQ(updates, expupdates);
      CHECK_EQ(s.zorder.size(), 3);
      CHECK_EQ(s.zorder[1], s.focusWindow);
      CHECK_EQ(s.zorder[1], win2);
      SUBCASE("DeleteWindow") {
        auto updates{s.deleteWindow(win2)};
        auto expupdates{Updates{{{2, 0}, Text("      ")}, {{2, 5}, Text("      ")}}};
        CHECK_EQ(updates, expupdates);
        CHECK_EQ(s.zorder.size(), 2);
        CHECK_EQ(s.zorder[1], s.focusWindow);
        CHECK_EQ(s.zorder[1], win1);
      }
      SUBCASE("ReshapeWindow") {
        auto updates{s.reshapeWindow(win2, Rectangle{4, 0, 10, 6})};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {{2, 0}, Text("  ")},
              {{2, 5}, Text("  ")},
              {{4, 0}, Text("      ")},
              {{4, 5}, Text("      ")},
              {{9, 1}, Text(" \n \n \n ")}
            }
          }
        );
      }
      SUBCASE("Fill") {
        auto updates{s.fill(win1, Char('1'))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {{1, 1}, Text("11111111\n11111111\n11111111\n11111111")}
            }
          }
        );
      }
      SUBCASE("Fill covered window") {
        auto updates{s.fill(win2, Char('2'))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {{2, 0}, Text("222222")},
              {{2, 5}, Text("222222")},
            }
          }
        );
      }
      SUBCASE("Text") {
        auto updates{s.text(win1, {0, 0}, Text("aaa\nbbbb"))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {{1, 1}, Text("aaa\nbbbb")}
            }
          }
        );
      }
      SUBCASE("Text covered window") {
        auto updates{s.text(win2, {0, 0}, Text("qqq\nqqq\nqqq\nqqq\nqqq\nwwww"))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {{2, 0}, Text("qqq ")}, // space here because update is a rectangle
              {{2, 5}, Text("wwww")}
            }
          }
        );
      }
      SUBCASE("Line") {
        auto updates{s.line(win1, Line{{0, 0}, 8, Horizontal, false, false}, 2)};
        CHECK_EQ(
          updates,
          Updates{{{{1, 1}, Text("╺━━━━━━╸")}}}
        );
      }
      SUBCASE("Line in covered window") {
        auto updates{s.line(win2, Line{{0, 0}, 6, Vertical, true, true}, 2)};
        CHECK_EQ(
          updates,
          Updates{{{{2, 0}, Text("┃")}, {{2, 5}, Text("┃")}}}
        );
      }
      SUBCASE("Box") {
        auto updates{s.box(win1, Box(Rectangle{0, 0, DimHigh, DimHigh}))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {{1, 1}, Text("┌──────┐")},
              {{1, 1}, Text("┌\n│\n│\n└")},
              {{1, 4}, Text("└──────┘")},
              {{8, 1}, Text("┐\n│\n│\n┘")},
            }
          }
        );
      }
      SUBCASE("Box in covered window") {
        auto updates{s.box(win2, Box(Rectangle{0, 0, DimHigh, DimHigh}))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {{2, 0}, Text("┌────┐")},
              {{2, 0}, Text("┌")},
              {{2, 5}, Text("└────┘")},
              {{2, 5}, Text("└")},
              {{7, 0}, Text("┐")},
              {{7, 5}, Text("┘")}
            }
          }
        );
      }
    }
  }
}
