@page adr_modal_windows Modal windows take the input exclusively

**Status:** accepted.

## Context

Dialogs must not let the mouse reach windows behind them while they are open, and embedding that check in every widget
would duplicate it.

## Decision

`jwezel::Window` has a `modal` flag, false by default. `Terminal` enforces it while dispatching mouse input: if the
focused window is modal, only that window receives the event; a transient of the modal window may also receive it
because it belongs to the modal one. Applications mark a dialog modal when they open it.

## Consequences

- Modality is a window state, not a property of each control.
- Popups owned by a modal window keep working.
- Keyboard input follows the focused window as before; modality only narrows mouse dispatch.
