#include "box.hh"

#include <cmath>
#include <vector>

namespace jwezel::ui {

Box::Box(Container &parent, const Rectangle &area, const BoxLayout &layout, Orientation orientation):
Container{&parent},
area_{area},
layout_{layout},
orientation_{orientation}
{
  parent.add(*this);
}

auto Box::add(Element &child) -> Container & {
  Container::add(child);
  ordered_.push_back(&child);
  layout();
  return *this;
}

void Box::remove(Element &child) {
  Container::remove(child);
  std::erase(ordered_, &child);
  layout();
}

void Box::move(const Rectangle &area) {
  area_ = area;
  layout();
}

void Box::layout() {
  const auto count{static_cast<Dim>(ordered_.size())};
  if (count == 0) {
    return;
  }
  const auto content{layout_.padding > 0? area_ - static_cast<Dim>(layout_.padding): area_};
  auto mainLength{orientation_ == Orientation::vertical? content.height(): content.width()};
  mainLength -= static_cast<Dim>(layout_.spacing) * (count - 1);
  if (mainLength <= 0) {
    return;
  }
  auto weightSum{0.};
  for (auto i = 0; i < count; ++i) {
    weightSum += weight(i);
  }
  if (weightSum <= 0.) {
    return;
  }
  auto accumulatedWeight{0.};
  auto previous{Dim{0}};  // end of the previous slice, relative to the content origin
  auto offset{Dim{0}};    // start of the current slice, relative to the content origin
  for (auto i = 0; i < count; ++i) {
    accumulatedWeight += weight(i);
    const auto end{toDim(std::llround(static_cast<double>(mainLength) * accumulatedWeight / weightSum))};
    const auto length{end - previous};
    previous = end;
    const auto childArea{
      orientation_ == Orientation::vertical?
      Rectangle{content.x1(), toDim(content.y1() + offset), content.x2(), toDim(content.y1() + offset + length)}:
      Rectangle{toDim(content.x1() + offset), content.y1(), toDim(content.x1() + offset + length), content.y2()}
    };
    offset = toDim(offset + length + static_cast<Dim>(layout_.spacing));
    ordered_[static_cast<size_t>(i)]->move(childArea);
  }
}

auto Box::weight(Dim index) const -> double {
  return index < static_cast<Dim>(layout_.weights.size())? layout_.weights[static_cast<size_t>(index)]: 1.;
}

} // namespace jwezel::ui
