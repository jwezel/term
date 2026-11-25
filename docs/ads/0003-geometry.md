@page adr_geometry Geometry, rectangles, and dimension sentinels

**Status:** accepted.

## Context

Layout needs positions, sizes, and "not set" values without allocating or adding optional wrappers to every geometry
operation.

## Decision

`Vector` is a pair of signed terminal dimensions, conventionally `{column, row}` or `{width, height}`. `Rectangle`
uses half-open bounds `[x1, x2) x [y1, y2)`. `Dim` is a signed 16-bit type; `DimLow` and `DimHigh` are sentinels for
"unset" and "largest", used by `defaultTo()`, `position()`, and `RectangleDefault`. `toDim()` checks the range in
debug builds.

## Consequences

- Empty rectangles are valid and adjacent rectangles do not overlap.
- Sentinels must be resolved with `defaultTo()` before use.
- Layout arithmetic uses `toDim()` so out-of-range results are caught while developing.
