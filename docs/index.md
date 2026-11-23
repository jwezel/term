# term manual

`term` is a C++23 library for building terminal interfaces from value-like objects: a terminal display, windows,
widgets, layouts, and typed events. This manual introduces the model, walks through a working program, explains the
main design concepts, and links to the generated API reference.

The quickest way to see the library in action is the [README quick-start program](https://github.com/jwezel/term#usage).
The same program is compiled as the Meson target `quick_start` and included below as a source example.

## Manual

\subpage introduction

\subpage tutorial

\subpage concepts

\subpage extensions

\subpage architecture

\subpage developer

## API reference

The reference is generated from the public headers under `src/term`, `src/ui`, `src/ext_ui`, and `src/util`. The most useful starting
points are `jwezel::Terminal`, `jwezel::Display`, `jwezel::Text`, `jwezel::ui::Ui`, `jwezel::ui::Window`,
`jwezel::ui::Widget`, and `jwezel::ui::Box`.
