#include <term/event.hh>
#include <term/geometry.hh>
#include <term/keyboard.hh>

#include <doctest/doctest.h>

#include <stdexcept>
#include <string>

using
  jwezel::Event,
  jwezel::InputEvent,
  jwezel::Key,
  jwezel::KeyEvent,
  jwezel::MouseAction,
  jwezel::MouseButton,
  jwezel::MouseButtonEvent,
  jwezel::MouseEvent,
  jwezel::MouseMoveEvent,
  jwezel::Vector;

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

namespace {

auto mouseButton() -> Event {
  return Event{new MouseButtonEvent{MouseButton::Button1, {}, Vector{3, 2}, MouseAction::Down}};
}

}

TEST_CASE("Event: typed access") {
  const Event key{new KeyEvent{'x'}};
  SUBCASE("get") {
    REQUIRE_NE(key.get<KeyEvent>(), nullptr);
    CHECK_EQ(key.get<KeyEvent>()->key(), 'x');
    CHECK_EQ(key.get<MouseButtonEvent>(), nullptr); // another kind
    CHECK_EQ(key.get<MouseMoveEvent>(), nullptr);
  }
  SUBCASE("is") {
    CHECK(key.is<KeyEvent>());
    CHECK_FALSE(key.is<MouseEvent>());
  }
  SUBCASE("as") {
    CHECK_EQ(key.as<KeyEvent>().key(), 'x');
    CHECK_EQ(&key.as<KeyEvent>(), key.get<KeyEvent>()); // not a copy
    CHECK_THROWS_AS((void)key.as<MouseButtonEvent>(), std::logic_error);
  }
  SUBCASE("The error says what it is and what it was expected to be") {
    try {
      (void)key.as<MouseButtonEvent>();
      FAIL("It should have thrown");
    } catch (const std::logic_error &error) {
      const std::string message{error.what()};
      CHECK_NE(message.find("KeyEvent"), std::string::npos);
      CHECK_NE(message.find("MouseButtonEvent"), std::string::npos);
    }
  }
  SUBCASE("A kind of event is also the kinds it is derived from") {
    const auto mouse{mouseButton()};
    CHECK(mouse.is<MouseButtonEvent>());
    CHECK(mouse.is<MouseEvent>());
    CHECK(mouse.is<InputEvent>());
    CHECK_FALSE(mouse.is<KeyEvent>());
    CHECK_FALSE(mouse.is<MouseMoveEvent>()); // a sibling is not
    CHECK_EQ(mouse.as<MouseEvent>().position(), Vector{3, 2});
    CHECK_EQ(mouse.as<MouseButtonEvent>().button(), MouseButton::Button1);
    CHECK_NOTHROW((void)key.as<InputEvent>()); // a key is an input event too
  }
  SUBCASE("An event that is empty is nothing") {
    const Event empty;
    CHECK_FALSE(empty.is<KeyEvent>());
    CHECK_EQ(empty.get<KeyEvent>(), nullptr);
    CHECK_THROWS_AS((void)empty.as<KeyEvent>(), std::logic_error);
    CHECK_EQ(empty.type(), 0);
    CHECK_EQ(std::string{empty.typeName()}, "empty");
  }
  SUBCASE("The kinds have names") {
    CHECK_EQ(std::string{KeyEvent::typeName_}, "KeyEvent");
    CHECK_NE(KeyEvent::type_, MouseButtonEvent::type_);
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(readability-magic-numbers)
