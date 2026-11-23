# Project checkpoint

## Objectives

- Maintain a value-owned C++ terminal UI library: widgets, layouts, events, and
  examples should compose cleanly without Qt-style parent ownership.
- Keep terminal I/O and terminal escape-sequence handling behind `Display` APIs;
  preserve the current screen/alternate-screen behavior.
- Keep the demo useful as a showcase of the UI library, including the expanded
  person-and-address dialog in `examples/demo/demo.cc`.

## Requirements

- Prefer value ownership. Do not introduce raw-pointer or `unique_ptr` ownership
  APIs; raw pointers may be used only as documented weak references. Pimpl is
  acceptable.
- Do not write to the terminal outside `Display`. Expose escape-sequence
  behavior through an API rather than emitting sequences from widgets or
  examples.
- Treat Escape as a logical `Key`. Keep text documents separate from their
  rendered UI, and preserve Page Up/Page Down behavior in text widgets.
- Event handlers use `(const Element &, const Event &)`. Widgets emit typed
  events.
- Multi-parameter constructors are not `explicit`; constructors callable with
  one argument keep `explicit` where appropriate.
- `TextInput` document accessors are `contents()` and `contents(string_view)`.
  Keep `TextBuffer::text()` and `AcceptEvent::text()` named `text()`.
- Build with the system compiler (GCC 16.2.1 in this workspace). Leave unrelated
  known warnings alone, including `tmpnam`, the unused `topmost` test helper,
  and `BoxLayout` missing-initializer warnings.
- Preserve user working-tree changes. Do not reset, check out, or discard
  unrelated work. When a commit is requested, use an explicit path list with
  `git commit -- <paths>`.

## Settled design decisions

- **Widget backgrounds:** `InheritBackground` is the default for widgets and
    resolves to the host window's background. Explicit `Space` still means the
    terminal background. A UI `Window` background remains explicit.
- **Window content:** Widgets are elements on the window's own content
    surface, offset by the decoration insets. `bounds()` is content-relative;
    `TextElement::area()` is window-relative. The window clips its content.
    `UiStyle::popupPlacement` controls where a popup opens: `inside` (the
    default) places it in the same window, clipped and flipped above the
    control when needed; `window` opens a transient `jwezel::Window` that
    can extend past the frame. `jwezel::Window::clientInsets()` reshapes
    the content area and fires `geometryEvent` so the root re-layouts.
- **Drop-down captions:** `caption(const Text &)` / `caption()` sets a fixed
    closed-state caption. A non-empty caption displays regardless of selection;
    an empty caption shows the selected item. It does not alter items,
    selection, select events, or the open list. Preferred width accounts for
    the widest item or caption plus glyph padding.
- **Box layouts:** `BoxLayout::sizes` selects `LayoutSize::fit` or `stretch`;
    `alignments` selects `LayoutAlign::stretch`, `begin`, `center`, or `end`.
    Fit sizing uses preferred size on the main axis, non-stretch alignment uses
    preferred cross size, leftover space stays at the end, and earlier fit
    children receive available space first on overflow.
- **Layout updates:** `Element::layoutChanged()` bubbles to the nearest `Box`;
    boxes update their children and expose preferred size. Concrete widgets
    notify after construction and after content changes. `TextArea` has
    explicit constructors because its `acceptsEnter()` policy is not ready
    during `TextInput` base construction.
- **Zero-width text:** `Text::extend` must widen zero-width existing lines;
    zero-sized layout slices can later be assigned a nonzero width.
- **Focus tests:** When a test creates a second window and later expects the
    first to render active, restore focus explicitly in the test. Do not change
    `Terminal::focus` semantics just to satisfy that test.
- **Fit clamp:** The preferred-size clamp in `Box::layout()` is intentionally
    retained. Removing it is an equivalent mutation because the overflow pass
    clamps fit children to the same available space.
- **Demo contact form:** Use line edits for free-form single-line values,
    drop-downs for choices, and a text area for notes. Keep the birth-date
    format hint (`YYYY-MM-DD`) and ensure the dialog fits the demo's declared
    minimum terminal size (70x22). Accepted form values are collected into a
    value-owned `PersonRecord`.
- **Documentation:** Generate a Doxygen HTML manual and API reference. Keep
    concise API contracts next to declarations in public headers; put
    tutorials, long conceptual explanations, and developer guidance in Markdown
    under `doc/`. Link manual pages with Doxygen page references. Keep docs
    optional in Meson when Doxygen is unavailable.
- **README teaser:** Keep a very short, complete, compiled `quick_start` example
    in the README Usage section, followed immediately by a screenshot captured
    from the running program. The source of the snippet is
    `examples/quick_start/quick_start.cc`.

## Development and verification workflow

- After each development step, build the release configuration as well as the
  normal build. Put the release tree directly in `./release`:
  - `timeout 900 ninja -C release`
  - `timeout 300 ./release/test_term`
- Normal build and tests:
  - `timeout 600 ninja -C build`
  - `timeout 300 ./build/test_term`
- The ASan/UBSan tree is `/tmp/opencode/build-asan`; when relevant, run it with
  `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1`.
- Use pty checks for interactive demo behavior and test both the normal 90x28
  terminal and the demo's 70x22 minimum when changing its layout.
- When Doxygen is installed, generate and inspect the manual with `timeout 600
  ninja -C build docs`; HTML goes to `build/html/`, with Doxygen diagnostics in
  `build/doxygen-warnings.log`.
- Commit using conventional commits
- Conceptualize everything. Think about whether the feature just developed could
  be used somewhere else. Try to make a concept out of it and apply that
  concept to similar parts of the software.
- Format code with structured indentation (https://structured-indentation.org)

## Current checkpoint (2026-11-23)

- The working tree has an uncommitted refactor: widgets are elements on the
  window's own content surface (no separate `ElementDevice` client), offset
  by decoration insets. `jwezel::Window` stores insets and fires
  `geometryEvent` when they change. Popups open inside the window by default
  (`PopupPlacement::inside`, with flip-above), or as a transient
  `jwezel::Window` (`PopupPlacement::window`). `WidgetPlacement` is removed.
  `Insets` moved to `geometry.hh`. New files: `popup_window.{hh,cc}`.
- The normal, ASan/UBSan, and release test suites each pass with 10,443
  assertions, and `ninja -C build docs` reports zero Doxygen warnings. The
  demo pty checks cover the menu popup, dialog cancellation, Tab navigation,
  typing into fields, saving, and the 70x22 minimum terminal size. The
  Country dropdown in the demo now stays inside its dialog window.
