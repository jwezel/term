#include "Yoga.h"

#include "element.hh"
#include "range/v3/algorithm/copy.hpp"
#include "range/v3/all.hpp"
#include "text.hh"
#include "update.hh"
#include "widget.hh"
#include <iostream>
#include <iterator>
#include <string_view>

using namespace jwezel::ui;

Widget::Widget(Element *parent, const Char &background):
Element{parent},
background_{background}
{
  // YGNodeStyleSetMinWidthPercent(layoutNode_, 100.);
  // YGNodeStyleSetMinHeightPercent(layoutNode_, 100.);
  YGNodeSetMeasureFunc(
    layoutNode_,
    [](YGNode *node, float width, YGMeasureMode wmm, float height, YGMeasureMode hmm) -> YGSize {
      // std::cout << "Measure: " << string(static_cast<Widget *>(YGNodeGetContext(node))->text.size()) << "\r\n";
      auto size{static_cast<Widget *>(YGNodeGetContext(node))->text_.size()};
      return YGSize{float(size.x), float(size.y)};
    }
  );
}

jwezel::Updates Widget::draw() {
  Updates result{Element::draw()};
  text_.fill(background_);
  ranges::copy(Updates{{area_.position(), text_}}, std::back_inserter(result)); // TODO: optimize
}
