#include "controller.hh"
#include "Yoga.h"
#include "element.hh"
#include "fmt/core.h"
#include "layout_element.hh"
#include <stdexcept>

using namespace jwezel::ui;

Controller::Controller(Container *parent, const Rectangle &area):
LayoutElement(parent, area)
{
  YGNodeStyleSetFlexDirection(layoutNode, YGFlexDirectionColumn);
}

int Controller::add(Element *controlled_, Element *) {
  if (controlled) {
    throw std::runtime_error("Controlled has already a controllee");
  }
  controlled.reset(controlled_);
#ifndef NDEBUG
  if (Container::parent) {
    assert(YGNodeGetChildCount(layoutNode) == 1);
  }
#endif
  return 0;
}

int Controller::add(Element *controlled_) {
  return add(controlled_, 0);
}

void Controller::doLayout() {
  LayoutElement::doLayout();
  // if (auto controlledLe = dynamic_cast<LayoutElement *>(controlled.get())) {
  //   controlledLe->doLayout();
  // } else {
  //   throw std::runtime_error("Controller::doLayout(): dynamic_cast<LayoutElement *>(controlled.get()) did not yield LayoutElement");
  // }
}
