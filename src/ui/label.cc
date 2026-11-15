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

void Label::render() {
  write(Vector{0, 0}, text_);
}

} // namespace jwezel::ui
