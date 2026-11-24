#include <term/keyboard.hh>

#include <cstdio>
#include <doctest/doctest.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using jwezel::Key;

// NOLINTBEGIN(misc-use-anonymous-namespace)

TEST_CASE("Keyboard") {
  auto *tmpFile{tmpfile()};
  (void)fputs("\x1b\x0d\x1b\x1b\x1bO_\x1b[D\x1b[\xff\xff\xff""Dx", tmpFile); // \xff bytes simulate a delay
  (void)fseek(tmpFile, 0, SEEK_SET);
  jwezel::Keyboard kb(tmpFile->_fileno);
  CHECK_EQ(kb.keyPrefixes().nodes.at('\x7f')->key, Key::Backspace);
  CHECK_EQ(kb.keyPrefixes().nodes.at('\x1b')->nodes['\x0d']->key, Key::AltEnter);
  CHECK_EQ(kb.keyPrefixes().nodes.at('\x1b')->nodes['\x0d']->nodes.size(), 0);
  CHECK_EQ(kb.keyPrefixes().nodes.at('\x1b')->nodes.size(), 6);
  SUBCASE("Read key") {
    CHECK_EQ(kb.key(), jwezel::Key::AltEnter);
    CHECK_EQ(kb.key(), Key::Escape);
    CHECK_EQ(kb.key(), Key::Escape);
    CHECK_EQ(kb.key(), Key::Escape);  // |
    CHECK_EQ(kb.key(), 'O');          // |
    CHECK_EQ(kb.key(), '_');          // `--> no such function key, hence Escape and individual keys
    CHECK_EQ(kb.key(), jwezel::Key::Left);
    CHECK_EQ(kb.key(), Key::Escape);  // |
    CHECK_EQ(kb.key(), '[');          // |
    CHECK_EQ(kb.key(), 'D');          // `--> did not arrive in time (triple delay), hence not part of a function key sequence
    CHECK_EQ(kb.key(), 'x');
  }
  (void)fclose(tmpFile);
}

TEST_CASE("Keyboard: Tab, Shift-Tab, Escape") {
  auto *tmpFile{tmpfile()};
  (void)fputs("\x09\x1b[Z\x1b", tmpFile); // Tab sends HT, Shift-Tab a sequence, and a lone ESC is Escape
  (void)fseek(tmpFile, 0, SEEK_SET);
  jwezel::Keyboard kb(tmpFile->_fileno);
  CHECK_EQ(kb.key(), Key::Tab);
  CHECK_EQ(kb.key(), Key::ShiftTab);
  CHECK_EQ(kb.key(), Key::Escape);
  (void)fclose(tmpFile);
}

namespace {

// All keys that there are sequences for
void collectKeys(const jwezel::Keyboard::PrefixNode &node, std::vector<jwezel::Unicode> &keys) {
  if (node.key != Key::None) {
    keys.push_back(node.key);
  }
  for (const auto &[byte, next]: node.nodes) {
    collectKeys(*next, keys);
  }
}

// The keys that come out of an input that ends after it
auto keysOf(const std::string &input, size_t count) -> std::vector<jwezel::Unicode> {
  auto *tmpFile{tmpfile()};
  (void)fputs(input.c_str(), tmpFile);
  (void)fseek(tmpFile, 0, SEEK_SET);
  std::vector<jwezel::Unicode> result;
  {
    jwezel::Keyboard kb(tmpFile->_fileno);
    for (size_t i = 0; i < count; ++i) {
      result.push_back(kb.key());
    }
  }
  (void)fclose(tmpFile);
  return result;
}

using Keys = std::vector<jwezel::Unicode>;

// A test that waits for input that never comes has to fail, not hang: when it has waited for too long, the input comes
struct Watchdog {
  explicit Watchdog(int fd):
  fd_{fd},
  thread_{
    [this] {
      for (auto i = 0; i < 300 and not done_; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
      }
      if (not done_) {
        (void) not write(fd_, "\x01", 1); // a key that nothing expects, so that what was waited for is not what comes
      }
    }
  }
  {}

  Watchdog(const Watchdog &) = delete;

  Watchdog(Watchdog &&) = delete;

  auto operator=(const Watchdog &) -> Watchdog & = delete;

  auto operator=(Watchdog &&) -> Watchdog & = delete;

  void stop() {
    done_ = true;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  ~Watchdog() {stop();}

  private:
  int fd_;
  std::atomic<bool> done_{false};
  std::thread thread_;
};

} // namespace

TEST_CASE("Keyboard: Escape is a key, and the start of others") {
  SUBCASE("It is a logical key like the others, with its bytes in the translations") {
    CHECK_GT(static_cast<jwezel::Unicode>(Key::Escape), static_cast<jwezel::Unicode>(Key::None));
    auto *tmpFile{tmpfile()};
    jwezel::Keyboard kb(tmpFile->_fileno);
    CHECK_EQ(kb.keyPrefixes().nodes.at('\x1b')->key, Key::Escape);
    CHECK_GT(kb.keyPrefixes().nodes.at('\x1b')->nodes.size(), 0); // and sequences go on from there
    (void)fclose(tmpFile);
  }
  SUBCASE("Alone") {
    CHECK_EQ(keysOf("\x1b", 1), Keys{Key::Escape});
    CHECK_EQ(keysOf("a\x1b" "b", 3), Keys{'a', Key::Escape, 'b'}); // (a hex escape takes all the hex digits)
  }
  SUBCASE("Followed by a character that is not part of a key") {
    CHECK_EQ(keysOf("\x1bx", 2), Keys{Key::Escape, 'x'});
    CHECK_EQ(keysOf("\x1b[", 2), Keys{Key::Escape, '['});
    CHECK_EQ(keysOf("\x1b[99X", 5), Keys{Key::Escape, '[', '9', '9', 'X'});
    CHECK_EQ(keysOf("\x1bO", 2), Keys{Key::Escape, 'O'});
  }
  SUBCASE("More than one") {
    CHECK_EQ(keysOf("\x1b\x1b", 2), Keys{Key::Escape, Key::Escape});
    CHECK_EQ(keysOf("\x1b\x1bx", 3), Keys{Key::Escape, Key::Escape, 'x'});
    CHECK_EQ(keysOf("\x1b\x1b\x1b", 3), Keys{Key::Escape, Key::Escape, Key::Escape});
  }
  SUBCASE("It does not get in the way of sequences") {
    CHECK_EQ(keysOf("\x1b[A", 1), Keys{Key::Up});
    CHECK_EQ(keysOf("\x1b[A\x1b[B", 2), Keys{Key::Up, Key::Down});
    CHECK_EQ(keysOf("\x1bOP", 1), Keys{Key::F1});
    CHECK_EQ(keysOf("\x1b[15~", 1), Keys{Key::F5});
    CHECK_EQ(keysOf("\x1b[1;5A", 1), Keys{Key::CtrlUp});
    CHECK_EQ(keysOf("\x1b\x1b[A", 1), Keys{Key::AltUp});
    CHECK_EQ(keysOf("\x1b\x0d", 1), Keys{Key::AltEnter});
    CHECK_EQ(keysOf("\x1b\x09", 1), Keys{Key::AltTab});
    CHECK_EQ(keysOf("\x1b[A\x1b[B\x1b", 3), Keys{Key::Up, Key::Down, Key::Escape});
    CHECK_EQ(keysOf("\x1b\x1b[A\x1b", 2), Keys{Key::AltUp, Key::Escape});
  }
  SUBCASE("A mouse report is a key and characters") {
    CHECK_EQ(keysOf("\x1b[<0;3;2M", 6), Keys{Key::Mouse, '0', ';', '3', ';', '2'});
  }
  SUBCASE("A key sequence that is split by a delay is not one") {
    // 0xff bytes simulate a delay of 1 ms each
    CHECK_EQ(keysOf("\x1b\xff\xff\xff[A", 3), Keys{Key::Escape, '[', 'A'});
  }
  SUBCASE("There is nothing after the end of the input") {
    auto *tmpFile{tmpfile()};
    (void)fputs("\x1b", tmpFile);
    (void)fseek(tmpFile, 0, SEEK_SET);
    jwezel::Keyboard kb(tmpFile->_fileno);
    CHECK_EQ(kb.key(), Key::Escape);
    CHECK_THROWS_AS(kb.key(), std::runtime_error);
    (void)fclose(tmpFile);
  }
}

TEST_CASE("Keyboard: input that arrives on a stream") {
  // A terminal is a stream: when there is nothing to read, reading says so, which is not the end of the input
  int fds[2];
  REQUIRE_EQ(pipe(fds), 0);
  jwezel::Keyboard kb(fds[0]);
  Watchdog watchdog{fds[1]};
  auto put{[&](const std::string &input) {REQUIRE_EQ(write(fds[1], input.data(), input.size()), static_cast<ssize_t>(input.size()));}};
  SUBCASE("A single Escape is one key, not two") {
    put("\x1b");
    CHECK_EQ(kb.key(), Key::Escape);
    put("a"); // used to be preceded by another Escape
    CHECK_EQ(kb.key(), 'a');
  }
  SUBCASE("Sequences") {
    put("\x1b[A");
    CHECK_EQ(kb.key(), Key::Up);
    put("\x1b[B\x1bOP");
    CHECK_EQ(kb.key(), Key::Down);
    CHECK_EQ(kb.key(), Key::F1);
    put("\x1b");
    CHECK_EQ(kb.key(), Key::Escape);
    put("\x1b[");
    CHECK_EQ(kb.key(), Key::Escape);
    CHECK_EQ(kb.key(), '[');
    put("q");
    CHECK_EQ(kb.key(), 'q');
  }
  watchdog.stop();
  close(fds[0]);
  close(fds[1]);
}

TEST_CASE("Key names") {
  auto *tmpFile{tmpfile()};
  jwezel::Keyboard kb(tmpFile->_fileno);
  SUBCASE("Both ways") {
    CHECK_EQ(jwezel::keyFromName("Escape"), Key::Escape);
    CHECK_EQ(jwezel::keyFromName("Enter"), Key::Enter);
    CHECK_EQ(jwezel::keyFromName("CtrlShiftF5"), Key::CtrlShiftF5);
    CHECK_EQ(jwezel::keyFromName("AltLeft"), Key::AltLeft);
    CHECK_EQ(jwezel::nameOfKey(Key::Escape), "Escape");
    CHECK_EQ(jwezel::nameOfKey(Key::Tab), "Tab");
    CHECK_EQ(jwezel::nameOfKey(Key::AltF12), "AltF12");
  }
  SUBCASE("What is not a key") {
    CHECK_FALSE(jwezel::keyFromName("").has_value());
    CHECK_FALSE(jwezel::keyFromName("escape").has_value()); // names are exact
    CHECK_FALSE(jwezel::keyFromName("Nothing").has_value());
    CHECK_FALSE(jwezel::nameOfKey('a').has_value()); // characters are not logical keys
    CHECK_FALSE(jwezel::nameOfKey(Key::None).has_value());
  }
  SUBCASE("Every key that a terminal can send has a name, which is the name it was given") {
    Keys keys;
    collectKeys(kb.keyPrefixes(), keys);
    CHECK_GT(keys.size(), 100);
    for (const auto key: keys) {
      const auto name{jwezel::nameOfKey(key)};
      INFO("Key: ", static_cast<unsigned>(key));
      REQUIRE(name.has_value());
      CHECK_EQ(jwezel::keyFromName(name.value()), key);
    }
  }
  SUBCASE("Keys that no terminal sends have names too") {
    for (const auto *name: {"System", "AltSystem", "Mouse"}) {
      INFO("Name: ", name);
      REQUIRE(jwezel::keyFromName(name).has_value());
      CHECK_EQ(jwezel::nameOfKey(jwezel::keyFromName(name).value()), name);
    }
  }
  (void)fclose(tmpFile);
}

TEST_CASE("Real user-operated keyboard" * doctest::skip(true)) {
  jwezel::Keyboard kb;
  std::cout << "Press F1\n";
  kb.raw();
  auto key{kb.key()};
  kb.reset();
  CHECK_EQ(key, jwezel::Key::F1);
}

// NOLINTEND(misc-use-anonymous-namespace)
