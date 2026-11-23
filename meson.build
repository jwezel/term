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
  'src/term/element_device.cc',
  'src/term/geometry.cc',
  'src/term/keyboard.cc',
  'src/term/surface.cc',
  'src/term/term.cc',
  'src/term/text.cc',
  'src/term/window.cc',
  'src/ui/action.cc',
  'src/ui/box.cc',
  'src/ui/button.cc',
  'src/ui/color_style.cc',
  'src/ui/container.cc',
  'src/ui/decoration.cc',
  'src/ui/drop_down.cc',
  'src/ui/element.cc',
  'src/ui/frame.cc',
  'src/ui/label.cc',
  'src/ext_ui/list_editor.cc',
  'src/ui/menu.cc',
  'src/ui/popup_choice.cc',
  'src/ui/popup_window.cc',
  'src/ui/root.cc',
  'src/ui/text_buffer.cc',
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
  'src/term/element_device.hh',
  'src/term/geometry.hh',
  'src/term/keyboard.hh',
  'src/term/mouse_capture.hh',
  'src/term/surface.hh',
  'src/term/term.hh',
  'src/term/text.hh',
  'src/term/window.hh',
  'src/ui/action.hh',
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
  'src/ui/list_box.hh',
  'src/ext_ui/list_editor.hh',
  'src/ui/list_event.hh',
  'src/ui/list_popup.hh',
  'src/ui/list_source.hh',
  'src/ui/list_view.hh',
  'src/ui/menu.hh',
  'src/ui/popup_choice.hh',
  'src/ui/popup_window.hh',
  'src/ui/root.hh',
  'src/ui/selection.hh',
  'src/ui/text_area.hh',
  'src/ui/text_buffer.hh',
  'src/ui/text_input.hh',
  'src/ui/ui.hh',
  'src/ui/ui_event.hh',
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
  'test/test_event.cc',
  'test/test_geometry.cc',
  'test/test_keyboard.cc',
  'test/test_surface.cc',
  'test/test_term.cc',
  'test/test_text.cc',
  'test/test_window.cc',
  'test/ui/test_action.cc',
  'test/ext_ui/test_list_editor.cc',
  'test/ui/test_list_source.cc',
  'test/ui/test_style.cc',
  'test/ui/test_text_buffer.cc',
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

demo_exe = executable(
  'demo',
  'examples/demo/demo.cc',
  link_with: shlib,
  dependencies: [lib_dep],
  include_directories: lib_incdir
)

quick_start_exe = executable(
  'quick_start',
  'examples/quick_start/quick_start.cc',
  link_with: shlib,
  dependencies: [lib_dep],
  include_directories: lib_incdir
)

doxygen = find_program('doxygen', required: false)
if doxygen.found()
  doc_config = configuration_data()
  header_inputs = []
  foreach header : headers
    header_inputs += '"' + join_paths(meson.project_source_root(), header) + '"'
  endforeach
  doc_pages = [
    'doc/index.md',
    'doc/introduction.md',
    'doc/tutorial.md',
    'doc/concepts.md',
    'doc/extensions.md',
    'doc/developer.md',
    'doc/architecture/index.md'
  ]
  page_inputs = []
  foreach page : doc_pages
    page_inputs += '"' + join_paths(meson.project_source_root(), page) + '"'
  endforeach
  doc_config.set('HEADER_INPUTS', ' '.join(header_inputs))
  doc_config.set('DOC_INPUTS', ' '.join(page_inputs))
  doc_config.set_quoted('EXAMPLE_DIR', join_paths(meson.project_source_root(), 'examples/quick_start'))
  doc_config.set_quoted('IMAGE_DIR', join_paths(meson.project_source_root(), 'doc/images'))
  doc_config.set_quoted('MAIN_PAGE', join_paths(meson.project_source_root(), 'doc/index.md'))
  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: doc_config
  )
  doc_inputs = files(
    'Doxyfile.in',
    'doc/build_docs.py',
    'doc/index.md',
    'doc/introduction.md',
    'doc/tutorial.md',
    'doc/concepts.md',
    'doc/extensions.md',
    'doc/developer.md',
    'doc/architecture/index.md',
    'examples/quick_start/quick_start.cc',
    'doc/images/quick_start.png'
  )
  custom_target(
    'docs',
    input: doxyfile,
    output: 'docs',
    command: [find_program('python3'), files('doc/build_docs.py'), doxygen, '@INPUT@', '@OUTPUT@'],
    depend_files: headers + doc_inputs,
    build_by_default: false
  )
else
  message('Doxygen was not found; the docs target is unavailable')
endif

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
