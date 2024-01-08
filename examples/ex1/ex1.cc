#include <util/basic.hh>
#include <term/display.hh>
#include <term/event.hh>
#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <term/term.hh>
#include <term/text.hh>

#include <format>
#include <memory>
#include <unistd.h>
#include <vector>

using
  jwezel::bold,
  jwezel::Box,
  jwezel::Char,
  jwezel::Dim,
  jwezel::Down,
  jwezel::Home,
  jwezel::Left,
  jwezel::AttributeMode::mix,
  jwezel::PageDown,
  jwezel::PageUp,
  jwezel::Rectangle,
  jwezel::RgbBlue5,
  jwezel::RgbCyan2,
  jwezel::RgbGray2,
  jwezel::RgbGray7,
  jwezel::RgbNone,
  jwezel::RgbRed,
  jwezel::RgbWhite,
  jwezel::Right,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::toDim,
  jwezel::u4,
  jwezel::Up,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin;

using std::vector;
using std::format;
using std::string;

const Dim
  wi = 50,
  hi = 20;

void moveWindow(Terminal &term, jwezel::Window &window, const Vector &pos, const Vector &size) {
  term.moveWindow(window, Rectangle{pos.x(), pos.y(), toDim(pos.x() + size.x()), toDim(pos.y() + size.y())});
}

auto main(int /*unused*/, char * /*unused*/[]) -> int {
  auto result = 0;
  const Vector si{wi, hi};
  Terminal term{Char(L' ', RgbNone, RgbCyan2), VectorMin, VectorMin, VectorMax, 1, 0, true, false};
  term.display().mouseMode(jwezel::Display::MouseMode::anything);
  vector<std::unique_ptr<jwezel::Window>> ws;
  u4 cw = 0;
  ws.push_back(std::make_unique<jwezel::Window>(&term, Rectangle{0, 0, wi, hi}, Char(' ', RgbNone, RgbBlue5)));
  ws[cw]->box(Box{{Vector{0, 0}, VectorMax}, 2});
  ws[cw]->write(Vector{1, 0}, Text(format("Window {}", ws.size()), RgbRed, RgbGray7, bold, mix));
  term.desktop().write(Vector{0, 0}, Text("Desktop", RgbWhite, RgbNone, {}, mix));
  while (true) {
    auto *event{term.event()};
    if (event->vid() == jwezel::KeyEvent::id()) {
      auto *kbEvent{dynamic_cast<jwezel::KeyEvent *>(event)};
      switch (kbEvent->key()) {

        case Up:
        case '8':
        {
          auto rec{ws[cw]->area()};
          rec.up();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case Down:
        case '2':
        {
          auto rec{ws[cw]->area()};
          rec.down();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case Left:
        case '4':
        {
          auto rec{ws[cw]->area()};
          rec.left();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case Right:
        case '6':
        {
          auto rec{ws[cw]->area()};
          rec.right();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case '7':
        {
          auto rec{ws[cw]->area()};
          rec.left();
          rec.up();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case '1':
        {
          auto rec{ws[cw]->area()};
          rec.left();
          rec.down();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case '9':
        {
          auto rec{ws[cw]->area()};
          rec.right();
          rec.up();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case '3':
        {
          auto rec{ws[cw]->area()};
          rec.right();
          rec.down();
          moveWindow(term, *ws[cw], Vector{rec.x1(), rec.y1()}, si);
        }
        break;

        case Home:
        case '5':
        {
          moveWindow(term, *ws[cw], Vector{0, 0}, si);
        }
        break;

        case 'q':
        goto end;

        case jwezel::Unicode('w'):
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray2, bold, mix));
        ws.push_back(std::make_unique<jwezel::Window>(&term, ws[cw]->area() + Vector{1, 1}));
        cw = ws.size() - 1;
        ws[cw]->box(Box{});
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray7, bold, mix));
        break;

        case PageDown:
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray2, bold, mix));
        cw = (cw + 1) % ws.size();
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray7, bold, mix));
        break;

        case PageUp:
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray2, bold, mix));
        cw = (cw - 1);
        if (cw >= ws.size()) {
          cw = ws.size() - 1;
        }
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray7, bold, mix));
        break;

        case jwezel::Delete:
        ws.erase(ws.begin() + cw);
        if (cw >= ws.size()) {
          if (cw-- == 0) {
            if (!ws.empty()) {
              ++cw;
            } else {
              goto end;
            }
          }
        }
        ws[cw]->write(Vector{1, 0}, Text(format("Window {}", cw + 1), RgbRed, RgbGray7, bold, mix));
        break;

        case 'd':
        ws[cw]->below(nullptr);
        break;

        case 'D':
        ws[cw]->above(1);
        break;

        case 'u':
        ws[cw]->above(nullptr);
        break;

        case 'U':
        ws[cw]->above(-1);
        break;

        default:
        break;
      }
    } else if (event->vid() == jwezel::MouseButtonEvent::id()) {
      auto *mbEvent{dynamic_cast<jwezel::MouseButtonEvent *>(event)};
      static vector<string> mouseEvent{
        "Nothing", "Mouse button 1 clicked", "Mouse Button 2 clicked", "Mouse button 3 clicked", "Mouse moved"
      };
      static vector<string> mouseState{
        "released", "pressed"
      };
      ws[0]->write(
        Vector{1, 1},
        format(
          "Event: {:22s}\nColumn: {:3d}\nLine: {:3d}\nState: {}\n"
          "Shift: {}\nControl: {}\nAlt: {}\nMod 4: {}\nMod 5: {}\nMod 6: {}",
          mouseEvent[static_cast<int>(mbEvent->button())],
          mbEvent->column(),
          mbEvent->line(),
          mouseState[static_cast<int>(mbEvent->action())],
          mbEvent->modifiers().shift,
          mbEvent->modifiers().control,
          mbEvent->modifiers().alt,
          mbEvent->modifiers().mod4,
          mbEvent->modifiers().mod5,
          mbEvent->modifiers().mod6
        )
      );
    } else if (event->vid() == jwezel::MouseMoveEvent::id()) {
      auto *mbEvent{dynamic_cast<jwezel::MouseMoveEvent *>(event)};
      ws[0]->write(
        Vector{1, 1},
        format(
          "Event: {:22s}\nColumn: {:3d}\nLine: {:3d}\nState: {}\n"
          "Shift: {}\nControl: {}\nAlt: {}\nMod 4: {}\nMod 5: {}\nMod 6: {}",
          "Mouse moved",
          mbEvent->column(),
          mbEvent->line(),
          "              ",
          "              ",
          "              ",
          "              ",
          "              ",
          "              ",
          "              "
        )
      );
    } else {
      term.display().write("\a");
    }
  }
  end:
  return result;
}
