#include "term/update.hh"
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
    auto w1 = jwezel::ui::Window(ui_, ' '_C, Rectangle{0, 0, 10, 6}, Text{"Title"});
    CHECK_EQ(w1.text(Rectangle{1, 0, 6, 1}), Text{"Title"});
    CHECK_EQ(term.display().text(), w1.text());
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
