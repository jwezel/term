@page introduction Introduction

`term` provides two layers. The lower layer models terminal cells, text, windows, input, and display updates. The UI
layer composes those primitives into widgets and layouts. Applications keep their data and event loop; the library
owns terminal rendering and translates input into events.

## The mental model

- A `jwezel::Terminal` coordinates the display, keyboard, and terminal-sized surface.
- A `jwezel::ui::Ui` exposes that terminal to the UI layer. It supplies the desktop and a `run()` / `stop()` loop.
- A `jwezel::ui::Window` is a root for UI elements. Its children use coordinates relative to the window's content and
  are clipped to it; the window itself is positioned on the terminal display.
- A `jwezel::ui::Widget` is an element that paints on a surface and can participate in focus and input handling.
- A `jwezel::ui::Container` holds child elements. A `jwezel::ui::Box` lays out its children along one axis.
- Events flow from the terminal to the focused window, through the focused widget, and then to registered handlers
  and actions that were not consumed by a widget.

The ownership model follows ordinary C++ object lifetimes: application objects are declared as values, and parents
keep non-owning references to their children. Destroy children before the container they refer to. The widgets do not
delete themselves through a parent-child ownership scheme.

## UI and terminal are separate concerns

Widgets draw through the display abstraction rather than writing escape sequences. This lets the same application
use the normal screen, which preserves terminal history, or the alternate screen, which is restored on exit. Use the
`jwezel::Display::Screen` option on `jwezel::Terminal` to choose the behavior; widgets do not choose or manipulate
terminal modes themselves.

The UI is intentionally not an application framework. It does not impose a model, a document type, an application
class, or a storage policy. A `jwezel::ui::TextBuffer` stores editable text separately from a text widget; applications
can keep other data in their own types.

## Continue reading

- Follow @ref tutorial "the tutorial" to create a window, arrange widgets, and handle input.
- Read @ref concepts "core concepts" for geometry, ownership, layout, colors, and events.
- See @ref architecture "the architecture overview" and @ref developer "the developer's manual" for contribution
  and extension guidance.
