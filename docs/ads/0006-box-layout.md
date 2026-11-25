@page adr_box_layout Box sizing, alignment, and the fit clamp

**Status:** accepted.

## Context

Boxes need to mix content-sized children with children that share the remaining space, and to align children on the
cross axis.

## Decision

`BoxLayout::sizes` selects `LayoutSize::fit` or `LayoutSize::stretch`; `alignments` selects `LayoutAlign::stretch`,
`begin`, `center`, or `end`. Fit sizing uses preferred size on the main axis; non-stretch alignment uses preferred
cross size. Leftover space stays at the end. On overflow, earlier fit children receive available space first. The
preferred-size clamp in `Box::layout()` is retained: removing it is an equivalent mutation because the overflow pass
clamps fit children to the same space.

## Consequences

- A layout is deterministic from preferred sizes, weights, and the available area.
- Stretching children absorb slack; fit children keep their content size unless space runs out.
- The clamp is not redundant bookkeeping; tests should treat it as part of the layout contract.
