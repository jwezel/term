#include "popup_window.hh"

#include <term/term_interface.hh>

namespace jwezel::ui {

PopupWindow::PopupWindow(
  TerminalInterface &terminal,
  const Rectangle &area,
  const Char &background,
  jwezel::Window &owner,
  const ColorStyle *colors,
  const UiStyle *style
):
jwezel::Window{&terminal, area, background, nullptr, &owner},
Root{static_cast<jwezel::Window &>(*this), colors, style}
{}

} // namespace jwezel::ui
