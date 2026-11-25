@page adr_surfaces_backdrop Surface fragments and the backdrop

**Status:** accepted.

## Context

Overlapping windows and widgets must redraw only what is visible, without recomputing every element after each change.

## Decision

A `Surface` keeps elements in z-order, bottom to top, and each element has disjoint fragments of the area it shows.
Adding, removing, moving, and reordering update fragments incrementally and tell the device only about the affected
rectangles. The lowest element is the backdrop: it is the terminal background itself and is never painted
(`Surface::isBackdrop`).

## Consequences

- Visible parts of overlapping elements are explicit and testable.
- The backdrop is a positional concept, so it never needs a type test at the call sites.
- A layout pass that moves an element to the area it already has must not invalidate its fragments or popups.
