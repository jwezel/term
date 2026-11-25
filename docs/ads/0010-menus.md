@page adr_menus Menus, menu bars, and submenus

**Status:** accepted.

## Context

A menu is not a drop-down: it is labeled, it can be grouped with other menus, and its items can be structured beyond
plain actions.

## Decision

`MenuItem` describes a `text` with an optional weak `Action`, a right-aligned `hint`, a `checked` mark, a separator,
or a weak submenu. A `MenuBar` does not draw; it orders application-owned menus and puts them in a `FocusGroup` so the
bar is one Tab stop. Left and Right move between menus, open or closed, and an open popup follows. A submenu opens at
its item's row through `PopupChoice::openAt`; the parent keeps the input and forwards it until Left or Escape closes
the child. Choosing a leaf action closes the whole path first. Menu buttons have their own glyphs and color roles
(`UiStyle::menu`, `menuRoles`) without the drop-down arrow.

## Consequences

- Menus, bars, and submenus are plain values; the application keeps them alive.
- Navigation is one concept (`FocusGroup`) and one input owner per open path.
- A bar changes only focus and ordering; rendering stays with the menus.
