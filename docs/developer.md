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
- `src/ext_ui/` contains reusable composite widgets built on the core UI library.
- `src/util/` contains shared utilities.
- `examples/` contains standalone programs; `test/` contains doctest-based tests.

Public headers are listed in `headers` in the root `meson.build`. Keep concise API contracts beside declarations in
those headers, and put longer rationale, workflows, and tutorials in `docs/`.

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

## Code formatting

Format C++ with [structured indentation](https://structured-indentation.org): 2 spaces per level, at most 128
columns per line, no tabs, no trailing whitespace, and no alignment padding. The structure of an expression should be
visible from its indentation:

- A context that fits on one line stays on one line. Otherwise it opens at the end of its first line, its contents
  are indented one level, and its closing token returns to the level of the opening line.
- Put the items of a multi-line argument list, braced list, or initializer list one per line; do not pack several
  items onto a shared line. Keep a trailing comma after the last item so later additions do not touch the line above.
- Align a constructor's base and member initializers with the constructor declaration, one per line, and dedent the
  body brace to the same level. Keep an empty body as `{}`.
- Small bodies may stay on one line when they fit, for example `{return value;}` or an empty `{}`.
- Indent access specifiers one level inside a class; their declarations follow at that same level.
- Write `not`, `and`, and `or` instead of `!`, `&&`, and `||`.
- Put project includes before standard-library includes, keep each group alphabetical, and separate the groups with a
  blank line.

A multi-line value follows the same rule: the opening brace ends the first line, the elements are one per line, and
the closing brace lines up with the declaration.

```cpp
const Vector position{
  static_cast<Dim>(area.x1() + (width - Dialog::width) / 2),
  static_cast<Dim>(area.y1() + (height - Dialog::height) / 2)
};
```

## Documentation conventions

Use Doxygen `///` comments for API declarations. Describe purpose and behavior rather than repeating an identifier;
document parameters, return values, and exceptions where they add useful contract information. Document public enum
values and data fields. Use `@ref` to connect related API types. Use Markdown pages in `docs/` for multi-step explanations
and link from the relevant header comment.

The initial API reference extracts every public declaration so users can browse the complete surface while comments are
being filled in. Doxygen warns about malformed commands and links, but `EXTRACT_ALL` necessarily disables its missing-
documentation diagnostics. A clean warning log therefore validates links and markup, not documentation completeness;
new or substantially changed public API still needs substantive header documentation and review.
