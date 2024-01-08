#include "frame.hh"
#include "term/geometry.hh"
#include "ui/container.hh"

namespace jwezel::ui {

Frame::Frame(FrameStyle *style, Container *parent, ui::Element *content, const Rectangle &area, const Char &background):
Widget(area, background, parent),
style_(style),
content_(content)
{
  if (style) {
    style->draw();
  }
}

} // namespace jwezel::ui
