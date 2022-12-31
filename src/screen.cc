#include "_screen_window.hh"
#include "geometry.hh"
#include "screen.hh"
#include "surface.hh"
#include "update.hh"

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>

#include <fmt/format.h>
#include <range/v3/action/push_back.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/action/remove_if.hpp>
#include <range/v3/view/transform.hpp>


using fmt::format;

using namespace jwezel;
using namespace ranges;

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
    views::transform(
      [window] (const Rectangle &area) {
        return Surface::Fragment{area, window};
      }
    ) |
    to<vector>();
}

Screen::Screen():
Surface{&backdrop}
{}

void Screen::focus(Window *window) {
    assert(find(zorder.begin(), zorder.end(), window) != zorder.end());
    focusWindow = window;
}

BaseWindow *Screen::focus() const {
  return focusWindow;
}

bool Screen::unfocus(Window *window) {
  if (focusWindow == window) {
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
    assert(pos != zorder.end());
    focusWindow = dynamic_cast<BaseWindow *>(*pos);
  }
  return true;
}

tuple<Window *, Updates> Screen::addWindow(const Rectangle &area_, const Char &background, Window *below, Window *) {
  // Setup
  const auto topArea{zorder.back()->area()};
  const auto area{
    area_ == RectangleDefault?
      Rectangle(topArea.x1 + 1, topArea.y1 + 1, topArea.x2 + 1, topArea.y2 + 1)
    :
      area_
  };
  // Create window
  auto window{new Window(area, background)};
  auto result{Surface::addElement(window, below)};
  focusWindow = window;
  return std::make_tuple(window, result);
}

Updates Screen::deleteWindow(Window *window) {
  Updates result;
  if (unfocus(window)) {
    result = Surface::deleteElement(window);
  }
  return result;
}

Updates Screen::reshapeWindow(Window *window, const Rectangle &area) {
  Updates updates;
  if (window->area() != area) {
    updates = Surface::reshapeElement(window, area);
  }
  return updates;
}

Updates Screen::fill(Window *window, const Char &fillChar, const Rectangle &area) {
  window->fill(fillChar, area);
  return SurfaceUpdates(fragmentsOf(window, window->fragments));
}

Updates Screen::text(Window *window, const Vector &position, const Text &text) {
  window->write(position, text);
  // This implementation generates just one update for the whole text which in the end
  // is a rectangle. Shorter lines will therefore be filled with unchanged content.
  // However, the screen update algorithm will straighten these unnecessary updates out.
  // An alternative would be to split the updates line by line, or better, blocks of
  // consecutive lines having the same length.
  auto textArea{Rectangle(position, position + text.size())};
  return SurfaceUpdates(
    window->fragments |
    views::transform([&textArea, window](const Rectangle &r) {return r & textArea + window->position;}) |
    views::remove_if([](const std::optional<Rectangle> &r){return !r;}) |
    views::transform(
      [window](const std::optional<Rectangle> &r) {
        return Fragment{r.value(), window};
      }
    ) |
    to<std::vector<Fragment>>()
  );
}

Updates Screen::line(Window * window, const Line &line, u1 strength, u1 dash, bool roundedCorners) {
  const auto area{window->line(line, strength, dash, roundedCorners) + window->area().position()};
  // Screen updates have the same limitations as in @c Screen::text
  return SurfaceUpdates(
    window->fragments |
    views::transform([&area](const Rectangle &r) {return r & area;}) |
    views::remove_if([](const std::optional<Rectangle> &r) {return !r;}) |
    views::transform([window](const std::optional<Rectangle> &r) {return Fragment{r.value(), window};}) |
    to<std::vector<Fragment>>()
  );
}

Updates Screen::box(Window *window, const Box &box) {
  const auto areas{window->box(box)};
  vector<Fragment> updates;
  for (const auto &fragment: window->fragments) {
    for (const auto &area: areas) {
      const auto update{fragment & area + window->area().position()};
      if (update) {
        updates.push_back(Fragment{update.value(), window});
      }
    }
  }
  return SurfaceUpdates(updates);
}

Vector Screen::relative(Window *window, const Vector &position) const {
  const auto &area{window->area()};
  if (position.x < 0 or position.x > area.width() or position.y < 0 or position.y > area.height()) {
    throw range_error(format("Position outside window dimensions {}: {}", string(area.size()), string(position)));
  }
  return position + area.position();
}

Vector Screen::minSize(Window *exclude) const {
  Vector result{0, 0};
  for (const auto &element: elements) {
    auto window{dynamic_cast<const Window *>(element.first)};
    if (exclude and window != exclude) {
      result = max(result, window->position + window->size());
    }
  }
  return result;
}
