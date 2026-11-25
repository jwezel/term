@page adr_window_management Windows are moved and resized by their frame

**Status:** accepted.

## Context

Interactive applications need to move and resize windows, but a terminal has no window manager. Every application
that wanted to drag a window would have to duplicate hit-testing, mouse capture, and geometry clamping.

## Decision

A window with a frame manages itself with the mouse. A press on the title bar drags the window; a press on the left,
right, or bottom border resizes it from that edge, and a corner resizes both axes. The title bar is the frame above the
client area, so thick title bands drag as well. The window forwards the press to its input handler first, which keeps
click-to-focus and popup dismissal working, then starts the gesture. Motion and release stay with the window through
the terminal's mouse capture. A gesture keeps the window within the terminal's maximum size and leaves at least one
client cell; dragging never shrinks or re-anchors the window.

An undecorated window has no frame, so it keeps its area; transients and popups are likewise left alone.

## Consequences

- Window management works for every framed window without application code.
- `TerminalInterface::maxSize()` is the bound for gestures, while `size()` remains the current display size.
- A plain `jwezel::Window` without insets has no border to grab.
