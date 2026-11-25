@page adr_demo_and_docs The demo and documentation conventions

**Status:** accepted.

## Context

The library needs a working showcase and documentation that stays next to the code it describes.

## Decision

The demo is the showcase: line edits for free-form values, drop-downs for choices, a text area for notes, modal dialogs
for forms and notices, and a menu bar with submenus and accelerators. It must fit the declared minimum terminal size
(70x22). API contracts live beside declarations in public headers; tutorials, rationale, and workflows live as Markdown
under `docs/`. Doxygen generates the manual and API reference, and remains optional in Meson when Doxygen is absent. The
README keeps a very short, complete, compiled quick-start followed by a screenshot from the running program, with the
source in `examples/quick_start/quick_start.cc`.

## Consequences

- The demo exercises the public API rather than private shortcuts.
- Documentation changes do not require a Doxygen installation to be reviewed, but the docs target must stay clean.
- The README teaser cannot drift from a compiled example.
