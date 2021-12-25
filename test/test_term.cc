#include <cstdio>
#include <doctest/doctest.h>

#include "geometry.hh"
#include "term.hh"

using namespace jwezel;

namespace doctest {
    template<> struct StringMaker<Vector> {
        static String convert(const Vector& value) {
            return string(value).c_str();
        }
    };
}

TEST_CASE("Terminal") {
  auto output{tmpfile()};
  auto input{tmpfile()};
  fputs("\x1b[1;1R", input);
  fputs("\x1b[1;1R", input);
  fputs("\x1b[10;20R", input);
  fseek(input, 0, SEEK_SET);
  Terminal t('.', VectorMin, VectorMin, VectorMax, output->_fileno, input->_fileno);
  SUBCASE("Window") {
    t.newWindow(Rectangle{0, 0, 10, 4}, '1');
    CHECK_EQ(string(t.display.size()), string(Vector{10, 4}));
    CHECK_EQ(string(t.desktop->text().size()), string(Vector{10, 4}));
    CHECK_EQ(t.display.text_.repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr());
    SUBCASE("Another window") {
      auto w2{t.newWindow(Rectangle{2, 2, 8, 6}, '2')};
      CHECK_EQ(string(t.display.size()), string(Vector{10, 6}));
      CHECK_EQ(string(t.desktop->text().size()), string(Vector{10, 6}));
      CHECK_EQ(t.display.text_.repr(), Text("1111111111\n1111111111\n1122222211\n1122222211\n..222222..\n..222222..").repr());
      SUBCASE("Delete window 2") {
        t.deleteWindow(w2);
        CHECK_EQ(t.display.text_.repr(), Text("1111111111\n1111111111\n1111111111\n1111111111").repr()); // display contracted
      }
      SUBCASE("Move window 2") {
        t.moveWindow(w2, Rectangle{2, 2, 12, 7});
        CHECK_EQ(
          t.display.text_.repr(),
          Text("1111111111..\n1111111111..\n112222222222\n112222222222\n..2222222222\n..2222222222\n..2222222222").repr()
        );
      }
    }
  }
}
