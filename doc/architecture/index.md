@page architecture Architecture overview

The library separates terminal mechanics from interface composition. The terminal layer is usable without the UI layer;
the UI layer composes those same surface and display primitives into widgets.

## Terminal layer

`Terminal` coordinates the `Display`, `Keyboard`, terminal-sized `Surface`, and terminal windows. `Display` is the
only component that owns terminal output and screen/cursor control. `Surface` stores and composes drawable elements;
it does not decide what a widget means or how input should be handled. An `ElementDevice` is both a surface element and
a device: it nests the elements drawn into it, clips them to its own area, and composes them above its own drawing.

## UI layer

`Ui` binds the terminal to UI roots. A `ui::Window` is both a terminal window and a `Root`; widgets are elements in a
non-owning parent tree. The root keeps its widgets in an `ElementDevice` that covers the window's content area: the
device nests the widget elements, clips them to the content, and passes their updates on to the window's surface, so a
widget cannot paint over the frame. Widget areas are relative to that content. A widget that must show outside the
window, like a drop-down popup, is placed on the window's own surface instead (see `WidgetPlacement`), where the frame
does not clip it and it is drawn at the display position of its content-relative bounds. Containers manage
relationships and boxes compute geometry. The UI window translates terminal focus, geometry, and input events into
the widget tree.

## Event path

Keyboard and mouse input is decoded by `Keyboard` and read by `Terminal`. The terminal routes it to the focused
window, which routes it through the focused widget and its ancestors. A handler that returns `true` consumes an input
event. When input is not consumed, window and widget `Action`s can trigger shortcuts such as Enter and Escape.
Typed widget events are emitted separately for observers of actions such as a button press or text acceptance.

## List and menu path

List data is provided through `ListSource<Index>`, a value adapter over a lazy source, vector, function pair, or
runtime interface. `ListView<Index>` owns the scrolling/current/selection mechanics and asks the source only for rows
needed to render or measure it. `ListPopup` is the one-choice view shared by `DropDown` and `Menu`; it is a widget
placed above the host window by default, so the list can escape the window's frame, and `UiStyle::popupPlacement`
selects between that and clipping to the window's content. A `ListBox` keeps its view visible and handles multi-item
selection itself. Single-choice changes use `SelectEvent`, multi-selection deltas use `SelectionEvent`, and activation
is an independent `ActivateEvent`. A menu closes its popup before dispatching the selected item's weakly referenced
`Action`.

## Rendering path

Content widgets render `Text` and `Char` cells into the window's `ElementDevice`, which clips them to the content area
and passes updates on to the window. Host-placed widgets, like popup lists, render at their display position on the
window's own surface, above everything the window draws. Layout and geometry changes mark the affected regions;
the display update mechanism writes only the necessary terminal changes. This keeps rendering, terminal control, and
application behavior in separate layers.
