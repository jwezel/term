#pragma once

#include "controller.hh"
#include "geometry.hh"
#include "term.hh"

namespace jwezel::ui {

struct Window: Controller
{
  Window(struct Ui *ui, const Char &background=Space, const Rectangle &area=RectangleDefault);

  virtual Rectangle area() const;

  virtual Vector size() const;

  virtual Dim width() const;

  virtual Dim height() const;

  jwezel::Window window;
};

} // namespace
