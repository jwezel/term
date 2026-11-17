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
        const auto changed{armed_ != inside};
        armed_ = inside;
        if (changed) {
          render(); // looks pressed
        }
        return inside;
      }
      const auto fire{armed_ and inside};
      const auto wasArmed{armed_};
      armed_ = false;
      if (wasArmed) {
        render();
      }
      if (fire) {
        press(event);
      }
      return wasArmed;
    }
  );
}

void Button::render() {
  const auto size{area().size()};
  if (size.x() <= 0 or size.y() <= 0 or text_.height() == 0 or text_.width() == 0) {
    return;
  }
  const auto state{static_cast<size_t>(widgetState(armed_))};
  const auto &brackets{uiStyle().button[state]};
  const Text left{brackets.left}, right{brackets.right};
  auto visible{canvas(buttonRoles[state])};
  for (Dim row = 0; row < text_.height(); ++row) {
    overlay(visible, left, Vector{0, row});
    overlay(visible, text_[Rectangle{0, row, text_.width(), toDim(row + 1)}], Vector{left.width(), row});
    overlay(visible, right, Vector{toDim(left.width() + text_.width()), row});
  }
  write(Vector{0, 0}, visible);
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
