#pragma once

#include <memory>

#include "element.hh"
#include "geometry.hh"
#include "layout_element.hh"

namespace jwezel::ui {

struct Controller: public LayoutElement, Container
{
  Controller(Container *parent=0, const Rectangle &area=RectangleDefault);

  virtual int add(Element *controlled) override;

  virtual int add(Element *controlled, Element *before) override;

  virtual void doLayout() override;

  std::unique_ptr<Element> controlled;
};

} // namespace
