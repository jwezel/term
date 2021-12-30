#pragma once

#include <memory>
#include <vector>

#include "container.hh"
#include "element.hh"

namespace jwezel::ui {

struct MultiContainer: public Container {
  MultiContainer(struct Container *parent=0): Container{parent} {}

  virtual int add(Element *element);

  ///
  /// Add element
  ///
  /// @param      element  The element
  virtual int add(Element *element, Element *before);

  std::vector<std::unique_ptr<Element>> children;
};

}
