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

## Lazy list sources and list widgets

A list source reports its count from `size()` and provides each row with either `item(index)` or `operator[](index)`.
`ListSourceLike` accepts either spelling; if both are present, `item()` is used. The signed return type of `size()` is
the source's index type, so `ListBox<i1>` can use an `i1` source and a larger list can choose `i2` or the default
`ListIndex` (`i8`). Both source and widget APIs support `item(index)` and `operator[](index)`.

`listSource(source)` adapts a concept-based source or a `ListSourceInterface` by reference; the original object must
outlive its widget. `listSource(items)` is a non-owning vector view. `ownedListSource(items)` keeps a copy of the vector
alive with the source. A pair of size/item functions can be wrapped directly with `listSource(size, item)`. The widgets
store this small `ListSource<Index>` value and request only the rows they need. Preferred width scans at most 256 rows
by default, and preferred height is capped at eight rows; both limits can be changed.

`ListView<Index>` provides the shared scrolling and selection state. `ListPopup<Index>` uses it for transient choices;
`DropDown` stays non-templated. `ListBox<Index>` is a focusable persistent list and can use single or multiple
selection. In multiple mode, `Selection<Index>` gives the sorted selected indices, while the current row is tracked
separately. `Menu` also stays non-templated: its items refer weakly to application-owned `Action`s, and choosing an
enabled item closes the popup before triggering that action.

The overall `UiStyle::itemSelectionStyle` controls how a list box marks its current row: `ItemSelectionStyle::caret`
shows the configured `listMarker` (🢒), while `ItemSelectionStyle::highlight` highlights the full row. Highlight is the
default. A particular list can override the overall choice with `ListBox::itemSelectionStyle()` and return to the
overall style with `useUiItemSelectionStyle()`. Independently of this current-row style, every selected row uses the
`list.selected` color role. The current row has its own role, `list.current`, and when it is also selected it uses
`list.current.selected`, so keyboard position remains visible within a multi-selection.

### Multi-selection interaction

Multiple selection keeps the keyboard-current row, the selected set, and a range anchor separate. This follows familiar
file-picker/list conventions:

- **Arrow keys** move the current row without changing selection. **Space** selects only the current row, replacing
  any previous selection. **Ctrl+click** is the explicit way to toggle one item while preserving other selections.
- **Shift+arrow** extends an inclusive range from the anchor through the current row. The first extension selects the
  anchor as well as the row reached; extending or shrinking updates that range without dropping pre-existing
  selections outside it. **Shift+click** uses the same anchored range behavior.
- A plain click selects just that item. An unmodified drag replaces the selection with the rows crossed;
  **Ctrl-drag** extends the anchored range while preserving selected rows outside that range. **Ctrl+click** toggles
  one item without replacing the rest of the selection. **Shift+click** also selects an inclusive anchored range.
- **Ctrl+A** selects all enabled items. **Escape** clears a non-empty selection; when nothing is selected, Escape is
  left for the window's cancel/close action.

Every user-driven change emits one `SelectionEvent` per changed item, in ascending index order. Programmatic
`selectAll()`, `deselect()`, `clearSelection()`, `toggle()`, and range operations remain silent, like the other
programmatic selection APIs.

### `SelectEvent` and `SelectionEvent` are different

These event names are deliberately distinct because they describe different contracts:

| Event | Emitted by | Meaning |
| --- | --- | --- |
| `SelectEvent` | `DropDown`; `ListBox` in single mode | A single-choice item was chosen or the one selected row changed. It carries `index`, `previous`, `item`, and `changed()`. A drop-down reports choosing the current item again too; `changed()` tells whether the index actually differs. |
| `SelectionEvent` | `ListBox` in multiple mode | One item was added to or removed from the selection. It carries the changed `index`, the new `selected` state, and `item`; it is a delta, not the whole set. |
| `ActivateEvent` | `ListBox` | The current item was activated with Enter. Activation does not change selection. |

A range gesture emits one `SelectionEvent` for every changed index, in ascending order. Query `ListBox::selection()` for
the full selection. Programmatic calls such as `select()`, `toggle()`, `clearSelection()`, and `refresh()` are silent;
they do not synthesize user-input events. See [`ListBox`](\ref jwezel::ui::ListBox), [`SelectEvent`](\ref
jwezel::ui::SelectEvent), and [`SelectionEvent`](\ref jwezel::ui::SelectionEvent) in the API reference.

## Terminal I/O boundary

`jwezel::Display` owns terminal output, cursor control, screen selection, and display updates. Widgets write cells to
the display surface; they do not emit ANSI sequences or write directly to standard output. Keeping this boundary
centralized makes terminal state restoration predictable and testable.
