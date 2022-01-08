#pragma once

#include "frame.hh"
#include "geometry.hh"
#include "keyboard.hh"
#include "term.hh"
#include "update.hh"
#include "widget.hh"

namespace jwezel::ui {

struct Window: public Frame
{
  Window(struct Ui *ui, const Char &background=Space, const Rectangle &area=RectangleDefault);

  virtual Rectangle area() const;

  virtual Vector size() const;

  virtual Dim width() const;

  virtual Dim height() const;

  void update(const Updates &updates);

  jwezel::Window window;
};

} // namespace
