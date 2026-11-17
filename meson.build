project(
  'term',
  'cpp',
  version: '0.2',
  license: 'Apache-2.0',
  default_options: ['warning_level=3', 'cpp_std=c++23', 'buildtype=debug']
)

add_project_arguments('-Wno-parentheses', language: ['c', 'cpp'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_TERM']

sources = [
  'src/term/display.cc',
  'src/term/geometry.cc',
  'src/term/keyboard.cc',
  'src/term/surface.cc',
  'src/term/term.cc',
  'src/term/text.cc',
  'src/term/window.cc',
  'src/ui/box.cc',
  'src/ui/button.cc',
  'src/ui/color_style.cc',
  'src/ui/container.cc',
  'src/ui/decoration.cc',
  'src/ui/drop_down.cc',
  'src/ui/element.cc',
  'src/ui/frame.cc',
  'src/ui/label.cc',
  'src/ui/list_popup.cc',
  'src/ui/root.cc',
  'src/ui/text_input.cc',
  'src/ui/ui.cc',
  'src/ui/ui_style.cc',
  'src/ui/widget.cc',
  'src/ui/window.cc',
  'src/util/error.cc',
  'src/util/string.cc'
]
headers = [
  'src/term/display.hh',
  'src/term/geometry.hh',
  'src/term/keyboard.hh',
  'src/term/mouse_capture.hh',
  'src/term/surface.hh',
  'src/term/term.hh',
  'src/term/text.hh',
  'src/term/window.hh',
  'src/ui/box.hh',
  'src/ui/button.hh',
  'src/ui/color_style.hh',
  'src/ui/container.hh',
  'src/ui/decoration.hh',
  'src/ui/drop_down.hh',
  'src/ui/element.hh',
  'src/ui/frame.hh',
  'src/ui/horizontal_box.hh',
  'src/ui/label.hh',
  'src/ui/line_edit.hh',
  'src/ui/list_popup.hh',
  'src/ui/root.hh',
  'src/ui/text_area.hh',
  'src/ui/text_input.hh',
  'src/ui/ui.hh',
  'src/ui/ui_style.hh',
  'src/ui/widget.hh',
  'src/ui/window.hh',
  'src/util/basic.hh',
  'src/util/error.hh',
  'src/util/string.hh',
  'src/util/xxh32.hpp',
  'src/util/xxh64.hpp'
]

lib_incdir = include_directories([
  'src'
])

lib_dep = [
  dependency('utfcpp'),
  dependency('frozen', include_type: 'system'),
  # dependency('tl-optional'),
]

shlib = shared_library(
  'term',
  sources,
  install: true,
  cpp_args: lib_args,
  dependencies: lib_dep,
  gnu_symbol_visibility: 'default',
  include_directories: lib_incdir
)

test_dep = [
  dependency('doctest'),
]

test_exe = executable(
  'test_term',
  'test/test_display.cc',
  'test/test_geometry.cc',
  'test/test_keyboard.cc',
  'test/test_surface.cc',
  'test/test_term.cc',
  'test/test_text.cc',
  'test/test_window.cc',
  'test/ui/test_style.cc',
  'test/ui/test_ui.cc',
  link_with: shlib,
  dependencies: [test_dep, lib_dep],
  include_directories: lib_incdir
)
test('term', test_exe)

example_exe = executable(
  'ex1',
  'examples/ex1/ex1.cc',
  link_with: shlib,
  dependencies: [lib_dep],
  include_directories: lib_incdir
)

# Make this library usable as a Meson subproject.
term_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with: shlib
)

# Make this library usable from the system's
# package manager.
install_headers(headers, subdir: 'term')
install_headers(headers, subdir: 'util')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name: 'term',
  filebase: 'term',
  description: 'C++ TUI',
  subdirs: 'term',
  libraries: shlib,
  version: '0.2',
)
