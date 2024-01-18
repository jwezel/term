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
  'src/ui/container.cc',
  'src/ui/element.cc',
  'src/ui/frame.cc',
  'src/ui/ui.cc',
  'src/ui/widget.cc',
  'src/ui/window.cc',
  'src/util/error.cc',
  'src/util/string.cc'
]
headers = [
  'src/term/display.hh',
  'src/term/geometry.hh',
  'src/term/keyboard.hh',
  'src/term/surface.hh',
  'src/term/term.hh',
  'src/term/text.hh',
  'src/term/window.hh',
  'src/ui/container.hh',
  'src/ui/element.hh',
  'src/ui/frame.hh',
  'src/ui/ui.hh',
  'src/ui/widget.hh',
  'src/ui/window.hh',
  'src/util/basic.hh',
  'src/util/error.hh',
  'src/util/string.hh',
  'src/util/xxh32.hpp',
  'src/util/xxh64.hpp'
]

lib_incdir = include_directories([
  'src',
  'extern/Taitank/src'
])

taitank_sources = [
  'extern/Taitank/src/taitank_cache.cc',
  'extern/Taitank/src/taitank.cc',
  'extern/Taitank/src/taitank_config.cc',
  'extern/Taitank/src/taitank_flex.cc',
  'extern/Taitank/src/taitank_flexline.cc',
  'extern/Taitank/src/taitank_node.cc',
  'extern/Taitank/src/taitank_style.cc',
  'extern/Taitank/src/taitank_util.cc',
]
taitank_headers = [
  'extern/Taitank/src/taitank_cache.h',
  'extern/Taitank/src/taitank_config.h',
  'extern/Taitank/src/taitank_flex.h',
  'extern/Taitank/src/taitank_flexline.h',
  'extern/Taitank/src/taitank.h',
  'extern/Taitank/src/taitank_node.h',
  'extern/Taitank/src/taitank_style.h',
  'extern/Taitank/src/taitank_util.h'
]
taitank_args = ['-w']
taitank_lib = library(
  'taitank',
  taitank_sources,
  install: false,
  cpp_args: taitank_args,
  gnu_symbol_visibility: 'default',
  include_directories: lib_incdir
)

lib_dep = [
  dependency('utfcpp'),
  # dependency('tl-optional'),
  dependency('boost'),
]

shlib = shared_library(
  'term',
  sources,
  install: true,
  cpp_args: lib_args,
  dependencies: lib_dep,
  link_with: taitank_lib,
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
  'test/ui/test_window.cc',
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
