@page adr_window_content Widgets on the window's own content surface

**Status:** accepted.

## Context

An earlier design gave every window a separate element device and client surface. That added a second coordinate space
and made clipping, focus, and popups harder to reason about.

## Decision

Widgets are elements on the window's own content surface, offset by the decoration insets. `bounds()` is
content-relative, while `TextElement::area()` is window-relative. The window clips its content, so a widget cannot paint
beyond the frame. `jwezel::Window::clientInsets()` reshapes the content area and fires `geometryEvent`, and the root
re-lays its children. Popups open inside the same window (`PopupPlacement::inside`, the default, clipped and flipped
above the control when needed) or in a transient window (`PopupPlacement::window`).

## Consequences

- There is one surface per window; no separate client device or widget placement object.
- A window frame is drawn below its content and never participates in widget coordinates.
- Popup clipping and flipping are the placement strategy's job, not the widget's.
