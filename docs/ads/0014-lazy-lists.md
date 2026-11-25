@page adr_lazy_lists Lazy list sources and choice events

**Status:** accepted.

## Context

Lists can be large or computed, so a widget must not copy or eagerly scan every row. Single-choice and multi-choice
interaction also need different notifications.

## Decision

`ListSource<Index>` is a value adapter over a lazy source, a vector view, a function pair, or a runtime interface; the
signed `size()` type is the index type. `ListView<Index>` owns scrolling, current row, and selection mechanics and asks
the source only for rows it renders or measures. `ListBox<Index>` adds single or multiple selection. A single-choice
change emits `SelectEvent`; a multi-selection delta emits `SelectionEvent`; activation is a separate `ActivateEvent`.
Programmatic calls stay silent.

## Consequences

- Preferred-size scans are bounded (256 rows by default, eight visible rows).
- Widgets can present data they do not own.
- Observers distinguish choosing a value from changing a selected set.
