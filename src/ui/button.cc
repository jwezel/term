#include "button.hh"

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

Button::Button(Container &parent, const Rectangle &area, const Text &text, const Char &background):
Label{parent, area, text, background}
{
  render();
  registerHandlers();
}

Button::Button(Container &parent, const Text &text, const Char &background):
Label{parent, text, background}
{
  render();
  registerHandlers();
}

void Button::registerHandlers() {
  onKey(
    [this](const ui::Element &, const Event &event) {
      if (dynamic_cast<KeyEvent *>(event())->key() == ' ') {
        press(event);
        return true;
      }
      return false;
    }
  );
  onMouseButton(
    [this](const ui::Element &, const Event &event) {
      auto *mouse{dynamic_cast<MouseButtonEvent *>(event())};
      if (mouse->button() != MouseButton::Button1 or mouse->modifiers().mod5) { // not the left button, or the wheel
        return false;
      }
      const Rectangle pointArea{mouse->position(), mouse->position() + Vector{1, 1}};
      const auto inside{bool{bounds() & pointArea}};
      if (mouse->action() == MouseAction::Down) {
        armed_ = inside;
        return inside;
      }
      const auto fire{armed_ and inside};
      const auto wasArmed{armed_};
      armed_ = false;
      if (fire) {
        press(event);
      }
      return wasArmed;
    }
  );
}

void Button::render() {
  if (text_.height() == 0 or text_.width() == 0) {
    return;
  }
  for (Dim row = 0; row < text_.height(); ++row) {
    auto line{Text{Space, Vector{toDim(text_.width() + 4), 1}}};
    line.patch(Text{"[ "}, Vector{0, 0});
    line.patch(text_[Rectangle{0, row, text_.width(), toDim(row + 1)}], Vector{2, 0});
    line.patch(Text{" ]"}, Vector{toDim(text_.width() + 2), 0});
    write(Vector{0, row}, line);
  }
}

void Button::press(const Event &event) {
  for (const auto &handler : pressHandlers_) {
    handler(*this, event);
  }
}

auto Button::onPress(const Handler &handler) -> int {
  pressHandlers_.push_back(handler);
  return static_cast<int>(pressHandlers_.size() - 1);
}

} // namespace jwezel::ui
