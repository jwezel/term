#include "text_input.hh"

#include <algorithm>

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

TextInput::TextInput(Container &parent, const Rectangle &area, const Text &content, const Char &background):
Widget{area, background, parent},
content_{content}
{
  if (content_.data.empty()) {
    content_.data.emplace_back();
  }
  cursor_.x() = lineWidth(0);
  scrollIntoView();
  render();
  registerHandlers();
}

TextInput::TextInput(Container &parent, const Text &content, const Char &background):
Widget{RectangleDefault, background, parent},
content_{content}
{
  if (content_.data.empty()) {
    content_.data.emplace_back();
  }
  cursor_.x() = lineWidth(0);
  scrollIntoView();
  render();
  registerHandlers();
}

void TextInput::registerHandlers() {
  onKey([this](const ui::Element &, const Event &event) {return handleKey(event);});
}

auto TextInput::handleKey(const Event &event) -> bool {
  auto *keyEvent{dynamic_cast<KeyEvent *>(event())};
  const auto key{keyEvent->key()};
  switch (key) {
    case Key::Left:
    moveLeft();
    break;

    case Key::Right:
    moveRight();
    break;

    case Key::Up:
    moveUp();
    break;

    case Key::Down:
    moveDown();
    break;

    case Key::Home:
    cursor_.x() = 0;
    break;

    case Key::End:
    cursor_.x() = lineWidth(cursor_.y());
    break;

    case Key::Backspace:
    backspace();
    break;

    case Key::Delete:
    del();
    break;

    case Key::Enter:
    if (acceptsEnter()) {
      insertNewline();
    } else {
      accept(event);
    }
    break;

    default:
    if (key >= ' ' and key < Key::None) {
      insertChar(key);
    } else {
      return false;
    }
  }
  scrollIntoView();
  render();
  return true;
}

void TextInput::insertChar(Unicode key) {
  auto &row{content_.data[cursor_.y()]};
  row.insert(row.begin() + cursor_.x(), Char{key});
  ++cursor_.x();
}

void TextInput::insertNewline() {
  auto &row{content_.data[cursor_.y()]};
  String tail{row.begin() + cursor_.x(), row.end()};
  row.erase(row.begin() + cursor_.x(), row.end());
  content_.data.insert(content_.data.begin() + cursor_.y() + 1, tail);
  ++cursor_.y();
  cursor_.x() = 0;
}

void TextInput::backspace() {
  if (cursor_.x() > 0) {
    auto &row{content_.data[cursor_.y()]};
    row.erase(row.begin() + cursor_.x() - 1);
    --cursor_.x();
  } else if (cursor_.y() > 0) {
    auto &previous{content_.data[cursor_.y() - 1]};
    const auto previousWidth{toDim(previous.size())};
    previous.insert(previous.end(), content_.data[cursor_.y()].begin(), content_.data[cursor_.y()].end());
    content_.data.erase(content_.data.begin() + cursor_.y());
    --cursor_.y();
    cursor_.x() = previousWidth;
  }
}

void TextInput::del() {
  auto &row{content_.data[cursor_.y()]};
  if (cursor_.x() < toDim(row.size())) {
    row.erase(row.begin() + cursor_.x());
  } else if (cursor_.y() + 1 < toDim(content_.data.size())) {
    row.insert(row.end(), content_.data[cursor_.y() + 1].begin(), content_.data[cursor_.y() + 1].end());
    content_.data.erase(content_.data.begin() + cursor_.y() + 1);
  }
}

void TextInput::moveLeft() {
  if (cursor_.x() > 0) {
    --cursor_.x();
  } else if (cursor_.y() > 0) {
    --cursor_.y();
    cursor_.x() = lineWidth(cursor_.y());
  }
}

void TextInput::moveRight() {
  if (cursor_.x() < lineWidth(cursor_.y())) {
    ++cursor_.x();
  } else if (cursor_.y() + 1 < toDim(content_.data.size())) {
    ++cursor_.y();
    cursor_.x() = 0;
  }
}

void TextInput::moveUp() {
  if (cursor_.y() > 0) {
    --cursor_.y();
    cursor_.x() = std::min(cursor_.x(), lineWidth(cursor_.y()));
  }
}

void TextInput::moveDown() {
  if (cursor_.y() + 1 < toDim(content_.data.size())) {
    ++cursor_.y();
    cursor_.x() = std::min(cursor_.x(), lineWidth(cursor_.y()));
  }
}

void TextInput::accept(const Event &event) {
  for (const auto &handler : acceptHandlers_) {
    handler(*this, event);
  }
}

auto TextInput::onAccept(const Handler &handler) -> int {
  acceptHandlers_.push_back(handler);
  return static_cast<int>(acceptHandlers_.size() - 1);
}

void TextInput::scrollIntoView() {
  const auto size{area().size()};
  if (cursor_.x() < scrollX_) {
    scrollX_ = cursor_.x();
  }
  if (cursor_.x() >= toDim(scrollX_ + size.x())) {
    scrollX_ = toDim(cursor_.x() - size.x() + 1);
  }
  if (cursor_.y() < scrollY_) {
    scrollY_ = cursor_.y();
  }
  if (cursor_.y() >= toDim(scrollY_ + size.y())) {
    scrollY_ = toDim(cursor_.y() - size.y() + 1);
  }
}

auto TextInput::lineWidth(Dim line) const -> Dim {
  return toDim(content_.data[static_cast<size_t>(line)].size());
}

void TextInput::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0) {
    return;
  }
  Text visible{background(), size};
  const auto width{std::min(size.x(), toDim(content_.width() - scrollX_))};
  const auto height{std::min(size.y(), toDim(content_.height() - scrollY_))};
  for (Dim row = 0; row < height; ++row) {
    const auto visibleWidth{std::min(width, toDim(content_.data[static_cast<size_t>(scrollY_ + row)].size() - scrollX_))};
    if (visibleWidth > 0) {
      visible.patch(
        content_[Rectangle{scrollX_, toDim(scrollY_ + row), toDim(scrollX_ + visibleWidth), toDim(scrollY_ + row + 1)}],
        Vector{0, row}
      );
    }
  }
  if (
    cursor_.x() >= scrollX_ and cursor_.x() < toDim(scrollX_ + size.x()) and
    cursor_.y() >= scrollY_ and cursor_.y() < toDim(scrollY_ + size.y())
  ) {
    visible.setAttr(
      CharAttributes{RgbNone, RgbNone, reverse},
      Rectangle{
        toDim(cursor_.x() - scrollX_), toDim(cursor_.y() - scrollY_),
        toDim(cursor_.x() - scrollX_ + 1), toDim(cursor_.y() - scrollY_ + 1)
      }
    );
  }
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
