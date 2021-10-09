///
/// @defgroup   STRING string
///
/// This file defines supplemental string functions.
///
/// @author     J
/// @date       2021-01-02
///

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>
#include <assert.h>
#include <exception>
#include <iostream>

#include <fmt/core.h>

#include "text.hh"
#include <range.hh>

namespace jwezel {

template<class charT>
class BasicSplitString {
  public:
  using value_type = basic_string<charT>;

  private:
  value_type subject;
  value_type sep;
  long pos;

  public:
  BasicSplitString(const value_type &subject, const value_type &sep, long pos=0):
  subject(subject),
  sep(sep),
  pos(pos)
  {}


  class iterator: public std::iterator<
    std::input_iterator_tag,  // iterator_category
    long,                     // value_type
    long,                     // difference_type
    long *,                   // pointer
    value_type &              // reference
  > {
    private:
    BasicSplitString &range;
    mutable size_t pos = 0;
    size_t next = 0;

    void advance() {
      if (pos == range.subject.npos) {
        return;
      }
      if (next <= pos) {
        next = range.subject.find(range.sep, pos);
      }
    }

    public:
    explicit iterator(BasicSplitString &range, long pos = 0):
    range(range),
    pos(pos),
    next(pos)
    {
      advance();
    }

    iterator& operator++() {
      if (next == range.subject.npos) {
        pos = next;
      } else {
        pos = min(next + range.sep.size(), range.subject.size());
        advance();
      }
      return *this;
    }

    iterator operator++(int) {
      iterator result = *this;
      ++(*this);
      return result;
    }

    bool operator==(iterator other) const {
      return pos == other.pos;
    }

    bool operator!=(iterator other) const {
      return !(*this == other);
    }

    basic_string_view<charT> operator*() const {
      if (pos == range.subject.npos) throw out_of_range(fmt::format("{:<64}", range.subject));
      assert(pos <= range.subject.size());
      assert(next >= 0);
      assert(pos >= 0);
      return basic_string_view<charT>(
        range.subject.data() + pos,
        (next == range.subject.npos? range.subject.size(): next) - pos
      );
    }
  };

  iterator begin() {
    return iterator(*this, pos);
  }

  iterator end() {
    return iterator(*this, subject.npos);
  }

};

using SplitString = BasicSplitString<char>;

}
