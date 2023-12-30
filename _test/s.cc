#include <algorithm>
#include <iostream>
#include <memory>
#include <string_view>

using std::string, std::cout;

struct O {
  explicit O(std::string_view s): s{s} {}
  O(O &&) = default;
  O(O &) = delete;
  O &operator =(O &&) = default;
  O &operator =(O &) = default;
  string s{""};
};

struct OC {
  explicit OC(std::string_view s): p{new O{s}} {}
  OC(OC &&) = default;
  OC(OC &) = delete;
  OC &operator =(OC &&) = default;
  OC &operator =(OC &) = default;
  std::shared_ptr<O> p;
};

int main(int /*argc*/, char const */*argv*/[])
{
  OC o{"shit"};
  OC o2{std::move(o)};
  cout << o.p.use_count() << ", " << o2.p.use_count() << "\n";
  return 0;
}
