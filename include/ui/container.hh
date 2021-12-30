#pragma once

#include <memory>
#include <vector>

#include "element.hh"

namespace jwezel::ui {

struct Container: public Element {
  Container(struct Container *parent=0): Element{parent} {}

  virtual int add(Element *element) = 0;

  virtual int add(Element *element, Element *before) = 0;
};

}
