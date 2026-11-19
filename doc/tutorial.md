@page tutorial Tutorial

This tutorial starts with a complete, compiled program, then explains how to extend it. The source is
`examples/quick_start/quick_start.cc`; the README contains the same complete program and a screenshot.

## 1. Build and run

After installing dependencies and configuring the build as described in the [README Usage section](../README.md#usage),
build and run the example:

```sh
meson compile -C build quick_start
./build/quick_start
```

The program creates a `jwezel::Terminal`, wraps it in `jwezel::ui::Ui`, and creates a window with a label and a button.
The button receives focus. Its `onPress()` handler stops the UI loop; Enter or a mouse click can press it. The label
is passive and does not take focus.

The terminal, UI, window, and widgets are all local objects. Their declaration order is deliberate: the children are
destroyed before the window and the terminal they use.

@image html quick_start.png "The quick-start window running in a terminal"

## 2. Let a layout assign areas

Explicit rectangles are convenient for the first screen, but boxes make a UI adapt to a changing window size. A
vertical box arranges children from top to bottom; a horizontal box arranges them from left to right. To replace the
quick-start label and button, include `ui/line_edit.hh` and `ui/vertical_box.hh`, then use this block inside the same
`main()` after `window` has been created:

```cpp
#include <ui/line_edit.hh>
#include <ui/vertical_box.hh>

jwezel::ui::VerticalBox column{
  window,
  jwezel::Rectangle{2, 2, 38, 9},
  jwezel::ui::BoxLayout{.sizes = {jwezel::ui::LayoutSize::fit}, .spacing = 1}
};
jwezel::ui::Label heading{column, jwezel::Text{"Contacts"}};
jwezel::ui::LineEdit search{column};
jwezel::ui::Button add{column, jwezel::Text{"Add"}};
add.onPress([&ui](const jwezel::ui::Element &, const jwezel::Event &) {
  ui.stop();
  return true;
});
```

The heading requests its preferred size; the line edit and button stretch to share the remaining height. Passing
`jwezel::RectangleDefault` as a widget's area explicitly requests parent-managed sizing.

By default children stretch to fill their slice. Add `LayoutSize::fit` to `BoxLayout::sizes` to size a child along the
box's main axis to its `preferredSize()`. `BoxLayout::alignments` controls its cross-axis placement. See
`jwezel::ui::BoxLayout`, `jwezel::ui::LayoutSize`, and `jwezel::ui::LayoutAlign` in the API reference.

## 3. Handle widget events and actions

Widgets expose typed event helpers such as `Button::onPress`, `DropDown::onSelect`, and `TextInput::onAccept`. General
event handlers use the same `(const Element &, const Event &)` signature and can inspect the typed event with
`event.as<YourEvent>()`.

Use an `Action` when an operation should have both a visible control and a keyboard shortcut. Add the same action to
the button and its window, or use a window action for default Enter / Escape behavior. An action is a value that must
outlive every host to which it is registered.

## 4. Keep application data in the application

Widgets edit and display input; they are not the application's data model. Read a `LineEdit` with `contents()`, or use
`TextArea` with a `TextBuffer` for multi-line documents. Copy values into your own record type when the application
accepts them. This keeps validation, persistence, and domain behavior independent of rendering.

For a larger working example, build `demo` and run `./build/demo`. It includes a menu, a detailed person-and-address
dialog, a text area, focus traversal, and an event log.

\example quick_start.cc
