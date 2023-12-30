#include "keyboard.hh"

#include <cstdio>
#include <doctest/doctest.h>
#include <iostream>

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
    CHECK_EQ(kb.key(), '\x1b');
    CHECK_EQ(kb.key(), '\x1b');
    CHECK_EQ(kb.key(), '\x1b');  // |
    CHECK_EQ(kb.key(), 'O');     // |
    CHECK_EQ(kb.key(), '_');     // `--> no such function key, hence indiviual keys
    CHECK_EQ(kb.key(), jwezel::Key::Left);
    CHECK_EQ(kb.key(), '\x1b');  // |
    CHECK_EQ(kb.key(), '[');     // |
    CHECK_EQ(kb.key(), 'D');     // `--> did not arrive in time (triple delay), hence not part of a function key sequence
    CHECK_EQ(kb.key(), 'x');
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
