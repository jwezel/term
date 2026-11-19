#include "button.hh"

#include <term/event.hh>
#include <term/keyboard.hh>

namespace jwezel::ui {

Button::Button(Container &parent, const Rectangle &area, const Text &text, const Char &background):
Label{parent, area, text, background}
{
  render();
  registerHandlers();
  focusPolicy(FocusPolicy::strong); // unlike a label, a button gets the focus
  layoutChanged(); // the brackets make it wider than its text
}

Button::Button(Container &parent, const Text &text, const Char &background):
Label{parent, text, background}
{
  render();
  registerHandlers();
  focusPolicy(FocusPolicy::strong);
  layoutChanged();
}

void Button::registerHandlers() {
  onKey(
    [this](const ui::Element &, const Event &event) {
      if (const auto key{event.as<KeyEvent>().key()}; key == ' ' or key == Key::Enter) {
        press(event);
        return true;
      }
      return false;
    }
  );
  onMouseButton(
    [this](const ui::Element &, const Event &event) {
      const auto &mouse{event.as<MouseButtonEvent>()};
      if (mouse.button() != MouseButton::Button1 or mouse.modifiers().mod5) { // not the left button, or the wheel
        return false;
      }
      const Rectangle pointArea{mouse.position(), mouse.position() + Vector{1, 1}};
      const auto inside{bool{bounds() & pointArea}};
      if (mouse.action() == MouseAction::Down) {
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
  emit(PressEvent{UiEvent::causeOf(event)});
  triggerActions();
}

auto Button::onPress(const Handler &handler) -> int {
  return on<PressEvent>(handler);
}

auto Button::preferredSize() -> Vector {
  if (text_.height() <= 0 or text_.width() <= 0) {
    return Vector{0, 0}; // it shows nothing
  }
  const auto &brackets{uiStyle().button[static_cast<size_t>(WidgetState::normal)]};
  const Text left{brackets.left}, right{brackets.right};
  return Vector{
    toDim(left.width() + text_.width() + right.width()),
    std::max(Dim{1}, text_.height())
  };
}

} // namespace jwezel::ui
