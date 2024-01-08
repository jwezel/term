#include <term/geometry.hh>
#include <util/string.hh>

#include <algorithm>
#include <sstream>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using
  jwezel::Vector,
  jwezel::Rectangle,
  jwezel::string,
  jwezel::diff,
  jwezel::DimLow,
  jwezel::DimHigh
  ;
using std::vector, std::stringstream;

namespace doctest {
  template<> struct StringMaker<Vector> {
    static auto convert(const Vector& value) -> String {
      return string(value).c_str();
    }
  };
} // namespace doctest

namespace doctest {
  template<> struct StringMaker<Rectangle> {
    static auto convert(const Rectangle& value) -> String {
      return string(value).c_str();
    }
  };
} // namespace doctest

namespace doctest {
  template<> struct StringMaker<vector<Rectangle>> {
    static auto convert(const vector<Rectangle>& v) -> String {
      stringstream result;
      const char *sep = "";
      result << "[";
      for_each(
        v.begin(),
        v.end(),
        [&](const Rectangle &r) {
          result << sep << r;
          sep = ", ";
        }
      );
      result << "]";
      return result.str().c_str();
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

TEST_CASE("Vector") {
  Vector vec(20, 10);
  CHECK_EQ(vec.x(), 20);
  CHECK_EQ(vec.y(), 10);
  SUBCASE("String representation") {
    CHECK_EQ(string(vec), "Vec(20, 10)");
  }
  SUBCASE("operator ==") {
    CHECK(vec == Vector(20, 10));
  }
  SUBCASE("operator !=") {
    CHECK(vec != Vector(30, 10));
  }
  SUBCASE("position()") {
    CHECK_EQ(vec.position(Vector(2, 1)), Vector(2, 1));
    CHECK_EQ(vec.position(Vector(-2, -2)), Vector(18, 8));
  }
  SUBCASE("defaultTo()") {
    CHECK_EQ(Vector(30, DimLow).defaultTo(vec), Vector(30, 10));
    CHECK_EQ(Vector(DimLow, 20).defaultTo(vec), Vector(20, 20));
    CHECK_EQ(Vector(DimLow, DimLow).defaultTo(vec), Vector(20, 10));
  }
  SUBCASE("operator + Dim") {
    CHECK_EQ(vec + 1, Vector(21, 11));
  }
  SUBCASE("operator - Dim") {
    CHECK_EQ(vec - 1, Vector(19, 9));
  }
  SUBCASE("operator + Vector") {
    CHECK_EQ(vec + Vector(2, 4), Vector(22, 14));
  }
  SUBCASE("operator - Vector") {
    CHECK_EQ(vec - Vector(2, 4), Vector(18, 6));
  }
  SUBCASE("left()") {
    CHECK_EQ(vec.left(2), Vector(18, 10));
  }
  SUBCASE("right()") {
    CHECK_EQ(vec.right(2), Vector(22, 10));
  }
  SUBCASE("up()") {
    CHECK_EQ(vec.up(2), Vector(20, 8));
  }
  SUBCASE("down()") {
    CHECK_EQ(vec.down(2), Vector(20, 12));
  }
  SUBCASE("operator | Vector") {
    CHECK_EQ(vec | Vector(2, 4), Vector(20, 10));
    CHECK_EQ(vec | Vector(30, 4), Vector(30, 10));
    CHECK_EQ(vec | Vector(10, 14), Vector(20, 14));
    CHECK_EQ(vec | Vector(30, 24), Vector(30, 24));
  }
  SUBCASE("center()") {
    CHECK_EQ(centered(vec, vec + 2), Rectangle(1, 1, 21, 11));
  }
}

TEST_CASE("Rectangle") {
  Rectangle rec(10, 10, 30, 30);
  CHECK_EQ(rec.x1(), 10);
  CHECK_EQ(rec.y1(), 10);
  CHECK_EQ(rec.x2(), 30);
  CHECK_EQ(rec.y2(), 30);

  SUBCASE("operator string") {
    CHECK_EQ(string(rec), "Rec(10, 10, 30, 30)");
  }
  SUBCASE("operator ==") {
    CHECK(rec == Rectangle(10, 10, 30, 30));
  }
  SUBCASE("operator !=") {
    CHECK(rec != Rectangle(11, 10, 30, 30));
    CHECK(rec != Rectangle(10, 11, 30, 30));
    CHECK(rec != Rectangle(10, 10, 31, 30));
    CHECK(rec != Rectangle(10, 10, 30, 31));
    CHECK(rec != Rectangle(11, 11, 31, 31));
  }
  SUBCASE("operator &") {
    CHECK_EQ((rec & Rectangle(12, 12, 33, 33)).value(), Rectangle(12, 12, 30, 30));
    CHECK_EQ((rec & Rectangle(8, 8, 16, 16)).value(), Rectangle(10, 10, 16, 16));
    CHECK_EQ((rec & Rectangle(8, 8, 33, 33)).value(), Rectangle(10, 10, 30, 30));
    CHECK_EQ((rec & Rectangle(12, 12, 16, 16)).value(), Rectangle(12, 12, 16, 16));
    CHECK_EQ((rec & Rectangle(10, 10, 30, 30)).value(), Rectangle(10, 10, 30, 30));
    CHECK_FALSE((rec & Rectangle(30, 30, 40, 40)));
  }
  SUBCASE("intersects()") {
    CHECK(rec.intersects(Rectangle(12, 12, 33, 33)));
    CHECK(rec.intersects(Rectangle(8, 8, 16, 16)));
    CHECK(rec.intersects(Rectangle(8, 8, 33, 33)));
    CHECK(rec.intersects(Rectangle(12, 12, 16, 16)));
    CHECK(rec.intersects(Rectangle(10, 10, 30, 30)));
    CHECK_FALSE(rec.intersects(Rectangle(30, 30, 40, 40)));
  }
  SUBCASE("operator + Vector") {
    CHECK_EQ(rec + Vector(2, 2), Rectangle(12, 12, 32, 32));
  }
  SUBCASE("operator - Vector") {
    CHECK_EQ(rec - Vector(2, 2), Rectangle(8, 8, 28, 28));
  }
  SUBCASE("operator | Rectangle") {
    CHECK_EQ(rec | Rectangle(10, 10, 30, 30), rec);
    CHECK_EQ(rec | Rectangle(10, 10, 32, 32), Rectangle(10, 10, 32, 32));
    CHECK_EQ(rec | Rectangle(8, 8, 32, 32), Rectangle(8, 8, 32, 32));
    CHECK_EQ(rec | Rectangle(12, 12, 32, 32), Rectangle(10, 10, 32, 32));
    CHECK_EQ(rec | Rectangle(12, 12, 20, 20), Rectangle(10, 10, 30, 30));
    CHECK_EQ(rec | Rectangle(40, 40, 60, 60), Rectangle(10, 10, 60, 60));
  }
  SUBCASE("operator |= Rectangle") {
    CHECK_EQ(rec |= Rectangle(10, 10, 30, 30), rec);
    CHECK_EQ(rec |= Rectangle(10, 10, 32, 32), Rectangle(10, 10, 32, 32));
    CHECK_EQ(rec |= Rectangle(8, 8, 32, 32), Rectangle(8, 8, 32, 32));
    CHECK_EQ(rec |= Rectangle(12, 12, 32, 32), Rectangle(8, 8, 32, 32));
    CHECK_EQ(rec |= Rectangle(12, 12, 20, 20), Rectangle(8, 8, 32, 32));
    CHECK_EQ(rec |= Rectangle(40, 40, 60, 60), Rectangle(8, 8, 60, 60));
  }
  SUBCASE ("operator - Rectangle") {
    CHECK_EQ(
      rec - Rectangle(8, 8, 12, 12),
      vector<Rectangle>({
        Rectangle(12, 10, 30, 12),
        Rectangle(10, 12, 30, 30)
      })
    );
    CHECK_EQ(
      rec - Rectangle(12, 12, 16, 16),
      vector<Rectangle>({
        Rectangle(10, 10, 30, 12),
        Rectangle(10, 12, 12, 16),
        Rectangle(16, 12, 30, 16),
        Rectangle(10, 16, 30, 30),
      })
    );
    CHECK_EQ(
      rec - Rectangle(28, 16, 32, 20),
      vector<Rectangle>({
        Rectangle(10, 10, 30, 16),
        Rectangle(10, 16, 28, 20),
        Rectangle(10, 20, 30, 30),
      })
    );
    CHECK_EQ(
      rec - Rectangle(30, 30, 40, 40),
      vector<Rectangle>()
    );
    CHECK_EQ(
      rec - Rectangle(8, 8, 40, 40),
      vector<Rectangle>()
    );
  }
  SUBCASE ("defaultIntersection()") {
    CHECK_EQ(
      rec.defaultIntersection(Rectangle(8, 8, 12, 12)),
      vector<Rectangle>({
        Rectangle(12, 10, 30, 12),
        Rectangle(10, 12, 30, 30)
      })
    );
    CHECK_EQ(
      rec.defaultIntersection(Rectangle(12, 12, 16, 16)),
      vector<Rectangle>({
        Rectangle(10, 10, 30, 12),
        Rectangle(10, 12, 12, 16),
        Rectangle(16, 12, 30, 16),
        Rectangle(10, 16, 30, 30),
      })
    );
    CHECK_EQ(
      rec.defaultIntersection(Rectangle(28, 16, 32, 20)),
      vector<Rectangle>({
        Rectangle(10, 10, 30, 16),
        Rectangle(10, 16, 28, 20),
        Rectangle(10, 20, 30, 30),
      })
    );
    CHECK_EQ(
      rec.defaultIntersection(Rectangle(30, 30, 40, 40)),
      vector<Rectangle>({rec})
    );
    CHECK_EQ(
      rec - Rectangle(8, 8, 40, 40),
      vector<Rectangle>()
    );
  }
  SUBCASE("isJoinedX()") {
    CHECK(rec.isJoinedX(Rectangle(8, 10, 10, 30)));
    CHECK(rec.isJoinedX(Rectangle(30, 10, 40, 30)));
    CHECK_FALSE(rec.isJoinedX(Rectangle(8, 11, 10, 30)));
    CHECK_FALSE(rec.isJoinedX(Rectangle(30, 11, 40, 30)));
  }
  SUBCASE("isJoinedY()") {
    CHECK(rec.isJoinedY(Rectangle(10, 8, 30, 10)));
    CHECK(rec.isJoinedY(Rectangle(10, 30, 30, 40)));
    CHECK_FALSE(rec.isJoinedY(Rectangle(11, 8, 30, 10)));
    CHECK_FALSE(rec.isJoinedY(Rectangle(11, 30, 30, 40)));
  }
  SUBCASE("isJoined()") {
    CHECK(rec.isJoined(Rectangle(8, 10, 10, 30)));
    CHECK(rec.isJoined(Rectangle(30, 10, 40, 30)));
    CHECK_FALSE(rec.isJoined(Rectangle(8, 11, 10, 30)));
    CHECK_FALSE(rec.isJoined(Rectangle(30, 11, 40, 30)));
    CHECK(rec.isJoined(Rectangle(10, 8, 30, 10)));
    CHECK(rec.isJoined(Rectangle(10, 30, 30, 40)));
    CHECK_FALSE(rec.isJoined(Rectangle(11, 8, 30, 10)));
    CHECK_FALSE(rec.isJoined(Rectangle(11, 30, 30, 40)));
  }
  SUBCASE("joinedWith()") {
    CHECK_EQ(rec.joinedWith(Rectangle(8, 10, 10, 30)), Rectangle(8, 10, 30, 30));
    CHECK_EQ(rec.joinedWith(Rectangle(30, 10, 40, 30)), Rectangle(10, 10, 40, 30));
    CHECK_FALSE(rec.joinedWith(Rectangle(8, 11, 10, 30)));
    CHECK_FALSE(rec.joinedWith(Rectangle(30, 11, 40, 30)));
    CHECK_EQ(rec.joinedWith(Rectangle(10, 8, 30, 10)), Rectangle(10, 8, 30, 30));
    CHECK_EQ(rec.joinedWith(Rectangle(10, 30, 30, 40)), Rectangle(10, 10, 30, 40));
    CHECK_FALSE(rec.joinedWith(Rectangle(11, 8, 30, 10)));
    CHECK_FALSE(rec.joinedWith(Rectangle(11, 30, 30, 40)));
  }
  SUBCASE("height()") {
    CHECK_EQ(rec.height(), 20);
  }
  SUBCASE("width()") {
    CHECK_EQ(rec.width(), 20);
  }
  SUBCASE("size()") {
    CHECK_EQ(rec.size(), Vector(20, 20));
  }
  SUBCASE("position()") {
    CHECK_EQ(rec.position(), Vector(10, 10));
  }
  SUBCASE("operator - Dim") {
    CHECK_EQ(rec - 1, Rectangle(11, 11, 29, 29));
  }
  SUBCASE("operator + Dim") {
    CHECK_EQ(rec + 1, Rectangle(9, 9, 31, 31));
  }
  SUBCASE("defaultTo()") {
    CHECK_EQ(Rectangle(DimLow, 10, 30, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, DimLow, 30, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, 10, DimLow, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, 10, 30, DimLow).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(DimLow, DimLow, DimLow, DimLow).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(DimHigh, 10, 30, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, DimHigh, 30, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, 10, DimHigh, 30).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(10, 10, 30, DimHigh).defaultTo(rec), rec);
    CHECK_EQ(Rectangle(DimHigh, DimHigh, DimHigh, DimHigh).defaultTo(rec), rec);
  }
}
TEST_CASE("diff()") {
  CHECK_EQ(diff(10, 20), 10);
  CHECK_EQ(diff(20, 10), 10);
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
