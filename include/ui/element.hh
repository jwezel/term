#pragma once

#include "geometry.hh"  // for Dim, Rectangle, RectangleDefault, Vector
#include "text.hh"
#include "update.hh"

#include <memory>

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
    const Vector &size=VectorMin
  );

  ///
  /// Destroy Element
  virtual ~Element() = default;

  virtual const Element *window() const;

  virtual Updates updated();

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

  virtual void setMinimumWidth(Dim width);

  virtual void setMinimumHeight(Dim height);

  virtual void setMaximumWidth(Dim width);

  virtual void setMaximumHeight(Dim height);

  virtual void setPadding(const Rectangle &padding);

  virtual void setBorder(const Rectangle &padding);

  virtual void setMargin(const Rectangle &padding);

  struct YGConfig *layout_;
  struct YGNode *layoutNode_;
  bool needsDrawing_;
  bool needsRendering_;
  Orientation orientation_;
  Rectangle area_;
  struct Element *parent_;
  vector<unique_ptr<Element>> children_;
};

} // namespace
