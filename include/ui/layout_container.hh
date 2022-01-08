#pragma once

#include "multi_container.hh"
#include "element.hh"
#include "geometry.hh"
#include "element.hh"
#include "text.hh"

namespace jwezel::ui {

///
/// LayoutContainer
struct LayoutContainer: public Element, MultiContainer {
  LayoutContainer(Container *parent, Orientation orientation=Vertical, const Rectangle &area=RectangleDefault);

  virtual int add(Element *element) override;

  virtual int add(Element *element, Element *before) override;

  Orientation orientation;
};

}
