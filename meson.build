project(
  'term',
  'cpp',
  version: '0.1',
  license: 'Apache-2.0',
  default_options: ['warning_level=3', 'cpp_std=c++17']
)

add_project_arguments('-Wno-parentheses', language: ['c', 'cpp'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_TERM']

sources = [
  'extern/yoga/event/event.cpp',
  'extern/yoga/internal/experiments.cpp',
  'extern/yoga/log.cpp',
  'extern/yoga/Utils.cpp',
  'extern/yoga/YGConfig.cpp',
  'extern/yoga/YGEnums.cpp',
  'extern/yoga/YGLayout.cpp',
  'extern/yoga/YGNode.cpp',
  'extern/yoga/YGNodePrint.cpp',
  'extern/yoga/YGStyle.cpp',
  'extern/yoga/YGValue.cpp',
  'extern/yoga/Yoga.cpp',
  'src/display.cc',
  'src/geometry.cc',
  'src/keyboard.cc',
  'src/screen.cc',
  'src/screen_window.cc',
  'src/string.cc',
  'src/term.cc',
  'src/text.cc',
#  'src/ui/element.cc',
#  'src/ui/ui.cc',
#  'src/ui/widget.cc',
#  'src/ui/window.cc',
]
headers = [
  'extern/yoga/BitUtils.h',
  'extern/yoga/CompactValue.h',
  'extern/yoga/event/event.h',
  'extern/yoga/internal/experiments-inl.h',
  'extern/yoga/internal/experiments.h',
  'extern/yoga/log.h',
  'extern/yoga/Utils.h',
  'extern/yoga/YGConfig.h',
  'extern/yoga/YGEnums.h',
  'extern/yoga/YGFloatOptional.h',
  'extern/yoga/YGLayout.h',
  'extern/yoga/YGMacros.h',
  'extern/yoga/YGNode.h',
  'extern/yoga/YGNodePrint.h',
  'extern/yoga/YGStyle.h',
  'extern/yoga/YGValue.h',
  'extern/yoga/Yoga-internal.h',
  'extern/yoga/Yoga.h',
  'include/term/_screen_window.hh',
  'include/term/basic.hh',
  'include/term/display.hh',
  'include/term/geometry.hh',
  'include/term/keyboard.hh',
  'include/term/screen.hh',
  'include/term/string.hh',
  'include/term/term.hh',
  'include/term/text.hh',
 # 'include/ui/element.hh',
 # 'include/ui/ui.hh',
 # 'include/ui/widget.hh',
 # 'include/ui/window.hh',
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
