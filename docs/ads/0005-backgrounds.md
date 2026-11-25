@page adr_backgrounds Widget backgrounds inherit the host window

**Status:** accepted.

## Context

Widgets used to need an explicit background to avoid painting the terminal background over their host window.

## Decision

`InheritBackground` is the default for widgets and resolves to the host window's background. An explicit `Space` still
means the terminal background. A UI `jwezel::Window` background remains explicit.

## Consequences

- Widgets blend with their window by default without repeating its color.
- Transparency remains available by asking for `Space` explicitly.
- Windows still declare what they cover, which keeps nested windows readable.
