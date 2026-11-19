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
  layoutManaged(area == RectangleDefault);
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
  auto crossLength{orientation_ == Orientation::vertical? content.width(): content.height()};
  crossLength = std::max(crossLength, Dim{0});
  std::vector<Vector> preferred(static_cast<size_t>(count));
  for (auto i = 0; i < count; ++i) {
    preferred[static_cast<size_t>(i)] = ordered_[static_cast<size_t>(i)]->preferredSize();
  }

  // What the children that are as large as they prefer take from the main axis; the others share the rest
  std::vector<Dim> lengths(static_cast<size_t>(count), 0);
  auto fitTotal{Dim{0}};
  for (auto i = 0; i < count; ++i) {
    if (sizing(i) == LayoutSize::fit) {
      const auto main{orientation_ == Orientation::vertical? preferred[static_cast<size_t>(i)].y(): preferred[static_cast<size_t>(i)].x()};
      lengths[static_cast<size_t>(i)] = std::clamp(main, Dim{0}, mainLength);
      fitTotal = toDim(fitTotal + lengths[static_cast<size_t>(i)]);
    }
  }
  if (fitTotal > mainLength) {
    // More than there is room for: the children that come first get what is left, the others nothing
    auto remaining{mainLength};
    for (auto i = 0; i < count; ++i) {
      if (sizing(i) == LayoutSize::fit) {
        lengths[static_cast<size_t>(i)] = std::min(lengths[static_cast<size_t>(i)], remaining);
        remaining = toDim(remaining - lengths[static_cast<size_t>(i)]);
      }
    }
    fitTotal = mainLength;
  }
  const auto stretchLength{toDim(mainLength - fitTotal)};
  auto weightSum{0.};
  for (auto i = 0; i < count; ++i) {
    if (sizing(i) == LayoutSize::stretch) {
      weightSum += weight(i);
    }
  }
  if (weightSum > 0.) {
    auto accumulatedWeight{0.};
    auto previous{Dim{0}};
    for (auto i = 0; i < count; ++i) {
      if (sizing(i) != LayoutSize::stretch) {
        continue;
      }
      accumulatedWeight += weight(i);
      const auto end{toDim(
        std::llround(static_cast<double>(stretchLength) * accumulatedWeight / weightSum)
      )};
      lengths[static_cast<size_t>(i)] = toDim(end - previous);
      previous = end;
    }
  }

  auto offset{Dim{0}}; // start of the current slice, relative to the content origin
  for (auto i = 0; i < count; ++i) {
    const auto length{lengths[static_cast<size_t>(i)]};
    auto crossSize{crossLength};
    auto crossOffset{Dim{0}};
    if (const auto align{alignment(i)}; align != LayoutAlign::stretch) {
      const auto wanted{orientation_ == Orientation::vertical? preferred[static_cast<size_t>(i)].x(): preferred[static_cast<size_t>(i)].y()};
      crossSize = std::clamp(wanted, Dim{0}, crossLength);
      switch (align) {
        case LayoutAlign::center:
        crossOffset = toDim((crossLength - crossSize) / 2);
        break;

        case LayoutAlign::end:
        crossOffset = toDim(crossLength - crossSize);
        break;

        default:
        break;
      }
    }
    const auto childArea{
      orientation_ == Orientation::vertical?
      Rectangle{
        toDim(content.x1() + crossOffset),
        toDim(content.y1() + offset),
        toDim(content.x1() + crossOffset + crossSize),
        toDim(content.y1() + offset + length)
      }:
      Rectangle{
        toDim(content.x1() + offset),
        toDim(content.y1() + crossOffset),
        toDim(content.x1() + offset + length),
        toDim(content.y1() + crossOffset + crossSize)
      }
    };
    offset = toDim(offset + length + static_cast<Dim>(layout_.spacing));
    ordered_[static_cast<size_t>(i)]->move(childArea);
  }
}

auto Box::preferredSize() -> Vector {
  const auto count{static_cast<Dim>(ordered_.size())};
  auto mainLength{Dim{0}};
  auto crossLength{Dim{0}};
  for (auto i = 0; i < count; ++i) {
    const auto size{ordered_[static_cast<size_t>(i)]->preferredSize()};
    mainLength = toDim(mainLength + (orientation_ == Orientation::vertical? size.y(): size.x()));
    crossLength = std::max(crossLength, orientation_ == Orientation::vertical? size.x(): size.y());
  }
  if (count > 0) {
    mainLength = toDim(mainLength + static_cast<Dim>(layout_.spacing) * (count - 1));
  }
  const auto padding{toDim(2 * static_cast<Dim>(layout_.padding))};
  mainLength = toDim(mainLength + padding);
  crossLength = toDim(crossLength + padding);
  return orientation_ == Orientation::vertical? Vector{crossLength, mainLength}: Vector{mainLength, crossLength};
}

void Box::layoutChanged() {
  layout();
  Container::layoutChanged();
}

auto Box::weight(Dim index) const -> double {
  return index < static_cast<Dim>(layout_.weights.size())? layout_.weights[static_cast<size_t>(index)]: 1.;
}

auto Box::sizing(Dim index) const -> LayoutSize {
  return index < static_cast<Dim>(layout_.sizes.size())? layout_.sizes[static_cast<size_t>(index)]: LayoutSize::stretch;
}

auto Box::alignment(Dim index) const -> LayoutAlign {
  return index < static_cast<Dim>(layout_.alignments.size())?
    layout_.alignments[static_cast<size_t>(index)]:
    LayoutAlign::stretch;
}

} // namespace jwezel::ui
