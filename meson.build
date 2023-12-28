project(
  'term',
  'cpp',
  version: '0.1',
  license: 'Apache-2.0',
  default_options: ['warning_level=3', 'cpp_std=c++23', 'buildtype=debug']
)

add_project_arguments('-Wno-parentheses', language: ['c', 'cpp'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_TERM']

sources = [
  'src/display.cc',
  'src/geometry.cc',
  'src/keyboard.cc',
  'src/string.cc',
  'src/surface.cc',
  'src/term.cc',
  'src/text.cc',
  'src/window.cc',
  #'src/ui/container.cc',
  #'src/ui/element.cc',
  #'src/ui/ui.cc',
  #'src/ui/visible_element.cc',
  #'src/ui/widget.cc',
  #'src/ui/window.cc',
  'src/util/error.cc',
  #'extern/Taitank/src/taitank_cache.cc',
  #'extern/Taitank/src/taitank.cc',
  #'extern/Taitank/src/taitank_config.cc',
  #'extern/Taitank/src/taitank_flex.cc',
  #'extern/Taitank/src/taitank_flexline.cc',
  #'extern/Taitank/src/taitank_node.cc',
  #'extern/Taitank/src/taitank_style.cc',
  #'extern/Taitank/src/taitank_util.cc',
]
headers = [
  'src/basic.hh',
  'src/display.hh',
  'src/geometry.hh',
  'src/keyboard.hh',
  'src/string.hh',
  'src/surface.hh',
  'src/term.hh',
  'src/text.hh',
  'src/window.hh',
  'src/util/error.hh',
  #'include/ui/container.hh',
  #'include/ui/element.hh',
  #'include/ui/ui.hh',
  #'include/ui/visible_element.hh',
  #'include/ui/widget.hh',
  #'include/ui/window.hh',
  #'extern/Taitank/src/taitank_cache.h',
  #'extern/Taitank/src/taitank_config.h',
  #'extern/Taitank/src/taitank_flex.h',
  #'extern/Taitank/src/taitank_flexline.h',
  #'extern/Taitank/src/taitank.h',
  #'extern/Taitank/src/taitank_node.h',
  #'extern/Taitank/src/taitank_style.h',
  #'extern/Taitank/src/taitank_util.h'
]

incdir = include_directories([
  'src',
  'src/util',
  #'extern',
  #'extern/Taitank/src'
])

lib_dep = [
  dependency('utfcpp'),
  dependency('rxcpp'),
  dependency('tl-optional'),
  dependency('boost'),
]

shlib = shared_library(
  'term',
  sources,
  install: true,
  cpp_args: lib_args,
  gnu_symbol_visibility: 'default',
  dependencies: lib_dep,
  include_directories: incdir
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
  link_with: shlib,
  dependencies: [test_dep, lib_dep],
  include_directories: incdir
)
test('term', test_exe)

example_exe = executable(
  'ex1',
  'examples/ex1/ex1.cc',
  link_with: shlib,
  dependencies: [lib_dep],
  include_directories: incdir
)

# Make this library usable as a Meson subproject.
term_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with: shlib
)

# Make this library usable from the system's
# package manager.
install_headers(headers, subdir: 'term')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name: 'term',
  filebase: 'term',
  description: 'C++ TUI',
  subdirs: 'term',
  libraries: shlib,
  version: '0.1',
)
