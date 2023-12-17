#include "ui/container.hh"

namespace jwezel::ui {

Element::Element(Element *parent):
parent_(parent)
{
  if (parent) {
    dynamic_cast<Container *>(parent)->add(this);
  }
}

} // namespace jwezel::ui
