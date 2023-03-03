#include "_screen_window.hh"
#include "geometry.hh"
#include "screen.hh"
#include "surface.hh"
#include "update.hh"

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>

#include <fmt/format.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>


namespace jwezel::screen {

using fmt::format;
using ranges::views::transform, ranges::_to_::to, ranges::views::remove_if;
using std::cerr, std::endl;

///
/// Transform rectangles and a Window * to vector of Fragments
///
/// @param      window      The window
/// @param[in]  rectangles  The rectangles
///
/// @return     vector<Fragment>
static auto fragmentsOf(Window *window, const vector<Rectangle>&rectangles) {
  return
    rectangles |
    transform([window] (const Rectangle &area) {return Surface::Fragment{area, window}; } ) |
    to<vector>();
}

Screen::Screen():
Surface{&backdrop_},
focusWindow_{0}
{}

void Screen::focus(Window *window) {
    assert(find(zorder.begin(), zorder.end(), window) != zorder.end()); // NOLINT
    focusWindow_ = window;
}

auto Screen::focus() const -> BaseWindow * {
  return focusWindow_;
}

auto Screen::unfocus(Window *window) -> bool {
  if (focusWindow_ == window) {
    const auto zpos = find(zorder.begin(), zorder.end(), window);
    if (zpos == zorder.end()) {
      cerr << "Window to unfocus not found" << endl;
      return false;
    }
    auto pos{
      zpos + 1 == zorder.end()?
        zpos > zorder.begin()?
          zpos - 1
        :
          zpos
      :
        zpos + 1
    };
    assert(pos != zorder.end()); // NOLINT
    focusWindow_ = dynamic_cast<BaseWindow *>(*pos);
  }
  return true;
}

auto Screen::addWindow(
  const Rectangle &area_,
  const Char &background,
  Window *below,
  Window * /*unused*/
) -> tuple<Window *, Updates> {
  // Setup
  const auto topArea{zorder.back()->area()};
  const auto area{
    area_ == RectangleDefault?
      Rectangle(toDim(topArea.x1() + 1), toDim(topArea.y1() + 1), toDim(topArea.x2() + 1), toDim(topArea.y2() + 1))
    :
      area_
  };
  // Create window
  auto *window{new Window(area, background)};
  auto result{Surface::addElement(window, below)};
  focusWindow_ = window;
  return std::make_tuple(window, result);
}

auto Screen::deleteWindow(Window *window) -> Updates {
  Updates result;
  if (unfocus(window)) {
    result = Surface::deleteElement(window);
  }
  return result;
}

auto Screen::reshapeWindow(Window *window, const Rectangle &area) -> Updates {
  Updates updates;
  if (window->area() != area) {
    updates = Surface::reshapeElement(window, area);
  }
  return updates;
}

auto Screen::fill(Window *window, const Char &fillChar, const Rectangle &area) -> Updates {
  window->fill(fillChar, area);
  return SurfaceUpdates(fragmentsOf(window, window->fragments));
}

auto Screen::text(Window *window, const Vector &position, const Text &text) -> Updates {
  window->write(position, text);
  // This implementation generates just one update for the whole text which in the end
  // is a rectangle. Shorter lines will therefore be filled with unchanged content.
  // However, the screen update algorithm will straighten these unnecessary updates out.
  // An alternative would be to split the updates line by line, or better, blocks of
  // consecutive lines having the same length.
  auto textArea{Rectangle(position, position + text.size())};
  return SurfaceUpdates(
    window->fragments |
    transform([&textArea, window](const Rectangle &r) {return r & textArea + window->position;}) |
    remove_if([](const std::optional<Rectangle> &r){return !r;}) |
    transform(
      [window](const std::optional<Rectangle> &r) {
        return Fragment{r.value(), window};
      }
    ) |
    to<std::vector<Fragment>>()
  );
}

auto Screen::line(Window * window, const Line &line, u1 strength, u1 dash, bool roundedCorners) -> Updates {
  const auto area{window->line(line, strength, dash, roundedCorners) + window->area().position()};
  // Screen updates have the same limitations as in @c Screen::text
  return SurfaceUpdates(
    window->fragments |
    transform([&area](const Rectangle &r) {return r & area;}) |
    remove_if([](const std::optional<Rectangle> &r) {return !r;}) |
    transform([window](const std::optional<Rectangle> &r) {return Fragment{r.value(), window};}) |
    to<std::vector<Fragment>>()
  );
}

auto Screen::box(Window *window, const Box &box) -> Updates {
  const auto areas{window->box(box)};
  vector<Fragment> updates;
  for (const auto &fragment: window->fragments) {
    for (const auto &area: areas) {
      const auto update{fragment & area + window->area().position()};
      if (update) {
        updates.emplace_back(update.value(), window);
      }
    }
  }
  return SurfaceUpdates(updates);
}

auto Screen::relative(Window *window, const Vector &position) const -> Vector {
  const auto &area{window->area()};
  if (position.x() < 0 or position.x() > area.width() or position.y() < 0 or position.y() > area.height()) {
    throw range_error(format("Position outside window dimensions {}: {}", string(area.size()), string(position)));
  }
  return position + area.position();
}

auto Screen::minSize(Window *exclude) const -> Vector {
  Vector result{0, 0};
  for (const auto &element: elements) {
    const auto *window{dynamic_cast<const Window *>(element.first)};
    if (exclude and window != exclude) {
      result = max(result, window->position + window->size());
    }
  }
  return result;
}
} // namespace jwezel::screen
