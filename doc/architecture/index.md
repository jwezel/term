@page architecture Architecture overview

The library separates terminal mechanics from interface composition. The terminal layer is usable without the UI layer;
the UI layer composes those same surface and display primitives into widgets.

## Terminal layer

`Terminal` coordinates the `Display`, `Keyboard`, terminal-sized `Surface`, and terminal windows. `Display` is the
only component that owns terminal output and screen/cursor control. `Surface` stores and composes drawable elements;
it does not decide what a widget means or how input should be handled.

## UI layer

`Ui` binds the terminal to UI roots. A `ui::Window` is both a terminal window and a `Root`; widgets are elements in a
non-owning parent tree. Containers manage relationships and boxes compute geometry. The UI window translates terminal
focus, geometry, and input events into the widget tree.

## Event path

Keyboard and mouse input is decoded by `Keyboard` and read by `Terminal`. The terminal routes it to the focused
window, which routes it through the focused widget and its ancestors. A handler that returns `true` consumes an input
event. When input is not consumed, window and widget `Action`s can trigger shortcuts such as Enter and Escape.
Typed widget events are emitted separately for observers of actions such as a button press or text acceptance.

## List and menu path

List data is provided through `ListSource<Index>`, a value adapter over a lazy source, vector, function pair, or
runtime interface. `ListView<Index>` owns the scrolling/current/selection mechanics and asks the source only for rows
needed to render or measure it. `ListPopup` is the transient one-choice view shared by `DropDown` and `Menu`; a
`ListBox` keeps its view visible and handles multi-item selection itself. Single-choice changes use `SelectEvent`,
multi-selection deltas use `SelectionEvent`, and activation is an independent `ActivateEvent`. A menu closes its popup
before dispatching the selected item's weakly referenced `Action`.

## Rendering path

Widgets render `Text` and `Char` cells to their surface areas. Layout and geometry changes mark the affected regions;
the display update mechanism writes only the necessary terminal changes. This keeps rendering, terminal control, and
application behavior in separate layers.
