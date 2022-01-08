#pragma once

#include "geometry.hh"  // for Dim, Rectangle, RectangleDefault, Vector
#include "optional.hh"  // for optional
#include "text.hh"
#include "update.hh"
#include <memory>
namespace jwezel { struct Text; }

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
    const Rectangle &area=RectangleDefault,
    struct Window *window=0
  );

  ///
  /// Destroy Element
  virtual ~Element() = default;

  virtual void updateEvent() const;

  virtual Updates updated();

  inline virtual Updates draw() {return {};}

  virtual void render();

  ///
  /// Adjust layout
  virtual void doLayout();

  virtual void area(const Rectangle &area);

  virtual Rectangle area() const;

  virtual Vector size() const;

  virtual Dim width() const;

  virtual Dim height() const;

  virtual int add(Element *element);

  virtual int add(Element *element, Element *before);

  struct YGConfig *layout_;
  struct YGNode *layoutNode_;
  bool needsDrawing_;
  bool needsRendering_;
  Orientation orientation_;
  Rectangle area_;
  struct Element *parent_;
  struct Window *window_;
  vector<unique_ptr<Element>> children_;
};

} // namespace
