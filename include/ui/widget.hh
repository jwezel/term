#pragma once

#include "element.hh"
#include "text.hh"

namespace jwezel::ui {

struct Widget: public Element
{
  Widget(struct Element *parent=0, const Char &background=Space);

  virtual ~Widget() = default;

  virtual void drawEvent() const;

  virtual Updates draw();

  virtual void render();

  Char background_;
  Text text_;
};

} // namespace
