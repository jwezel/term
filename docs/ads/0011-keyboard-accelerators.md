@page adr_keyboard_accelerators Keyboard decoding and accelerators

**Status:** accepted.

## Context

Actions need keyboard shortcuts that work the same in menus and widgets, including alt-modified letters, without a
separate modifier field on every key event.

## Decision

The keyboard decodes sequences with a prefix tree and a short window (about 2 ms) for continuation bytes. Printable
keys are their Unicode code points; named and modified keys use distinct `Key` values (`AltLeft`, `CtrlF3`, and so on).
Escape is a logical key on its own; escape followed by a letter within the window decodes to `Key::AltA` through
`Key::AltZ`. `Action` shortcuts are Unicode values, and a menu item's action shortcuts become window accelerators, with
the first shortcut used as the item's hint unless it supplies one.

## Consequences

- Actions and menus share one shortcut mechanism.
- A letter typed within the window of a real Escape is read as Alt+letter, as terminals already do.
- Adding a modifier later means extending the decoder or `Key`, not changing every handler.
