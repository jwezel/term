#include <ui/action.hh>

#include <term/keyboard.hh>

#include <doctest/doctest.h>

#include <memory>
#include <optional>
#include <string>

using
  jwezel::Key,
  jwezel::ui::Action,
  jwezel::ui::ActionHost;

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Action") {
  auto count{0};
  Action action{[&count] {++count;}, {Key::Enter, 'q'}};
  SUBCASE("Trigger") {
    CHECK(action.trigger());
    CHECK_EQ(count, 1);
    CHECK(action.trigger());
    CHECK_EQ(count, 2);
  }
  SUBCASE("An action that is not enabled does nothing") {
    action.enabled(false);
    CHECK_FALSE(action.enabled());
    CHECK_FALSE(action.trigger());
    CHECK_EQ(count, 0);
    action.enabled(true);
    CHECK(action.trigger());
  }
  SUBCASE("An action without a handler does nothing, and gets one") {
    Action empty;
    CHECK_FALSE(empty.trigger());
    empty.handler([&count] {count += 10;});
    CHECK(empty.trigger());
    CHECK_EQ(count, 10);
  }
  SUBCASE("Shortcuts") {
    CHECK(action.hasShortcut(Key::Enter));
    CHECK(action.hasShortcut('q'));
    CHECK_FALSE(action.hasShortcut('x'));
    CHECK_FALSE(action.hasShortcut(Key::Escape));
    action.shortcuts({Key::Escape});
    CHECK_EQ(action.shortcuts().size(), 1);
    CHECK(action.hasShortcut(Key::Escape));
    CHECK_FALSE(action.hasShortcut(Key::Enter));
  }
}

TEST_CASE("Action host") {
  auto first{0}, second{0};
  Action one{[&first] {++first;}, {'a', 'b'}};
  Action two{[&second] {++second;}, {'b', 'c'}};
  ActionHost host;
  host.addAction(one);
  host.addAction(two);
  SUBCASE("The first action with the shortcut") {
    CHECK(host.triggerShortcut('a'));
    CHECK_EQ(first, 1);
    CHECK(host.triggerShortcut('b')); // both have it, the one that was added first does it
    CHECK_EQ(first, 2);
    CHECK_EQ(second, 0);
    CHECK(host.triggerShortcut('c'));
    CHECK_EQ(second, 1);
    CHECK_FALSE(host.triggerShortcut('z'));
  }
  SUBCASE("Actions that are not enabled are skipped") {
    one.enabled(false);
    CHECK_FALSE(host.triggerShortcut('a'));
    CHECK(host.triggerShortcut('b')); // the next one has it
    CHECK_EQ(first, 0);
    CHECK_EQ(second, 1);
  }
  SUBCASE("Adding is idempotent") {
    host.addAction(one);
    host.addAction(one);
    CHECK_EQ(host.actions().size(), 2);
    CHECK(host.triggerShortcut('a'));
    CHECK_EQ(first, 1);
  }
  SUBCASE("Remove") {
    host.removeAction(one);
    CHECK_EQ(host.actions().size(), 1);
    CHECK_FALSE(host.triggerShortcut('a'));
    host.removeAction(one); // not there: nothing
    CHECK_EQ(host.actions().size(), 1);
  }
  SUBCASE("Trigger all") {
    CHECK(host.triggerActions());
    CHECK_EQ(first, 1);
    CHECK_EQ(second, 1);
    two.enabled(false);
    CHECK(host.triggerActions());
    CHECK_EQ(first, 2);
    CHECK_EQ(second, 1);
    one.enabled(false);
    CHECK_FALSE(host.triggerActions());
  }
  SUBCASE("An action can be in several hosts") {
    ActionHost other;
    other.addAction(one);
    CHECK(other.triggerShortcut('a'));
    CHECK(host.triggerShortcut('a'));
    CHECK_EQ(first, 2);
    SUBCASE("It leaves all of them when it is destroyed") {
      auto temporary{std::make_unique<Action>([&first] {first += 100;}, std::initializer_list<jwezel::Unicode>{'t'})};
      host.addAction(*temporary);
      other.addAction(*temporary);
      CHECK(host.triggerShortcut('t'));
      CHECK(other.triggerShortcut('t'));
      CHECK_EQ(first, 202);
      temporary.reset();
      CHECK_FALSE(host.triggerShortcut('t'));
      CHECK_FALSE(other.triggerShortcut('t'));
      CHECK_EQ(host.actions().size(), 2);
      CHECK_EQ(other.actions().size(), 1);
    }
  }
  SUBCASE("A host that is destroyed lets go of the actions") {
    auto temporary{std::make_unique<ActionHost>()};
    temporary->addAction(one);
    CHECK(temporary->triggerShortcut('a'));
    temporary.reset();
    CHECK(one.trigger()); // still all right
    host.removeAction(one);
    CHECK_FALSE(host.triggerShortcut('a'));
  }
}

TEST_CASE("Action handlers that change things") {
  ActionHost host;
  auto count{0};
  SUBCASE("An action that destroys itself") {
    std::optional<Action> action;
    action.emplace([&action, &count] {++count; action.reset();}, std::initializer_list<jwezel::Unicode>{'d'});
    host.addAction(*action);
    CHECK(host.triggerShortcut('d'));
    CHECK_EQ(count, 1);
    CHECK_FALSE(action.has_value());
    CHECK_EQ(host.actions().size(), 0);
    CHECK_FALSE(host.triggerShortcut('d'));
  }
  SUBCASE("An action that removes another one that is to come") {
    Action later{[&count] {count += 10;}};
    Action first{[&host, &later, &count] {++count; host.removeAction(later);}};
    host.addAction(first);
    host.addAction(later);
    CHECK(host.triggerActions());
    CHECK_EQ(count, 1); // the other one is not there anymore
  }
  SUBCASE("An action that adds one") {
    Action added{[&count] {count += 10;}};
    Action adder{[&host, &added, &count] {++count; host.addAction(added);}};
    host.addAction(adder);
    CHECK(host.triggerActions());
    CHECK_EQ(count, 1); // it is there for the next time
    CHECK(host.triggerActions());
    CHECK_EQ(count, 12);
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
// NOLINTEND(readability-magic-numbers)
