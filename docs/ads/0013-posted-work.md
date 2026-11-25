@page adr_posted_work The UI event loop owns posted work

**Status:** accepted.

## Context

Handlers sometimes must destroy the widget or window that is handling the event, which is unsafe while its event
dispatch is still on the stack.

## Decision

`Ui` owns the event loop. `Terminal::start()` and `running()` expose the loop state, and `Ui::run()` interleaves posted
work between events: work posted during an event runs after that event and before the next one. Callbacks that close a
dialog or return focus use `Ui::post` instead of doing it inline.

## Consequences

- A widget can finish its event handler before it is destroyed.
- Work posted while the queue is draining waits for the following event.
- The demo's dialog and notice callbacks follow this pattern.
