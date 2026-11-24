@page architecture Architecture overview

The library separates terminal mechanics from interface composition. The terminal layer is usable without the UI layer;
the UI layer composes those same surface and display primitives into widgets.

## Terminal layer

`Terminal` coordinates the `Display`, `Keyboard`, terminal-sized `Surface`, and terminal windows. `Display` is the
only component that owns terminal output and screen/cursor control. `Surface` stores and composes drawable elements;
it does not decide what a widget means or how input should be handled. A `Window` is an `ElementDevice` on the
terminal surface: it nests elements on its own content surface, clips them to its area, and composes them above its
own drawing. The window's client-area insets tell the UI where the frame is, so widgets stay inside the content.

## UI layer

`Ui` binds the terminal to UI roots. A `ui::Window` is both a `jwezel::Window` (the terminal window) and a `Root`
(the UI tree); widgets are elements on the terminal window's own content surface, offset by the content origin that
the `ui::Window`'s decoration insets define. The `ui::Window` draws its frame into the terminal window's own text;
widgets sit above the frame on the same surface. Containers manage relationships and boxes compute geometry. The
`ui::Window` translates the terminal window's focus, geometry, and input events into the widget tree.

## Event path

Keyboard and mouse input is decoded by `Keyboard` and read by `Terminal`. The terminal routes it to the focused
terminal window, whose `ui::Window` root routes it through the focused widget and its ancestors. A handler that
returns `true` consumes an input event. When input is not consumed, `ui::Window` and widget `Action`s can trigger
shortcuts such as Enter and Escape. Typed widget events are emitted separately for observers of actions such as a
button press or text acceptance.

A terminal window can be transient to another one: the transient never takes the terminal focus, and while its owner
has the focus the terminal routes input that lands on the transient to it. This is how a popup's terminal window
receives mouse clicks while the `ui::Window` that owns the popup keeps the focus.

## List and menu path

List data is provided through `ListSource<Index>`, a value adapter over a lazy source, vector, function pair, or
runtime interface. `ListView<Index>` owns the scrolling/current/selection mechanics and asks the source only for rows
needed to render or measure it. `ListPopup` is the one-choice view shared by `DropDown` and `Menu`.
`UiStyle::popupPlacement` controls where it opens: `inside` places it in the same `ui::Window` as its control,
clipped to the content area and flipped above the control when there is not enough room below; `window` opens it in
a transient terminal window of its own, above the owner and not clipped by it. The default is `inside`. A `ListBox`
keeps its view visible and handles multi-item selection itself. Single-choice changes use `SelectEvent`, multi-selection deltas use
`SelectionEvent`, and activation is an independent `ActivateEvent`. A menu closes its popup before dispatching the
selected item's weakly referenced `Action`.

`MenuBar` does not draw: it orders application-owned menus and puts them in a `FocusGroup` in the root, which makes
the bar one Tab stop while Left and Right still move between its members. `Menu` extends the popup with hints, check
marks, separators, and submenus; `PopupChoice::openAt()` places a submenu at its item's row, inside the window, and
the parent keeps the input and forwards it to the open child. The keyboard decoder turns the escape character followed
by a letter into `Key::AltA` through `Key::AltZ`, so Alt+letter accelerators work through the ordinary `Action`
shortcut mechanism.

## Rendering path

Widgets render `Text` and `Char` cells onto the terminal window's content surface; the `ui::Window`'s frame drawing is
below them. Widget areas are offset from content coordinates by the decoration insets, so a widget at content position
(0, 0) sits just inside the frame. Layout and geometry changes mark the affected regions; the display update mechanism
writes only the necessary terminal changes. This keeps rendering, terminal control, and application behavior in
separate layers.
