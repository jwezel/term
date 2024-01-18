#include "ui/widget.hh"
#include <ui/ui.hh>
#include <ui/window.hh>

#include <term/geometry.hh>
#include <term/term.hh>
#include <term/text.hh>
#include <term/window.hh>

#include <cstdio>
#include <doctest/doctest.h>

using
  jwezel::AttributeMode::mix,
  jwezel::Char,
  jwezel::clear,
  jwezel::Line,
  jwezel::Rectangle,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin,
  jwezel::Window,
  jwezel::operator""_C;

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

TEST_CASE("UI Window") {
  auto *output{tmpfile()};
  auto *input{tmpfile()};
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[1;1R", input);
  (void)fputs("\x1b[10;20R", input);
  (void)fseek(input, 0, SEEK_SET);
  Terminal term('.'_C, VectorMin, VectorMin, VectorMax, output->_fileno, input->_fileno);
  jwezel::ui::Ui ui_(&term);

  SUBCASE("Window") {
    auto w1{jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 10, 6}, Text{"Title"})};
    auto test{Text{' '_C, Vector{10, 6}}};
    test.box();
    test.patch(Text{"Title"}, Vector{1, 0});
    CHECK_EQ(w1.text(), test);
    CHECK_EQ(term.display().text(), test);
    SUBCASE("Widget") {
      auto w2{jwezel::ui::Widget(Rectangle{1, 1, 8, 2}, ' '_C, &w1)};
      w2.write(Vector{0, 0}, Text{"xxxxxxxxxxxxx"});
      auto test2{test};
      test2.patch(Text{"xxxxxxx"}, Vector{1, 1});
      CHECK_EQ(term.display().text(), test2);
      SUBCASE("Move widget") {
        w2.move(Rectangle{2, 1, 9, 2});
        auto test3{test};
        test3.patch(Text{"xxxxxxx"}, Vector{2, 1});
        CHECK_EQ(term.display().text(), test3);
      }
    }
    CHECK_EQ(term.display().text(), test);
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
