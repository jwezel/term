@page adr_layout_updates Layout propagation and zero-width text

**Status:** accepted.

## Context

Content changes happen after construction, outside the box that has to re-layout. A layout can also give an element a
zero width before later assigning a real one.

## Decision

`Element::layoutChanged()` bubbles to the nearest `Box`; boxes update their children and expose their preferred size.
Concrete widgets notify after construction and after content changes that affect size. `TextArea` has explicit
constructors because its `acceptsEnter()` policy is not ready during `TextInput` base construction. `Text::extend()`
must widen existing zero-width lines.

## Consequences

- Widgets re-layout without the application knowing which box owns them.
- Construction order matters when a policy is virtual.
- A zero-sized layout slice is not a final size; text has to grow when the slice does.
