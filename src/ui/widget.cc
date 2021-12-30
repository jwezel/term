#include "Yoga.h"

#include "text.hh"
#include "widget.hh"
#include <iostream>
#include <string_view>

using namespace jwezel::ui;

Widget::Widget(Container *parent, const Char &background, const std::string_view &text_):
LayoutElement{parent},
background{background},
text{text_, RgbNone, background.attributes.bg}
{
  // YGNodeStyleSetMinWidthPercent(layoutNode, 100.);
  // YGNodeStyleSetMinHeightPercent(layoutNode, 100.);
  YGNodeStyleSetAlignContent(layoutNode, YGAlignSpaceAround);
  YGNodeSetMeasureFunc(
    layoutNode,
    [](YGNode *node, float width, YGMeasureMode wmm, float height, YGMeasureMode hmm) -> YGSize {
      std::cout << "Measure: " << string(static_cast<Widget *>(YGNodeGetContext(node))->text.size()) << "\r\n";
      auto size{static_cast<Widget *>(YGNodeGetContext(node))->text.size()};
      return YGSize{float(size.x), float(size.y)};
    }
  );
}

void Widget::draw() {
  text.fill(background);
}
