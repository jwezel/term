#include "frame.hh"

namespace jwezel::ui {

Frame::Frame(
  Container &parent,
  const Rectangle &area,
  const Char &background,
  const Decoration *decoration,
  ui::Element *content,
  const Text &title,
  const ColorStyle *colors
):
Widget(area, background, parent),
decoration_(decoration),
colors_(colors),
content_(content),
title_(title)
{
  focusPolicy(FocusPolicy::none);
  render();
}

void Frame::state(DecorationState state) {
  if (state != state_) {
    state_ = state;
    render();
  }
}

void Frame::title(const Text &title) {
  title_ = title;
  render();
}

void Frame::render() {
  fill(background()); // nothing of an earlier frame may remain
  write(
    Vector{0, 0},
    (decoration_? *decoration_: defaultDecoration()).draw(
      area().size(), state_, title_, background(), colors_? *colors_: defaultColorStyle()
    )
  );
}

} // namespace jwezel::ui
