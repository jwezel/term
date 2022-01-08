#pragma once

#include <memory>

#include "container.hh"
#include "geometry.hh"
#include "element.hh"

namespace jwezel::ui {

struct Controller: public virtual Element
{
  Controller(Element *parent=0, const Rectangle &area=RectangleDefault);

  // virtual int add(Element *controlled) override;

  // virtual int add(Element *controlled, Element *before) override;

  virtual void render() override;

  std::unique_ptr<Element> controlled;
};

} // namespace
