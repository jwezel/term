#pragma once

#include "container.hh"
#include "geometry.hh"

struct YGNode;
struct YGConfig;

namespace jwezel::ui {

struct LayoutElement: public Element
{

  ///
  /// Create LayoutElement
  ///
  /// @param      parent  The parent
  LayoutElement(struct Container *parent=0, const Rectangle &area=RectangleDefault);

  ///
  /// Destroy LayoutElement
  virtual ~LayoutElement() = default;

  ///
  /// Draw element
  inline virtual void draw() {}

  ///
  /// Adjust layout
  virtual void doLayout();

  virtual void area(const Rectangle &area);

  virtual Rectangle area() const;

  virtual Vector size() const;

  virtual Dim width() const;

  virtual Dim height() const;

  struct YGConfig *layout;
  struct YGNode *layoutNode;
};

} // namespace
