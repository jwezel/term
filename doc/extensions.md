@page extensions Extension widgets

The `src/ext_ui/` module contains reusable composite widgets built from the core UI library. These controls are optional
building blocks: they combine existing widgets while keeping application data value-owned and outside the UI.

## `ListEditor`

[`jwezel::ui::ListEditor`](\ref jwezel::ui::ListEditor) manages a list of `Text` values and provides Add, Edit, and
Remove controls. Its rows are the complete set of available values; multi-selection is independent of that set. This
allows an application to interpret the selected subset separately, such as marking preferred traits.

`choices()` returns all rows, while `selected()` returns selected values in list order. `add()`, `editSelected()`, and
`removeSelected()` update the rows. The editor owns its list, line edit, and buttons as value members, and the parent
container must outlive the editor.

`onChange()` reports user-driven selection or choice changes through [`ListEditorEvent`](\ref
jwezel::ui::ListEditorEvent). Direct calls to the three choice-editing methods are silent. The address dialog in
`examples/demo/demo.cc` demonstrates using selected values as a contact's traits.
