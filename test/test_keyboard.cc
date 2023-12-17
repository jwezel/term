#include "keyboard.hh"

#include <cstdio>
#include <doctest/doctest.h>
#include <iostream>


using namespace jwezel;

TEST_CASE("Keyboard") {
  auto tmpFile{tmpfile()};
  fputs("\x1b\x0d\x1b\x1b\x1bO_\x1b[D\x1b[\xff\xff\xff""Dx", tmpFile); // \xff bytes simulate a delay
  fseek(tmpFile, 0, SEEK_SET);
  Keyboard k(tmpFile->_fileno);
  CHECK_EQ(k.keyPrefixes.nodes['\x7f']->key, Key::Backspace);
  CHECK_EQ(k.keyPrefixes.nodes['\x1b']->nodes['\x0d']->key, Key::AltEnter);
  CHECK_EQ(k.keyPrefixes.nodes['\x1b']->nodes['\x0d']->nodes.size(), 0);
  CHECK_EQ(k.keyPrefixes.nodes['\x1b']->nodes.size(), 6);
  SUBCASE("Read key") {
    CHECK_EQ(k.key(), Key::AltEnter);
    CHECK_EQ(k.key(), '\x1b');
    CHECK_EQ(k.key(), '\x1b');
    CHECK_EQ(k.key(), '\x1b');  // |
    CHECK_EQ(k.key(), 'O');     // |
    CHECK_EQ(k.key(), '_');     // `--> no such function key, hence indiviual keys
    CHECK_EQ(k.key(), Key::Left);
    CHECK_EQ(k.key(), '\x1b');  // |
    CHECK_EQ(k.key(), '[');     // |
    CHECK_EQ(k.key(), 'D');     // `--> did not arrive in time (triple delay), hence not part of a function key sequence
    CHECK_EQ(k.key(), 'x');
  }
  fclose(tmpFile);
}
TEST_CASE("Real user-operated keyboard" * doctest::skip(true)) {
  Keyboard k;
  std::cout << "Press F1\n";
  k.raw();
  auto key{k.key()};
  k.reset();
  CHECK_EQ(key, Key::F1);
}
