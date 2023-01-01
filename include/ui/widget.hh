#pragma once

#include "element.hh"
#include "geometry.hh"
#include "text.hh"
#include <string_view>

namespace jwezel::ui {

struct Widget: public Element
{
  Widget(struct Element *parent=0, const Char &background=Space, const Vector &size=VectorMin);

  virtual ~Widget() = default;

  virtual void drawEvent() const {}

  virtual Updates draw() override;

  // virtual void render() override {};

  Char background_;
  Text text_;
};

} // namespace
