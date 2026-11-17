#include "label.hh"

namespace jwezel::ui {

Label::Label(Container &parent, const Rectangle &area, const Text &text, const Char &background):
Widget{area, background, parent},
text_{text}
{
  render();
}

Label::Label(Container &parent, const Text &text, const Char &background):
Widget{RectangleDefault, background, parent},
text_{text}
{
  render();
}

void Label::text(const Text &text) {
  text_ = text;
  render();
}

void Label::render() {
  if (area().size().x() <= 0 or area().size().y() <= 0) {
    return;
  }
  auto visible{canvas(labelRole)};
  overlay(visible, text_, Vector{0, 0});
  write(Vector{0, 0}, visible);
}

} // namespace jwezel::ui
