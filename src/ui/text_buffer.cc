#include "text_buffer.hh"

#include <format>
#include <iterator>
#include <stdexcept>

#include <utf8cpp/utf8.h>

namespace jwezel::ui {

namespace {

constexpr char32_t LineFeed{U'\n'};
constexpr char32_t CarriageReturn{U'\r'};

}

TextBuffer::TextBuffer(std::string_view text) {
  this->text(text);
}

void TextBuffer::text(std::string_view text) {
  std::string valid;
  utf8::replace_invalid(text.begin(), text.end(), std::back_inserter(valid));
  const auto characters{utf8::utf8to32(valid)};
  lines_.assign(1, std::u32string{});
  for (const auto character: characters) {
    if (character == LineFeed) {
      if (!lines_.back().empty() and lines_.back().back() == CarriageReturn) {
        lines_.back().pop_back();
      }
      lines_.emplace_back();
    } else {
      lines_.back().push_back(character);
    }
  }
}

auto TextBuffer::text() const -> std::string {
  std::string result;
  for (const auto &line: lines_) {
    if (&line != &lines_.front()) {
      result += '\n';
    }
    result += utf8::utf32to8(line);
  }
  return result;
}

auto TextBuffer::line(Dim line) const -> const std::u32string & {
  if (line < 0 or static_cast<size_t>(line) >= lines_.size()) {
    throw std::out_of_range(std::format("There is no line {}, there are {}", line, lines_.size()));
  }
  return lines_[static_cast<size_t>(line)];
}

auto TextBuffer::end() const -> Vector {
  return Vector{toDim(lines_.back().size()), toDim(lines_.size() - 1)};
}

auto TextBuffer::contains(const Vector &position) const -> bool {
  return
    position.y() >= 0 and static_cast<size_t>(position.y()) < lines_.size() and
    position.x() >= 0 and static_cast<size_t>(position.x()) <= lines_[static_cast<size_t>(position.y())].size();
}

void TextBuffer::insert(const Vector &position, Unicode character) {
  if (!contains(position)) {
    throw std::out_of_range(std::format("{} is not in the text", string(position)));
  }
  auto &line{lines_[static_cast<size_t>(position.y())]};
  line.insert(line.begin() + position.x(), character);
}

void TextBuffer::split(const Vector &position) {
  if (!contains(position)) {
    throw std::out_of_range(std::format("{} is not in the text", string(position)));
  }
  auto &line{lines_[static_cast<size_t>(position.y())]};
  std::u32string tail{line.begin() + position.x(), line.end()};
  line.erase(line.begin() + position.x(), line.end());
  lines_.insert(lines_.begin() + position.y() + 1, std::move(tail));
}

void TextBuffer::erase(const Vector &position) {
  if (!contains(position)) {
    throw std::out_of_range(std::format("{} is not in the text", string(position)));
  }
  auto &line{lines_[static_cast<size_t>(position.y())]};
  if (static_cast<size_t>(position.x()) < line.size()) {
    line.erase(line.begin() + position.x());
  } else if (static_cast<size_t>(position.y()) + 1 < lines_.size()) {
    line += lines_[static_cast<size_t>(position.y()) + 1];
    lines_.erase(lines_.begin() + position.y() + 1);
  }
}

auto TextBuffer::eraseBefore(const Vector &position) -> Vector {
  if (!contains(position)) {
    throw std::out_of_range(std::format("{} is not in the text", string(position)));
  }
  if (position.x() > 0) {
    auto &line{lines_[static_cast<size_t>(position.y())]};
    line.erase(line.begin() + position.x() - 1);
    return Vector{toDim(position.x() - 1), position.y()};
  }
  if (position.y() > 0) {
    auto &previous{lines_[static_cast<size_t>(position.y()) - 1]};
    const auto length{toDim(previous.size())};
    previous += lines_[static_cast<size_t>(position.y())];
    lines_.erase(lines_.begin() + position.y());
    return Vector{length, toDim(position.y() - 1)};
  }
  return position;
}

} // namespace jwezel::ui
