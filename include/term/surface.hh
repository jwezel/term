#pragma once

#include "geometry.hh"
#include "text.hh"
#include "update.hh"

#include <initializer_list>
#include <memory>
#include <unordered_map>

namespace jwezel {

struct Surface {
  struct Element {
    Element(const Rectangle &area): fragments{area} {}
    virtual Text text(const Rectangle &area) const = 0;
    virtual Rectangle area() const = 0;
    virtual void move(const Rectangle &area) = 0;

    vector<Rectangle> fragments;
  };

  struct Fragment {
    Rectangle area;
    Element *element;

    operator string() const;
  };

  Surface() {}

  Surface(initializer_list<Element *>);

  Updates addElement(Surface::Element * element, Surface::Element *below);

  Updates deleteElement(Element *element);

  Updates reshapeElement(Element *element, const Rectangle &area);

  vector<Element *> zorder;
  unordered_map<Element *, unique_ptr<Element>> elements;
};

Updates SurfaceUpdates(const vector<Surface::Fragment> &fragments);

} //namespace jwezel
