@page adr_terminal_io Terminal I/O stays inside Display

**Status:** accepted.

## Context

Terminal escape sequences and cursor control are easy to get wrong and hard to test. Widgets that write sequences
directly would duplicate terminal state and make screen restoration unreliable.

## Decision

`jwezel::Display` owns terminal output, cursor control, screen selection, and display updates. Widgets render cells into
their element buffers on a window surface; the surface composes the visible fragments and `Display` turns them into
terminal output. Escape is a logical `Key`, not a raw byte. Nothing outside `Display` writes to the terminal.

## Consequences

- Screen and alternate-screen behavior can be restored predictably.
- Tests drive a fake terminal instead of parsing escape sequences from widgets.
- New terminal capabilities are exposed through `Display` APIs, never emitted from a widget or example.
