@page developer Developer's manual

## Build and test

The project uses Meson and Ninja with C++23. Follow the dependency setup in the [README Usage section](../README.md#usage),
then a normal local build and test run is:

```sh
PKG_CONFIG_PATH=packages meson setup build
ninja -C build
./build/test_term
```

The project also provides `quick_start`, `ex1`, and `demo` executables. If Doxygen is installed, generate this manual
and API reference with `ninja -C build docs`; the generated HTML is under `build/html/`. The docs target is optional
when Doxygen is unavailable.

Keep a release build separate from the development build when validating changes:

```sh
PKG_CONFIG_PATH=packages meson setup release --buildtype=release
ninja -C release
./release/test_term
```

## Source map

- `src/term/` implements terminal geometry, cells and text, input decoding, display updates, surfaces, and windows.
- `src/ui/` implements widgets, containers, layout, focus, actions, events, and text input.
- `src/util/` contains shared utilities.
- `examples/` contains standalone programs; `test/` contains doctest-based tests.

Public headers are listed in `headers` in the root `meson.build`. Keep concise API contracts beside declarations in
those headers, and put longer rationale, workflows, and tutorials in `doc/`.

## Adding or changing a widget

1. Derive from the smallest existing base that supplies the needed behavior (`Widget`, `Label`, `TextInput`, or
   `Container`).
2. Keep application data outside the widget. Use value types and non-owning parent relationships.
3. Render through the widget/display APIs; never write terminal escape sequences directly.
4. Give content-sized widgets a meaningful `preferredSize()`, and call `layoutChanged()` after construction or content
   changes that affect it.
5. Use typed events for widget notifications. Preserve the common `(const Element &, const Event &)` callback shape.
6. Add tests for geometry, drawing, input, and lifetime behavior. Add a pty check for interactive examples.
7. Document public classes, enum values, fields, and non-obvious contracts in the header next to the declaration.

## Documentation conventions

Use Doxygen `///` comments for API declarations. Describe purpose and behavior rather than repeating an identifier;
document parameters, return values, and exceptions where they add useful contract information. Document public enum
values and data fields. Use `@ref` to connect related API types. Use Markdown pages in `doc/` for multi-step explanations
and link from the relevant header comment.

The initial API reference extracts every public declaration so users can browse the complete surface while comments are
being filled in. Doxygen warns about malformed commands and links, but `EXTRACT_ALL` necessarily disables its missing-
documentation diagnostics. A clean warning log therefore validates links and markup, not documentation completeness;
new or substantially changed public API still needs substantive header documentation and review.
