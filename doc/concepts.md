@page concepts Core concepts

## Elements, containers, and lifetime

`jwezel::ui::Element` is the common base for UI objects. A `Container` provides a parent relationship and a place for
children; a `Root` connects a tree to a window and its input/focus state. These relationships are non-owning. Declare
parents before children so children are destroyed first, or otherwise ensure every parent outlives its children.

Widgets are ordinary, non-copyable C++ values. Their constructors register them with their parent, and their
destructors unregister them. Do not put an owning `unique_ptr` or raw owning pointer into a parent container.

## Geometry and preferred size

`jwezel::Vector` represents a position or size; `jwezel::Rectangle` uses half-open bounds `[x1, x2) × [y1, y2)`. UI
widget coordinates are relative to their window's content area. A window's terminal area is relative to the display.
`Element::bounds()` reports the assigned UI area, while `TextElement::area()` is the area rendered on the underlying
surface.

An element may provide a `preferredSize()`. A box uses this when a child opts into `LayoutSize::fit`; otherwise it
divides the available main-axis space between stretching children according to their weights. A child can be aligned
at the beginning, center, or end of the cross axis instead of stretching. `layoutChanged()` notifies the nearest box
when content changes the preferred size.

## Text, cells, and documents

`jwezel::Char` represents a terminal cell: a Unicode code point plus foreground, background, and attributes.
`jwezel::Text` is a collection of lines of cells, and its geometry is measured in terminal columns and rows. The
`jwezel::TextBuffer` used by text-input widgets is a separate editable document; the widget manages cursor, scrolling,
selection, and rendering around that document.

## Colors and backgrounds

Text cells carry their own color and attribute information. Color styles add named UI roles such as `label`, `button`,
and `button.focus`; unspecified parts show through from the underlying cell. A widget's default `InheritBackground`
uses its host window's background. Passing `jwezel::Space` explicitly instead uses the terminal's default background.

## Input, events, and actions

The terminal decodes keys and mouse input into typed events. The focused window routes input to the focused widget.
Input handlers return whether they handled the event; handled input does not continue to other handlers. Widget events
such as `PressEvent`, `SelectEvent`, and `AcceptEvent` are notifications: every registered handler for that typed event
is called.

An `Action` is an operation with optional keyboard shortcuts. It may be hosted by a window or widget and may also be
triggered by a visible control, so keyboard and mouse operation can share one behavior. Its lifetime remains the
application's responsibility.

## Terminal I/O boundary

`jwezel::Display` owns terminal output, cursor control, screen selection, and display updates. Widgets write cells to
the display surface; they do not emit ANSI sequences or write directly to standard output. Keeping this boundary
centralized makes terminal state restoration predictable and testable.
