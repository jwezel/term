#include "ui/container.hh"

#include <taitank.h>

namespace jwezel::ui {

Element::Element(struct Container *parent):
parent_(parent),
node_{taitank::NodeCreate()}
{
  if (parent) {
    dynamic_cast<Container *>(parent)->add(this);
  }
}

auto Element::window() /*NOLINT(misc-no-recursion)*/ -> Window * {
  return parent_? parent_->window(): 0;
}

}  // namespace jwezel::ui
