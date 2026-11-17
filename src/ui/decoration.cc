#include "decoration.hh"

#include <algorithm>

namespace jwezel::ui {

namespace {

///
/// Put a character with the attributes of a role onto what is there
auto layer(const Char &below, const Char &on) -> Char {
  auto result{below.combine(on)};
  result.attributes.mix = AttributeMode::replace; // so that it replaces the cell in the window, whatever was there
  return result;
}

} // namespace

BoxDecoration::BoxDecoration(const BorderStyle &active, const BorderStyle &inactive, Alignment titleAlignment):
borders_{inactive, active}, // by state
titleAlignment_{titleAlignment}
{}

auto BoxDecoration::draw(
  const Vector &size,
  DecorationState state,
  const Text &title,
  const Char &background,
  const ColorStyle &colors
) const -> Text {
  Text result{Untouched, size};
  if (size.x() < 2 or size.y() < 2) { // no room for a frame
    return result;
  }
  const auto &border{borders_[static_cast<std::size_t>(state)]};
  Text lines{Char{' '}, size};
  lines.box(Box{RectangleMax, border.strength, border.dash, border.rounded});
  const auto &frame{colors[frameRole(state)]};
  const auto lastX{toDim(size.x() - 1)}, lastY{toDim(size.y() - 1)};
  for (Dim y = 0; y < size.y(); ++y) {
    for (Dim x = 0; x < size.x(); ++x) {
      if (x == 0 or y == 0 or x == lastX or y == lastY) {
        result.data[y][x] = layer(background, Char{lines.data[y][x].rune, frame});
      }
    }
  }
  if (!title.data.empty()) {
    const auto &line{title.data[0]};
    const auto available{toDim(size.x() - 2)}; // between the corners
    const auto length{std::min(available, toDim(line.size()))};
    const auto slack{toDim(available - length)};
    const auto start{
      toDim(1 + (titleAlignment_ == Alignment::center? slack / 2: titleAlignment_ == Alignment::right? slack: 0))
    };
    const auto &role{colors[titleRole(state)]};
    for (Dim i = 0; i < length; ++i) {
      const auto &character{line[static_cast<std::size_t>(i)]};
      auto &cell{result.data[0][static_cast<std::size_t>(start + i)]};
      // the role puts its attributes onto the frame, and what the title has to say for itself onto that
      cell = layer(layer(cell, Char{character.rune, role}), Char{character.rune, character.attributes});
    }
  }
  return result;
}

auto defaultDecoration() -> const Decoration & {
  static const BoxDecoration decoration;
  return decoration;
}

} // namespace jwezel::ui
