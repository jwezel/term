#include <algorithm>
#include <tuple>
#include <vector>

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <range/v3/action/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/ref.hpp>
#include <range/v3/view/transform.hpp>

#include "geometry.hh"
#include "screen.hh"
#include "string.hh"
#include "text.hh"
#include "update.hh"

using
  jwezel::Vector,
  jwezel::Box,
  jwezel::Line,
  jwezel::DimHigh,
  jwezel::Char,
  jwezel::Horizontal,
  jwezel::Vertical,
  jwezel::Rectangle,
  jwezel::Text,
  jwezel::Space,
  jwezel::string,
  jwezel::str,
  jwezel::Update,
  jwezel::Updates
;

using
  jwezel::screen::Screen;
using jwezel::screen::Window;
;

using std::vector;

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
      return fmt::format(
        "[{}]",
        fmt::join(value | ranges::views::transform(str<Update>) | ranges::_to_::to<vector<string>>(), ", ")
      ).c_str();
    }
  };
} // namespace doctest

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

TEST_CASE("Screen") {
  Screen scr;
  SUBCASE("Constructor") {
    CHECK_EQ(scr.zorder.size(), 1);
  }
  SUBCASE("AddWindow") {
    auto [win1, updates]{scr.addWindow(Rectangle{1, 1, 9, 5})};
    auto expupdates{Updates{{Vector{1, 1}, Text("        \n        \n        \n        ")}}};
    CHECK_EQ(win1, scr.zorder[1]);
    CHECK_EQ(updates, expupdates);
    CHECK_EQ(scr.zorder.size(), 2);
    CHECK_EQ(scr.zorder[1], scr.focus());
    CHECK_EQ(string(dynamic_cast<Window *>(scr.zorder[1])->text()), "        \n        \n        \n        \n");
    SUBCASE("AddWindow below") {
      auto [win2, updates]{scr.addWindow(Rectangle{2, 0, 8, 6}, Space, win1)};
      auto [expwin, expupdates]{make_tuple(scr.zorder[1], Updates{{Vector{2, 0}, Text("      ")}, {Vector{2, 5}, Text("      ")}})};
      CHECK_EQ(win2, expwin);
      CHECK_EQ(updates, expupdates);
      CHECK_EQ(scr.zorder.size(), 3);
      CHECK_EQ(scr.zorder[1], scr.focus());
      CHECK_EQ(scr.zorder[1], win2);
      SUBCASE("DeleteWindow") {
        auto updates{scr.deleteWindow(win2)};
        auto expupdates{Updates{{Vector{2, 0}, Text("      ")}, {Vector{2, 5}, Text("      ")}}};
        CHECK_EQ(updates, expupdates);
        CHECK_EQ(scr.zorder.size(), 2);
        CHECK_EQ(scr.zorder[1], scr.focus());
        CHECK_EQ(scr.zorder[1], win1);
      }
      SUBCASE("ReshapeWindow") {
        auto updates{scr.reshapeWindow(win2, Rectangle{4, 0, 10, 6})};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{2, 0}, Text("  ")},
              {Vector{2, 5}, Text("  ")},
              {Vector{4, 0}, Text("      ")},
              {Vector{4, 5}, Text("      ")},
              {Vector{9, 1}, Text(" \n \n \n ")}
            }
          }
        );
      }
      SUBCASE("Fill") {
        auto updates{scr.fill(win1, Char('1'))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{1, 1}, Text("11111111\n11111111\n11111111\n11111111")}
            }
          }
        );
      }
      SUBCASE("Fill covered window") {
        auto updates{scr.fill(win2, Char('2'))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{2, 0}, Text("222222")},
              {Vector{2, 5}, Text("222222")},
            }
          }
        );
      }
      SUBCASE("Text") {
        auto updates{scr.text(win1, Vector{0, 0}, Text("aaa\nbbbb"))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{1, 1}, Text("aaa\nbbbb")}
            }
          }
        );
      }
      SUBCASE("Text covered window") {
        auto updates{scr.text(win2, Vector{0, 0}, Text("qqq\nqqq\nqqq\nqqq\nqqq\nwwww"))};
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{2, 0}, Text("qqq ")}, // space here because update is a rectangle
              {Vector{2, 5}, Text("wwww")}
            }
          }
        );
      }
      SUBCASE("Line") {
        auto updates{scr.line(win1, Line{Vector{0, 0}, 8, Horizontal, false, false}, 2)};
        CHECK_EQ(
          updates,
          Updates{{{Vector{1, 1}, Text("╺━━━━━━╸")}}}
        );
      }
      SUBCASE("Line in covered window") {
        auto updates{scr.line(win2, Line{Vector{0, 0}, 6, Vertical, true, true}, 2)};
        CHECK_EQ(
          updates,
          Updates{{{Vector{2, 0}, Text("┃")}, {Vector{2, 5}, Text("┃")}}}
        );
      }
      SUBCASE("Box") {
        auto updates{scr.box(win1, Box(Rectangle{0, 0, DimHigh, DimHigh}))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{1, 1}, Text("┌──────┐")},
              {Vector{1, 1}, Text("┌\n│\n│\n└")},
              {Vector{1, 4}, Text("└──────┘")},
              {Vector{8, 1}, Text("┐\n│\n│\n┘")},
            }
          }
        );
      }
      SUBCASE("Box in covered window") {
        auto updates{scr.box(win2, Box(Rectangle{0, 0, DimHigh, DimHigh}))};
        ranges::actions::sort(updates);
        CHECK_EQ(
          updates,
          Updates{
            {
              {Vector{2, 0}, Text("┌────┐")},
              {Vector{2, 0}, Text("┌")},
              {Vector{2, 5}, Text("└────┘")},
              {Vector{2, 5}, Text("└")},
              {Vector{7, 0}, Text("┐")},
              {Vector{7, 5}, Text("┘")}
            }
          }
        );
      }
    }
  }
}

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
