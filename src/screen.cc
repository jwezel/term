#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>

#include <fmt/core.h>
#include <fmt/format.h>
#include <range/v3/action/insert.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/utility/optional.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/ref.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/transform.hpp>

#include "geometry.hh"
#include "screen.hh"
#include "window.hh"

using fmt::print, fmt::format;

using namespace jwezel;
using namespace ranges;

///
/// Transform rectangles and a Window * to vector of Fragments
///
/// @param      window      The window
/// @param[in]  rectangles  The rectangles
///
/// @return     vector<Fragment>
static auto fragments_of(Window *window, const vector<Rectangle>&rectangles) {
  return
    rectangles |
    views::transform(
      [window] (const Rectangle &area) {
        return Fragment{area, window};
      }
    ) |
    to<vector>();
}

Screen::Screen(const Vector &size, const Char &backbround):
screen{new Window(Rectangle{0, 0, size.x, size.y}, backbround)},
zorder{screen},
focusWindow{screen}
{
  windows.push_back(unique_ptr<Window>(screen));
}

Fragment::operator string() const {
  return format("Fragment({})", string(area));
}

void Screen::split(const Rectangle &area, vector<Rectangle> &fragments1, const vector<Rectangle> fragments2) {
  for (const auto &fragment2: fragments2) {
    if (area.intersects(fragment2)) {
      vector<Rectangle> shards;
      for (const auto &fragment1: fragments1) {
        const auto intersection{fragment1.defaultIntersection(fragment2)};
        std::copy(intersection.begin(), intersection.end(), back_inserter(shards));
      }
      fragments1 = shards;
    }
  }
}

Updates Screen::screenUpdates(const vector<Fragment> &fragments) {
  Updates result;
  result.reserve(fragments.size());
  for (const auto &fragment: fragments) {
    const auto area = fragment.area & zorder[0]->area();
    if (area) {
      result.emplace_back(
        Update{
          Vector(fragment.area.x1, fragment.area.y1),
          fragment.window->text()[area.value() - Vector(fragment.window->area().x1, fragment.window->area().y1)]
        }
      );
    }
  }
  return result;
}

void Screen::focus(Window *window) {
    focusWindow = window;
}

Window *Screen::focus() const {
  return focusWindow;
}

bool Screen::unfocus(Window *window) {
  if (focusWindow == window) {
    const auto zpos = find(zorder.begin(), zorder.end(), window);
    focusWindow = *(
      zpos == zorder.rbegin().base()?
        zpos > zorder.begin()?
          zpos - 1
        :
          zpos
      :
        zpos + 1
    );
    return true;
  } else {
    return false;
  }
}

tuple<Window *, Updates> Screen::addWindow(const Rectangle &area_, const Char &background, Window *below, Window *) {
  // Setup
  const auto topArea = zorder.back()->area();
  const auto area =
    area_ == RectangleDefault?
      Rectangle(topArea.x1 + 1, topArea.y1 + 1, topArea.x2 + 1, topArea.y2 + 1)
    :
      area_;
  // Create window
  auto window = new Window(area, background);
  for (auto &windowPtr: windows) {
    if (!windowPtr) {
      windowPtr.reset(window);
      goto windowRegistered;
    }
  }
  windows.push_back(unique_ptr<Window>(window));

  windowRegistered:
  if (focusWindow)
    focusWindow = window;
  // Add window to screen
  const auto insertPos =
    below?
      find(zorder.begin(), zorder.end(), below)
    :
      zorder.end();
  const auto i = std::distance(zorder.begin(), zorder.insert(insertPos, window));
  // Create own fragments from overlaying fragments when inserting it below the top
  for (auto j = i + 1; j < int(zorder.size()); ++j) {
    split(window->area(), window->fragments, zorder[j]->fragments);
  }
  // Create fragments of all windows overlayed by this
  for (auto z = i - 1; z >= 0; --z) {
    const auto windowBelow = zorder[z];
    if (windowBelow->area().intersects(window->area())) {
      split(windowBelow->area(), windowBelow->fragments, window->fragments);
    }
  }
  return std::make_tuple(
    window,
    screenUpdates(
      window->fragments | views::transform([window](const Rectangle &area){return Fragment{area, window};}) | to<vector>()
    )
  );
}

Updates Screen::deleteWindow(Window *window) {
  Updates result;
  if (unfocus(window)) {
    const auto zit{find(zorder.begin(), zorder.end(), window)};
    if (zit == zorder.end()) {
      throw domain_error(format("Window {} not found in zorder\n", fmt::ptr(window)));
    }
    const auto z = std::distance(zorder.begin(), zit);
    vector<Fragment> updates;
    // Remove window from screen
    zorder.erase(zorder.begin() + z);
    // Recreate fragments covered by removed window
    for (auto z1 = z - 1; z1 >= 0; --z1) {
      if (zorder[z1]->area().intersects(window->area())) {
        const auto zwindow = zorder[z1];
        zwindow->fragments = {zwindow->area()};
        for (auto z2 = z1 + 1; z2 < int(zorder.size()); ++z2) {
          split(zwindow->area(), zwindow->fragments, zorder[z2]->fragments);
        }
        // Add screen updates for these fragments
        updates |= push_back(
          zwindow->fragments |
          views::transform([window](const Rectangle &r) {return r & window->area();}) |
          views::remove_if([](const std::optional<Rectangle> &r) {return !r.has_value();}) |
          views::transform([zwindow](const std::optional<Rectangle> &r) {return Fragment{r.value(), zwindow};})
        );
      }
    }
    result = screenUpdates(updates);
    // Remove window from registry
    for (auto &windowPtr: windows) {
      if (windowPtr.get() == window) {
        windowPtr.reset(0);
        goto end;
      }
    }
    throw domain_error(format("Window {} not found in windows\n", fmt::ptr(window)));
  }

  end:
  return result;
}

Updates Screen::reshapeWindow(Window *window, const Rectangle &area) {
  vector<Fragment> updates;
  if (window->area() != area) {
    const auto zpos = find(zorder.begin(), zorder.end(), window);
    if (zpos == zorder.end()) {
      throw range_error(format("Window {} not found", fmt::ptr(window)));
    }
    const auto z = std::distance(zorder.begin(), zpos);
    auto damageAreas{window->area() - area};
    damageAreas.push_back(area);
    const auto searchArea = window->area() | area;
    window->move(area);
    // Recreate fragments
    for (auto j = z; j >= 0; --j) {
      if (zorder[j]->area().intersects(searchArea)) {
        // Window fragments
        const auto window = zorder[j];
        vector<Rectangle> fragments{window->area()};
        for (auto i = j + 1; i < int(zorder.size()); ++i) {
          split(window->area(), fragments, zorder[i]->fragments);
        }
        window->fragments = fragments;
        // Screen update fragments
        for (const auto &fragment: fragments) {
          for (const auto &damageArea: damageAreas) {
            const auto intersection = fragment & damageArea;
            if (intersection) {
              updates.push_back(Fragment{intersection.value(), window});
            }
          }
        }
      }
    }
  }
  return screenUpdates(updates);
}

Updates Screen::fill(Window *window, const Char &fillChar) {
  window->fill(fillChar);
  return screenUpdates(fragments_of(window, window->fragments));
}

Updates Screen::text(Window *window, const Vector &position, const Text &text) {
  window->write(position, text);
  // This implementation generates just one update for the whole text which in the end
  // is a rectangle. Shorter lines will therefore be filled with unchanged content.
  // However, the screen update algorithm will straighten these unnecessary updates out.
  // An alternative would be to split the updates line by line, or better, blocks of
  // consecutive lines having the same length.
  auto textArea{Rectangle(position, position + text.size())};
  return screenUpdates(
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
  const auto area = window->line(line, strength, dash, roundedCorners) + window->area().position();
  // Screen updates have the same limitations as in @c Screen::text
  return screenUpdates(
    window->fragments |
    views::transform([&area](const Rectangle &r) {return r & area;}) |
    views::remove_if([](const std::optional<Rectangle> &r) {return !r;}) |
    views::transform([window](const std::optional<Rectangle> &r) {return Fragment{r.value(), window};}) |
    to<std::vector<Fragment>>()
  );
}

Updates Screen::box(Window *window, const Box &box) {
  const auto areas = window->box(box);
  vector<Fragment> updates;
  for (const auto &fragment: window->fragments) {
    for (const auto &area: areas) {
      const auto update = fragment & area + window->area().position();
      if (update) {
        updates.push_back(Fragment{update.value(), window});
      }
    }
  }
  return screenUpdates(updates);
}

Vector Screen::relative(Window *window, const Vector &position) const {
  const auto &area{window->area()};
  if (position.x < 0 or position.x > area.width() or position.y < 0 or position.y > area.height()) {
    throw range_error(format("Position outside window dimensions {}: {}", string(area.size()), string(position)));
  }
  return position + area.position();
}

Window * Screen::operator[](int windowId) const {
  auto &windowPtr{windows[windowId]};
  if (!windowPtr) {
    throw domain_error(format("Invalid Window ID {}", windowId));
  }
  return windowPtr.get();
}

int Screen::operator[](const Window *window) const {
  int i{0};
  for (const auto &windowPtr: windows) {
    if (windowPtr.get() == window) {
      return i;
    }
    ++i;
  }
  throw domain_error("Invalid Window *");
}

Updates Screen::resize(const Vector &size) {
  return reshapeWindow(screen, Rectangle{Vector{0, 0}, size});
}

Vector Screen::minSize() const {
  Vector result{0, 0};
  for (auto &window: windows) {
    if (window and window.get() != screen) {
      result = max(result, window->position + window->size());
    }
  }
  return result;
}
