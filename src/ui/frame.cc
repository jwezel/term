#include "frame.hh"
#include "term/geometry.hh"
#include "container.hh"

namespace jwezel::ui {

Frame::Frame(Container &parent, const Rectangle &area, const Char &background, FrameStyle *style, ui::Element *content):
Widget(area, background, parent),
style_(style),
content_(content)
{
  if (style_) {
    style_->draw();
  }
}

} // namespace jwezel::ui
