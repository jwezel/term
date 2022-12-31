#pragma once

#include "surface.hh"
#include "widget.hh"

namespace jwezel::ui {

struct Window: public Widget, public Surface
{
  Window(struct Ui *ui, const Char &background=Space, const Rectangle &area=RectangleDefault);

  virtual Rectangle area() const override;

  virtual const jwezel::ui::Element *window() const override {return this;}

  virtual Vector size() const override;

  virtual Dim width() const override;

  virtual Dim height() const override;

  virtual void update(const Updates &updates) override;

  jwezel::Window window_;
};

} // namespace
