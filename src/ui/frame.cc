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
framing_{},
content_(content)
{
  framing_.decoration = decoration;
  framing_.title = title;
  framing_.state = DecorationState::inactive;
  framing_.colors = colors;
  focusPolicy(FocusPolicy::none);
  render();
}

void Frame::state(DecorationState state) {
  if (state != framing_.state) {
    framing_.state = state;
    render();
  }
}

void Frame::title(const Text &title) {
  framing_.title = title;
  render();
}

void Frame::render() {
  fill(background()); // nothing of an earlier frame may remain
  write(Vector{0, 0}, framing_.draw(area().size(), background()));
}

} // namespace jwezel::ui
