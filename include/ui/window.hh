#pragma once

#include "element.hh"
#include "geometry.hh"
#include "term.hh"

namespace jwezel::ui {

struct Window: public Element
{
  Window(struct Ui *ui, const Char &background=Space, const Rectangle &area=RectangleDefault);

  virtual Rectangle area() const override;

  virtual Element *window() const override {return const_cast<Window *>(this);}

  virtual Vector size() const override;

  virtual Dim width() const override;

  virtual Dim height() const override;

  virtual void update(const Updates &updates) override;

  jwezel::Window window_;
};

} // namespace
