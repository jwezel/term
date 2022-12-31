#pragma once

#include "geometry.hh"  // for Dim, Rectangle, RectangleDefault, Vector
// #include "optional.hh"  // for optional
#include "term.hh"
#include "text.hh"
#include "update.hh"
#include <memory>

// namespace jwezel {struct Text;}

namespace jwezel::ui {

struct Element
{

  ///
  /// Create Element
  ///
  /// @param      parent  The parent
  Element(
    struct Element *parent=0,
    Orientation orientation=Vertical,
    const Rectangle &area=RectangleDefault
  );

  ///
  /// Destroy Element
  virtual ~Element() = default;

  virtual const Element *window() const;

  virtual jwezel::Updates updated();

  virtual Updates render();

  ///
  /// Adjust layout
  virtual void doLayout();

  virtual Rectangle area() const;

  virtual void area(const Rectangle &area);

  virtual Vector size() const;

  virtual Dim width() const;

  virtual void updateEvent() const {}

  inline virtual Updates draw() {return {};}

  virtual Dim height() const;

  virtual int add(Element *element);

  virtual int add(Element *element, Element *before);

  virtual void update(const Updates &) {}

  struct YGConfig *layout_;
  struct YGNode *layoutNode_;
  bool needsDrawing_;
  bool needsRendering_;
  Orientation orientation_;
  Rectangle area_;
  struct Element *parent_;
  // struct Element *window_;
  vector<unique_ptr<Element>> children_;
};

} // namespace
