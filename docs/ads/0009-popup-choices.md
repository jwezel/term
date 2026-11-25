@page adr_popup_choices Popup choices and drop-down captions

**Status:** accepted.

## Context

Drop-downs and menus share opening, navigation, and mouse behavior, but they differ in what choosing an item means and
in what the closed control shows.

## Decision

`ListPopup` is the shared one-choice popup and `PopupChoice` is the shared control behavior. A caption is a fixed
closed-state text: a non-empty caption displays regardless of selection, does not alter items, selection, select
events, or the open list, and preferred width accounts for the widest item or caption plus glyph padding. Placement is
`inside` or `window`. Moving a control closes its popup only when the area actually changes, so a layout pass that
re-affirms the current area keeps the popup open.

## Consequences

- Drop-downs and menus reuse one tested interaction path.
- Captions are presentation, not a synthetic item or selection.
- Non-managed children can be moved by layout without dismissing an open popup.
