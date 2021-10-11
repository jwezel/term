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
  'src/geometry.cc',
  'src/text.cc'
]
headers = [
  'include/term/geometry.hh'
]

incdir = include_directories([
  'include/term',
  'include/util',
])

lib_dep = [
  dependency('fmt'),
  dependency('utfcpp'),
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
  'test/test_term.cc',
  'test/test_text.cc',
  link_with: shlib,
  dependencies: test_dep,
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
