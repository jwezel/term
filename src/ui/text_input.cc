#include "text_input.hh"
#include "root.hh"

#include <algorithm>

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

TextInput::TextInput(Container &parent, const Rectangle &area, std::string_view text, const Char &background):
Widget{area, background, parent},
buffer_{text}
{
  start();
}

TextInput::TextInput(Container &parent, std::string_view text, const Char &background):
Widget{RectangleDefault, background, parent},
buffer_{text}
{
  start();
}

void TextInput::start() {
  cursor_ = buffer_.end();
  scrollIntoView();
  render();
  registerHandlers();
  layoutChanged();
}

void TextInput::contents(std::string_view text) {
  buffer_.text(text);
  cursor_ = buffer_.end();
  scrollX_ = 0;
  scrollY_ = 0;
  scrollIntoView();
  render();
  layoutChanged();
}

void TextInput::registerHandlers() {
  onKey([this](const ui::Element &, const Event &event) {return handleKey(event);});
}

auto TextInput::handleKey(const Event &event) -> bool {
  const auto key{event.as<KeyEvent>().key()};
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

    case Key::PageUp:
    case Key::PageDown:
    if (!acceptsEnter()) {
      return false; // a line has no pages: the window may have a use for the key
    }
    movePage(key == Key::PageDown? 1: -1);
    break;

    case Key::Home:
    cursor_.x() = 0;
    break;

    case Key::End:
    cursor_.x() = buffer_.lineLength(cursor_.y());
    break;

    case Key::Backspace:
    backspace();
    break;

    case Key::Delete:
    del();
    break;

    case Key::Enter:
    if (!acceptsEnter()) {
      return accept(event); // nothing has changed; and if nobody is interested, the window may be
    }
    insertNewline();
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
  buffer_.insert(cursor_, key);
  ++cursor_.x();
  layoutChanged();
}

void TextInput::insertNewline() {
  buffer_.split(cursor_);
  ++cursor_.y();
  cursor_.x() = 0;
  layoutChanged();
}

void TextInput::backspace() {
  cursor_ = buffer_.eraseBefore(cursor_);
  layoutChanged();
}

void TextInput::del() {
  buffer_.erase(cursor_);
  layoutChanged();
}

void TextInput::moveLeft() {
  if (cursor_.x() > 0) {
    --cursor_.x();
  } else if (cursor_.y() > 0) {
    --cursor_.y();
    cursor_.x() = buffer_.lineLength(cursor_.y());
  }
}

void TextInput::moveRight() {
  if (cursor_.x() < buffer_.lineLength(cursor_.y())) {
    ++cursor_.x();
  } else if (cursor_.y() + 1 < buffer_.lineCount()) {
    ++cursor_.y();
    cursor_.x() = 0;
  }
}

void TextInput::moveUp() {
  if (cursor_.y() > 0) {
    --cursor_.y();
    cursor_.x() = std::min(cursor_.x(), buffer_.lineLength(cursor_.y()));
  }
}

void TextInput::moveDown() {
  if (cursor_.y() + 1 < buffer_.lineCount()) {
    ++cursor_.y();
    cursor_.x() = std::min(cursor_.x(), buffer_.lineLength(cursor_.y()));
  }
}

void TextInput::movePage(int direction) {
  const auto page{std::max(Dim{1}, area().height())};
  const auto target{std::clamp(toDim(cursor_.y() + direction * page), Dim{0}, toDim(buffer_.lineCount() - 1))};
  const auto moved{toDim(target - cursor_.y())};
  cursor_.y() = target;
  cursor_.x() = std::min(cursor_.x(), buffer_.lineLength(target));
  // What is shown moves as far as the cursor does, so it stays where it is in the area, but it does not move
  // beyond what there is
  const auto lastTop{std::max(Dim{0}, toDim(buffer_.lineCount() - page))};
  scrollY_ = std::clamp(toDim(scrollY_ + moved), Dim{0}, lastTop);
}

auto TextInput::accept(const Event &event) -> bool {
  return emit(AcceptEvent{UiEvent::causeOf(event), contents()});
}

auto TextInput::onAccept(const Handler &handler) -> int {
  return on<AcceptEvent>(handler);
}

auto TextInput::preferredSize() -> Vector {
  auto width{Dim{1}}; // room for the caret behind the text
  for (const auto &line: buffer_.lines()) {
    width = std::max(width, toDim(line.size() + 1));
  }
  return Vector{width, acceptsEnter()? std::max(Dim{1}, buffer_.lineCount()): Dim{1}};
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

auto TextInput::alignmentOffset() const -> Vector {
  auto width{Dim{1}}; // includes the cursor cell behind the last character
  for (const auto &line: buffer_.lines()) {
    width = std::max(width, toDim(line.size() + 1));
  }
  const Vector content{width, acceptsEnter()? std::max(Dim{1}, buffer_.lineCount()): Dim{1}};
  const auto size{area().size()};
  const auto aligned{contentPosition(content)};
  return Vector{content.x() <= size.x()? aligned.x(): Dim{0}, content.y() <= size.y()? aligned.y(): Dim{0}};
}

void TextInput::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0) {
    return;
  }
  auto visible{canvas(inputRoles[static_cast<size_t>(widgetState())])};
  const auto offset{alignmentOffset()};
  for (Dim row = 0; row < size.y() and toDim(scrollY_ + row) < buffer_.lineCount(); ++row) {
    const auto &line{buffer_.line(toDim(scrollY_ + row))};
    // The part of the line that is in the area; a line can be shorter than the part that is scrolled away
    const auto length{std::min(size.x(), toDim(toDim(line.size()) - scrollX_))};
    if (length <= 0) {
      continue;
    }
    Text characters{Space, Vector{length, 1}};
    for (Dim column = 0; column < length; ++column) {
      characters.data[0][static_cast<size_t>(column)] = Char{line[static_cast<size_t>(scrollX_ + column)]};
    }
    overlay(visible, characters, Vector{offset.x(), toDim(offset.y() + row)});
  }
  if (auto *root{this->root()}; root->focus() == this) {
    root->updateCaret(); // before writing, so the display parks the cursor at the new caret afterwards
  }
  write(Vector{0, 0}, visible);
}

auto TextInput::caret() const -> std::optional<Vector> {
  const auto size{area().size()};
  if (
    cursor_.x() < scrollX_ or cursor_.x() >= toDim(scrollX_ + size.x()) or
    cursor_.y() < scrollY_ or cursor_.y() >= toDim(scrollY_ + size.y())
  ) {
    return std::nullopt;
  }
  const auto position{area().position()};
  const auto offset{alignmentOffset()};
  return Vector{
    toDim(position.x() + offset.x() + cursor_.x() - scrollX_),
    toDim(position.y() + offset.y() + cursor_.y() - scrollY_)
  };
}

} // namespace jwezel::ui
