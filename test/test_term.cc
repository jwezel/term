#include <algorithm>
#include <sstream>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <geometry.hh>
#include <string.hh>

using namespace jwezel;

namespace doctest {
    template<> struct StringMaker<Vector> {
        static String convert(const Vector& value) {
            return string(value).c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<Rectangle> {
        static String convert(const Rectangle& value) {
            return string(value).c_str();
        }
    };
}

namespace doctest {
    template<> struct StringMaker<vector<Rectangle>> {
        static String convert(const vector<Rectangle>& v) {
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
}

TEST_CASE("SplitString") {
	SUBCASE("3-part") {
		string s{"a;b;c"};
		SplitString r(s, ";");
		SplitString::iterator i{r.begin()};
		CHECK_EQ(*i, "a");
		++i;
		CHECK_EQ(*i, "b");
		++i;
		CHECK_EQ(*i, "c");
		++i;
		CHECK_EQ(i, r.end());
	}
	SUBCASE("3-part, 1st empty") {
		string s{";b;c"};
		SplitString r(s, ";");
		SplitString::iterator i{r.begin()};
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(*i, "b");
		++i;
		CHECK_EQ(*i, "c");
		++i;
		CHECK_EQ(i, r.end());
	}
	SUBCASE("3-part, 2nd empty") {
		string s{"a;;c"};
		SplitString r(s, ";");
		SplitString::iterator i{r.begin()};
		CHECK_EQ(*i, "a");
		++i;
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(*i, "c");
		++i;
		CHECK_EQ(i, r.end());
	}
	SUBCASE("3-part, 3rd empty") {
		string s{"a;b;"};
		SplitString r(s, ";");
		SplitString::iterator i{r.begin()};
		CHECK_EQ(*i, "a");
		++i;
		CHECK_EQ(*i, "b");
		++i;
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(i, r.end());
	}
	SUBCASE("3-part, all empty") {
		string s{";;"};
		SplitString r(s, ";");
		SplitString::iterator i{r.begin()};
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(*i, "");
		++i;
		CHECK_EQ(i, r.end());
		CHECK_THROWS(*i);
	}
	SUBCASE("range for loop") {
		string c;
		for (string_view p: SplitString("a;b;c", ";")) {
			c += string(p) + ".";
		}
		CHECK_EQ(c, "a.b.c.");
	}
	SUBCASE("range for loop") {
		string c;
		for (string_view p: SplitString("a;b;c;", ";")) {
			c += string(p) + ".";
		}
		CHECK_EQ(c, "a.b.c..");
	}
}

TEST_CASE("Vector") {
	Vector v(20, 10);
	CHECK_EQ(v.x, 20);
	CHECK_EQ(v.y, 10);
	SUBCASE("String representation") {
		CHECK_EQ(string(v), "Vec(20, 10)");
	}
	SUBCASE("operator ==") {
		CHECK(v == Vector(20, 10));
	}
	SUBCASE("operator !=") {
		CHECK(v != Vector(30, 10));
	}
	SUBCASE("position()") {
		CHECK_EQ(v.position(Vector(2, 1)), Vector(2, 1));
		CHECK_EQ(v.position(Vector(-2, -2)), Vector(18, 8));
	}
	SUBCASE("defaultTo()") {
		CHECK_EQ(Vector(30, DimLow).defaultTo(v), Vector(30, 10));
		CHECK_EQ(Vector(DimLow, 20).defaultTo(v), Vector(20, 20));
		CHECK_EQ(Vector(DimLow, DimLow).defaultTo(v), Vector(20, 10));
	}
	SUBCASE("operator + Dim") {
		CHECK_EQ(v + 1, Vector(21, 11));
	}
	SUBCASE("operator - Dim") {
		CHECK_EQ(v - 1, Vector(19, 9));
	}
	SUBCASE("operator + Vector") {
		CHECK_EQ(v + Vector(2, 4), Vector(22, 14));
	}
	SUBCASE("operator - Vector") {
		CHECK_EQ(v - Vector(2, 4), Vector(18, 6));
	}
	SUBCASE("left()") {
		CHECK_EQ(v.left(2), Vector(18, 10));
	}
	SUBCASE("right()") {
		CHECK_EQ(v.right(2), Vector(22, 10));
	}
	SUBCASE("up()") {
		CHECK_EQ(v.up(2), Vector(20, 8));
	}
	SUBCASE("down()") {
		CHECK_EQ(v.down(2), Vector(20, 12));
	}
	SUBCASE("operator | Vector") {
		CHECK_EQ(v | Vector(2, 4), Vector(20, 10));
		CHECK_EQ(v | Vector(30, 4), Vector(30, 10));
		CHECK_EQ(v | Vector(10, 14), Vector(20, 14));
		CHECK_EQ(v | Vector(30, 24), Vector(30, 24));
	}
	SUBCASE("center()") {
		CHECK_EQ(centered(v, v + 2), Rectangle(1, 1, 21, 11));
	}
}

TEST_CASE("Rectangle") {
	Rectangle r(10, 10, 30, 30);
	CHECK_EQ(r.x1, 10);
	CHECK_EQ(r.y1, 10);
	CHECK_EQ(r.x2, 30);
	CHECK_EQ(r.y2, 30);

	SUBCASE("operator string") {
		CHECK_EQ(string(r), "Rec(10, 10, 30, 30)");
	}
	SUBCASE("operator ==") {
		CHECK(r == Rectangle(10, 10, 30, 30));
	}
	SUBCASE("operator !=") {
		CHECK(r != Rectangle(11, 10, 30, 30));
		CHECK(r != Rectangle(10, 11, 30, 30));
		CHECK(r != Rectangle(10, 10, 31, 30));
		CHECK(r != Rectangle(10, 10, 30, 31));
		CHECK(r != Rectangle(11, 11, 31, 31));
	}
	SUBCASE("operator &") {
		CHECK_EQ((r & Rectangle(12, 12, 33, 33)).value(), Rectangle(12, 12, 30, 30));
		CHECK_EQ((r & Rectangle(8, 8, 16, 16)).value(), Rectangle(10, 10, 16, 16));
		CHECK_EQ((r & Rectangle(8, 8, 33, 33)).value(), Rectangle(10, 10, 30, 30));
		CHECK_EQ((r & Rectangle(12, 12, 16, 16)).value(), Rectangle(12, 12, 16, 16));
		CHECK_EQ((r & Rectangle(10, 10, 30, 30)).value(), Rectangle(10, 10, 30, 30));
		CHECK_FALSE((r & Rectangle(30, 30, 40, 40)));
	}
	SUBCASE("intersects()") {
		CHECK(r.intersects(Rectangle(12, 12, 33, 33)));
		CHECK(r.intersects(Rectangle(8, 8, 16, 16)));
		CHECK(r.intersects(Rectangle(8, 8, 33, 33)));
		CHECK(r.intersects(Rectangle(12, 12, 16, 16)));
		CHECK(r.intersects(Rectangle(10, 10, 30, 30)));
		CHECK_FALSE(r.intersects(Rectangle(30, 30, 40, 40)));
	}
	SUBCASE("operator + Vector") {
		CHECK_EQ(r + Vector(2, 2), Rectangle(12, 12, 32, 32));
	}
	SUBCASE("operator - Vector") {
		CHECK_EQ(r - Vector(2, 2), Rectangle(8, 8, 28, 28));
	}
	SUBCASE("operator | Rectangle") {
		CHECK_EQ(r | Rectangle(10, 10, 30, 30), r);
		CHECK_EQ(r | Rectangle(10, 10, 32, 32), Rectangle(10, 10, 32, 32));
		CHECK_EQ(r | Rectangle(8, 8, 32, 32), Rectangle(8, 8, 32, 32));
		CHECK_EQ(r | Rectangle(12, 12, 32, 32), Rectangle(10, 10, 32, 32));
		CHECK_EQ(r | Rectangle(12, 12, 20, 20), Rectangle(10, 10, 30, 30));
		CHECK_EQ(r | Rectangle(40, 40, 60, 60), Rectangle(10, 10, 60, 60));
	}
	SUBCASE("operator |= Rectangle") {
		CHECK_EQ(r |= Rectangle(10, 10, 30, 30), r);
		CHECK_EQ(r |= Rectangle(10, 10, 32, 32), Rectangle(10, 10, 32, 32));
		CHECK_EQ(r |= Rectangle(8, 8, 32, 32), Rectangle(8, 8, 32, 32));
		CHECK_EQ(r |= Rectangle(12, 12, 32, 32), Rectangle(8, 8, 32, 32));
		CHECK_EQ(r |= Rectangle(12, 12, 20, 20), Rectangle(8, 8, 32, 32));
		CHECK_EQ(r |= Rectangle(40, 40, 60, 60), Rectangle(8, 8, 60, 60));
	}
	SUBCASE ("operator - Rectangle") {
		CHECK_EQ(
			r - Rectangle(8, 8, 12, 12),
			vector<Rectangle>({
				Rectangle(12, 10, 30, 12),
				Rectangle(10, 12, 30, 30)
			})
		);
		CHECK_EQ(
			r - Rectangle(12, 12, 16, 16),
			vector<Rectangle>({
				Rectangle(10, 10, 30, 12),
				Rectangle(10, 12, 12, 16),
				Rectangle(16, 12, 30, 16),
				Rectangle(10, 16, 30, 30),
			})
		);
		CHECK_EQ(
			r - Rectangle(28, 16, 32, 20),
			vector<Rectangle>({
				Rectangle(10, 10, 30, 16),
				Rectangle(10, 16, 28, 20),
				Rectangle(10, 20, 30, 30),
			})
		);
		CHECK_EQ(
			r - Rectangle(30, 30, 40, 40),
			vector<Rectangle>()
		);
		CHECK_EQ(
			r - Rectangle(8, 8, 40, 40),
			vector<Rectangle>()
		);
	}
	SUBCASE ("defaultIntersection()") {
		CHECK_EQ(
			r.defaultIntersection(Rectangle(8, 8, 12, 12)),
			vector<Rectangle>({
				Rectangle(12, 10, 30, 12),
				Rectangle(10, 12, 30, 30)
			})
		);
		CHECK_EQ(
			r.defaultIntersection(Rectangle(12, 12, 16, 16)),
			vector<Rectangle>({
				Rectangle(10, 10, 30, 12),
				Rectangle(10, 12, 12, 16),
				Rectangle(16, 12, 30, 16),
				Rectangle(10, 16, 30, 30),
			})
		);
		CHECK_EQ(
			r.defaultIntersection(Rectangle(28, 16, 32, 20)),
			vector<Rectangle>({
				Rectangle(10, 10, 30, 16),
				Rectangle(10, 16, 28, 20),
				Rectangle(10, 20, 30, 30),
			})
		);
		CHECK_EQ(
			r.defaultIntersection(Rectangle(30, 30, 40, 40)),
			vector<Rectangle>({r})
		);
		CHECK_EQ(
			r - Rectangle(8, 8, 40, 40),
			vector<Rectangle>()
		);
	}
	SUBCASE("isJoinedX()") {
		CHECK(r.isJoinedX(Rectangle(8, 10, 10, 30)));
		CHECK(r.isJoinedX(Rectangle(30, 10, 40, 30)));
		CHECK_FALSE(r.isJoinedX(Rectangle(8, 11, 10, 30)));
		CHECK_FALSE(r.isJoinedX(Rectangle(30, 11, 40, 30)));
	}
	SUBCASE("isJoinedY()") {
		CHECK(r.isJoinedY(Rectangle(10, 8, 30, 10)));
		CHECK(r.isJoinedY(Rectangle(10, 30, 30, 40)));
		CHECK_FALSE(r.isJoinedY(Rectangle(11, 8, 30, 10)));
		CHECK_FALSE(r.isJoinedY(Rectangle(11, 30, 30, 40)));
	}
	SUBCASE("isJoined()") {
		CHECK(r.isJoined(Rectangle(8, 10, 10, 30)));
		CHECK(r.isJoined(Rectangle(30, 10, 40, 30)));
		CHECK_FALSE(r.isJoined(Rectangle(8, 11, 10, 30)));
		CHECK_FALSE(r.isJoined(Rectangle(30, 11, 40, 30)));
		CHECK(r.isJoined(Rectangle(10, 8, 30, 10)));
		CHECK(r.isJoined(Rectangle(10, 30, 30, 40)));
		CHECK_FALSE(r.isJoined(Rectangle(11, 8, 30, 10)));
		CHECK_FALSE(r.isJoined(Rectangle(11, 30, 30, 40)));
	}
	SUBCASE("joinedWith()") {
		CHECK_EQ(r.joinedWith(Rectangle(8, 10, 10, 30)), Rectangle(8, 10, 30, 30));
		CHECK_EQ(r.joinedWith(Rectangle(30, 10, 40, 30)), Rectangle(10, 10, 40, 30));
		CHECK_FALSE(r.joinedWith(Rectangle(8, 11, 10, 30)));
		CHECK_FALSE(r.joinedWith(Rectangle(30, 11, 40, 30)));
		CHECK_EQ(r.joinedWith(Rectangle(10, 8, 30, 10)), Rectangle(10, 8, 30, 30));
		CHECK_EQ(r.joinedWith(Rectangle(10, 30, 30, 40)), Rectangle(10, 10, 30, 40));
		CHECK_FALSE(r.joinedWith(Rectangle(11, 8, 30, 10)));
		CHECK_FALSE(r.joinedWith(Rectangle(11, 30, 30, 40)));
	}
	SUBCASE("height()") {
		CHECK_EQ(r.height(), 20);
	}
	SUBCASE("width()") {
		CHECK_EQ(r.width(), 20);
	}
	SUBCASE("size()") {
		CHECK_EQ(r.size(), Vector(20, 20));
	}
	SUBCASE("position()") {
		CHECK_EQ(r.position(), Vector(10, 10));
	}
	SUBCASE("operator - Dim") {
		CHECK_EQ(r - 1, Rectangle(11, 11, 29, 29));
	}
	SUBCASE("operator + Dim") {
		CHECK_EQ(r + 1, Rectangle(9, 9, 31, 31));
	}
	SUBCASE("defaultTo()") {
		CHECK_EQ(Rectangle(DimLow, 10, 30, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, DimLow, 30, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, 10, DimLow, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, 10, 30, DimLow).defaultTo(r), r);
		CHECK_EQ(Rectangle(DimLow, DimLow, DimLow, DimLow).defaultTo(r), r);
		CHECK_EQ(Rectangle(DimHigh, 10, 30, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, DimHigh, 30, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, 10, DimHigh, 30).defaultTo(r), r);
		CHECK_EQ(Rectangle(10, 10, 30, DimHigh).defaultTo(r), r);
		CHECK_EQ(Rectangle(DimHigh, DimHigh, DimHigh, DimHigh).defaultTo(r), r);
	}
}
TEST_CASE("diff()") {
	CHECK_EQ(diff(10, 20), 10);
	CHECK_EQ(diff(20, 10), 10);
}
