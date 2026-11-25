@page adr_value_ownership Value ownership and weak references

**Status:** accepted.

## Context

The UI tree needs parent relationships, but Qt-style parent ownership does not fit a library whose objects should be
ordinary values. Ownership expressed with raw owning pointers or `unique_ptr` makes lifetime errors easy and spreads
destruction logic through the tree.

## Decision

UI objects are non-copyable values. Constructors register a child with its parent and destructors unregister it, so a
parent must outlive its children. Ownership is never expressed with an owning raw pointer or `unique_ptr`; a raw
pointer is only a documented weak reference, as in `Action *`, submenus, styles, buddies, and parents. Pimpl is
allowed where it hides an implementation.

## Consequences

- Destruction order is part of the API: declare parents before children, or keep them in a container that preserves it.
- Weak references are cleared by both sides: `Root::forget`, `ActionHost`, and `FocusGroup` forget destroyed widgets.
- Optional windows and dialogs are held as `std::optional<T>` values rather than pointers.
