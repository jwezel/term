#include <algorithm>
#include <memory>
#include <stdexcept>

#include "Yoga.h"
#include "element.hh"
#include "multi_container.hh"
#include "element.hh"

using namespace jwezel::ui;

int MultiContainer::add(Element *element, Element *before) {
  decltype(children)::iterator insertPos;
  if (before) {
    insertPos = find(children.begin(), children.end(), std::unique_ptr<Element>(before));
    if (insertPos == children.end()) {
      throw std::runtime_error("MultiContainer::add: <before> Element not found");
    }
  } else {
    insertPos = children.end();
  }
  auto result = distance(children.begin(), insertPos);
  children.insert(insertPos, std::unique_ptr<Element>(element));
  return result;
}

int MultiContainer::add(Element *element) {
  return add(element, 0);
}
