project(
  'term',
  'cpp',
  version: '0.1',
  license: 'Apache-2.0',
  default_options: ['warning_level=3', 'cpp_std=c++17', 'debug=true']
)

add_project_arguments('-Wno-parentheses', language: ['c', 'cpp'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_TERM']

sources = [
  'extern/yoga/yoga/YGLayout.cpp',
  'extern/yoga/yoga/event/event.cpp',
  'extern/yoga/yoga/YGStyle.cpp',
  'extern/yoga/yoga/YGNode.cpp',
  'extern/yoga/yoga/YGConfig.cpp',
  'extern/yoga/yoga/YGNodePrint.cpp',
  'extern/yoga/yoga/YGValue.cpp',
  'extern/yoga/yoga/YGEnums.cpp',
  'extern/yoga/yoga/Utils.cpp',
  'extern/yoga/yoga/Yoga.cpp',
  'extern/yoga/yoga/log.cpp',
  'src/display.cc',
  'src/geometry.cc',
  'src/keyboard.cc',
  'src/screen.cc',
  'src/screen_window.cc',
  'src/string.cc',
  'src/term.cc',
  'src/text.cc',
  'src/ui/element.cc',
  'src/ui/ui.cc',
  'src/ui/widget.cc',
  'src/ui/window.cc',
]
headers = [
  'extern/yoga/yoga/YGLayout.h',
  'extern/yoga/yoga/YGNodePrint.h',
  'extern/yoga/yoga/event/event.h',
  'extern/yoga/yoga/log.h',
  'extern/yoga/yoga/YGEnums.h',
  'extern/yoga/yoga/YGValue.h',
  'extern/yoga/yoga/CompactValue.h',
  'extern/yoga/yoga/Utils.h',
  'extern/yoga/yoga/YGFloatOptional.h',
  'extern/yoga/yoga/YGNode.h',
  'extern/yoga/yoga/BitUtils.h',
  'extern/yoga/yoga/Yoga-internal.h',
  'extern/yoga/yoga/Yoga.h',
  'extern/yoga/yoga/YGMacros.h',
  'extern/yoga/yoga/YGConfig.h',
  'extern/yoga/yoga/YGStyle.h',
  'include/term/_screen_window.hh',
  'include/term/basic.hh',
  'include/term/display.hh',
  'include/term/geometry.hh',
  'include/term/keyboard.hh',
  'include/term/screen.hh',
  'include/term/string.hh',
  'include/term/term.hh',
  'include/term/text.hh',
  'include/ui/element.hh',
  'include/ui/ui.hh',
  'include/ui/widget.hh',
  'include/ui/window.hh',
]

incdir = include_directories([
  'include/term',
  'include/ui',
  'extern/yoga',
  'include',
  'extern'
])

lib_dep = [
  dependency('fmt'),
  dependency('utfcpp'),
  dependency('range-v3'),
  dependency('rxcpp'),
  dependency('tl-optional'),
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
  dependency('fmt')
]

test_exe = executable(
  'test_term',
  'test/test_display.cc',
  'test/test_geometry.cc',
  'test/test_keyboard.cc',
  'test/test_screen.cc',
  'test/test_term.cc',
  'test/test_text.cc',
  link_with: shlib,
  dependencies: [test_dep, lib_dep],
  include_directories: incdir
)
test('term', test_exe)

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
