#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <system_error>
#include <termios.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <utility>

#include "geometry.hh"
#include "keyboard.hh"
#include "text.hh"

namespace jwezel {
using std::format;
using std::chrono::steady_clock;
using std::this_thread::sleep_for;
using
  std::pair,
  std::smatch,
  std::basic_regex,
  std::make_pair,
  std::runtime_error,
  std::cerr,
  std::strerror,
  std::u32string,
  std::this_thread::sleep_for;
using namespace std::chrono_literals;

namespace
{
const vector<pair<string, Unicode>> KeyFromName { // NOLINT(cert-err58-cpp)
  {"Left", Key::Left},
  {"Right", Key::Right},
  {"Up", Key::Up},
  {"Down", Key::Down},
  {"Insert", Key::Insert},
  {"Delete", Key::Delete},
  {"Home", Key::Home},
  {"End", Key::End},
  {"PageUp", Key::PageUp},
  {"PageDown", Key::PageDown},
  {"Backspace", Key::Backspace},
  {"Enter", Key::Enter},
  {"Tab", Key::Tab},
  {"F1", Key::F1},
  {"F2", Key::F2},
  {"F3", Key::F3},
  {"F4", Key::F4},
  {"F5", Key::F5},
  {"F6", Key::F6},
  {"F7", Key::F7},
  {"F8", Key::F8},
  {"F9", Key::F9},
  {"F10", Key::F10},
  {"F11", Key::F11},
  {"F12", Key::F12},
  {"ShiftLeft", Key::ShiftLeft},
  {"ShiftRight", Key::ShiftRight},
  {"ShiftUp", Key::ShiftUp},
  {"ShiftDown", Key::ShiftDown},
  {"ShiftInsert", Key::ShiftInsert},
  {"ShiftDelete", Key::ShiftDelete},
  {"ShiftHome", Key::ShiftHome},
  {"ShiftEnd", Key::ShiftEnd},
  {"ShiftPageUp", Key::ShiftPageUp},
  {"ShiftPageDown", Key::ShiftPageDown},
  {"ShiftBackspace", Key::ShiftBackspace},
  {"ShiftEnter", Key::ShiftEnter},
  {"ShiftTab", Key::ShiftTab},
  {"ShiftF1", Key::ShiftF1},
  {"ShiftF2", Key::ShiftF2},
  {"ShiftF3", Key::ShiftF3},
  {"ShiftF4", Key::ShiftF4},
  {"ShiftF5", Key::ShiftF5},
  {"ShiftF6", Key::ShiftF6},
  {"ShiftF7", Key::ShiftF7},
  {"ShiftF8", Key::ShiftF8},
  {"ShiftF9", Key::ShiftF9},
  {"ShiftF10", Key::ShiftF10},
  {"ShiftF11", Key::ShiftF11},
  {"ShiftF12", Key::ShiftF12},
  {"CtrlLeft", Key::CtrlLeft},
  {"CtrlRight", Key::CtrlRight},
  {"CtrlUp", Key::CtrlUp},
  {"CtrlDown", Key::CtrlDown},
  {"CtrlInsert", Key::CtrlInsert},
  {"CtrlDelete", Key::CtrlDelete},
  {"CtrlHome", Key::CtrlHome},
  {"CtrlEnd", Key::CtrlEnd},
  {"CtrlPageUp", Key::CtrlPageUp},
  {"CtrlPageDown", Key::CtrlPageDown},
  {"CtrlBackspace", Key::CtrlBackspace},
  {"CtrlEnter", Key::CtrlEnter},
  {"CtrlTab", Key::CtrlTab},
  {"CtrlF1", Key::CtrlF1},
  {"CtrlF2", Key::CtrlF2},
  {"CtrlF3", Key::CtrlF3},
  {"CtrlF4", Key::CtrlF4},
  {"CtrlF5", Key::CtrlF5},
  {"CtrlF6", Key::CtrlF6},
  {"CtrlF7", Key::CtrlF7},
  {"CtrlF8", Key::CtrlF8},
  {"CtrlF9", Key::CtrlF9},
  {"CtrlF10", Key::CtrlF10},
  {"CtrlF11", Key::CtrlF11},
  {"CtrlF12", Key::CtrlF12},
  {"CtrlShiftLeft", Key::CtrlShiftLeft},
  {"CtrlShiftRight", Key::CtrlShiftRight},
  {"CtrlShiftUp", Key::CtrlShiftUp},
  {"CtrlShiftDown", Key::CtrlShiftDown},
  {"CtrlShiftInsert", Key::CtrlShiftInsert},
  {"CtrlShiftDelete", Key::CtrlShiftDelete},
  {"CtrlShiftHome", Key::CtrlShiftHome},
  {"CtrlShiftEnd", Key::CtrlShiftEnd},
  {"CtrlShiftPageUp", Key::CtrlShiftPageUp},
  {"CtrlShiftPageDown", Key::CtrlShiftPageDown},
  {"CtrlShiftBackspace", Key::CtrlShiftBackspace},
  {"CtrlShiftEnter", Key::CtrlShiftEnter},
  {"CtrlShiftTab", Key::CtrlShiftTab},
  {"CtrlShiftF1", Key::CtrlShiftF1},
  {"CtrlShiftF2", Key::CtrlShiftF2},
  {"CtrlShiftF3", Key::CtrlShiftF3},
  {"CtrlShiftF4", Key::CtrlShiftF4},
  {"CtrlShiftF5", Key::CtrlShiftF5},
  {"CtrlShiftF6", Key::CtrlShiftF6},
  {"CtrlShiftF7", Key::CtrlShiftF7},
  {"CtrlShiftF8", Key::CtrlShiftF8},
  {"CtrlShiftF9", Key::CtrlShiftF9},
  {"CtrlShiftF10", Key::CtrlShiftF10},
  {"CtrlShiftF11", Key::CtrlShiftF11},
  {"CtrlShiftF12", Key::CtrlShiftF12},
  {"AltLeft", Key::AltLeft},
  {"AltRight", Key::AltRight},
  {"AltUp", Key::AltUp},
  {"AltDown", Key::AltDown},
  {"AltInsert", Key::AltInsert},
  {"AltDelete", Key::AltDelete},
  {"AltHome", Key::AltHome},
  {"AltEnd", Key::AltEnd},
  {"AltPageUp", Key::AltPageUp},
  {"AltPageDown", Key::AltPageDown},
  {"AltBackspace", Key::AltBackspace},
  {"AltEnter", Key::AltEnter},
  {"AltTab", Key::AltTab},
  {"AltF1", Key::AltF1},
  {"AltF2", Key::AltF2},
  {"AltF3", Key::AltF3},
  {"AltF4", Key::AltF4},
  {"AltF5", Key::AltF5},
  {"AltF6", Key::AltF6},
  {"AltF7", Key::AltF7},
  {"AltF8", Key::AltF8},
  {"AltF9", Key::AltF9},
  {"AltF10", Key::AltF10},
  {"AltF11", Key::AltF11},
  {"AltF12", Key::AltF12}
};

const std::vector<pair<std::string, Unicode>> keyTranslations { // NOLINT(cert-err58-cpp)
  {"\x1b[D", Key::Left},
  {"\x1b[C", Key::Right},
  {"\x1b[A", Key::Up},
  {"\x1b[B", Key::Down},
  {"\x1b[2~", Key::Insert},
  {"\x1b[3~", Key::Delete},
  {"\x1b[H", Key::Home},
  {"\x1b[F", Key::End},
  {"\x1b[5~", Key::PageUp},
  {"\x1b[6~", Key::PageDown},
  {"\x7f", Key::Backspace},
  {"\x0d", Key::Enter},
  {"\x07", Key::Tab},
  // {"", Key::System},
  {"\x1bOP", Key::F1},
  {"\x1bOQ", Key::F2},
  {"\x1bOR", Key::F3},
  {"\x1bOS", Key::F4},
  {"\x1b[15~", Key::F5},
  {"\x1b[17~", Key::F6},
  {"\x1b[18~", Key::F7},
  {"\x1b[19~", Key::F8},
  {"\x1b[20~", Key::F9},
  {"\x1b[21~", Key::F10},
  {"\x1b[23~", Key::F11},
  {"\x1b[24~", Key::F12},
  {"\x1b[1;2D", Key::ShiftLeft},
  {"\x1b[1;2C", Key::ShiftRight},
  {"\x1b[1;2A", Key::ShiftUp},
  {"\x1b[1;2B", Key::ShiftDown},
  {"\x1b[2;3~", Key::ShiftInsert},
  {"\x1b[3;2~", Key::ShiftDelete},
  {"\x1b[1;2H", Key::ShiftHome},
  {"\x1b[1;2F", Key::ShiftEnd},
  // {"", Key::ShiftPageUp},
  // {"", Key::ShiftPageDown},
  // {"", Key::ShiftBackspace},
  {"\x1bOM", Key::ShiftEnter},
  {"\x1b[Z", Key::ShiftTab},
  {"\x1bO2P", Key::ShiftF1},
  {"\x1bO2Q", Key::ShiftF2},
  {"\x1bO2R", Key::ShiftF3},
  {"\x1bO2S", Key::ShiftF4},
  {"\x1b[15;2~", Key::ShiftF5},
  {"\x1b[17;2~", Key::ShiftF6},
  {"\x1b[18;2~", Key::ShiftF7},
  {"\x1b[19;2~", Key::ShiftF8},
  {"\x1b[20;2~", Key::ShiftF9},
  {"\x1b[21;2~", Key::ShiftF10},
  {"\x1b[23;2~", Key::ShiftF11},
  {"\x1b[24;2~", Key::ShiftF12},
  {"\x1b[1;5D", Key::CtrlLeft},
  {"\x1b[1;5C", Key::CtrlRight},
  {"\x1b[1;5A", Key::CtrlUp},
  {"\x1b[1;5B", Key::CtrlDown},
  {"\x1b[2;5~", Key::CtrlInsert},
  {"\x1b[3;5~", Key::CtrlDelete},
  {"\x1b[1;5H", Key::CtrlHome},
  {"\x1b[1;5F", Key::CtrlEnd},
  // {"", Key::CtrlPageUp},
  // {"", Key::CtrlPageDown},
  // {"", Key::CtrlBackspace},
  // {"", Key::CtrlEnter},
  // {"", Key::CtrlTab},
  {"\x1bO5P", Key::CtrlF1},
  {"\x1bO5Q", Key::CtrlF2},
  {"\x1bO5R", Key::CtrlF3},
  {"\x1bO5S", Key::CtrlF4},
  {"\x1b[15;5~", Key::CtrlF5},
  {"\x1b[17;5~", Key::CtrlF6},
  {"\x1b[18;5~", Key::CtrlF7},
  {"\x1b[19;5~", Key::CtrlF8},
  {"\x1b[20;5~", Key::CtrlF9},
  {"\x1b[21;5~", Key::CtrlF10},
  {"\x1b[23;5~", Key::CtrlF11},
  {"\x1b[24;5~", Key::CtrlF12},
  {"\x1b[1;6C", Key::CtrlShiftLeft},
  {"\x1b[1;6D", Key::CtrlShiftRight},
  {"\x1b[1;6A", Key::CtrlShiftUp},
  {"\x1b[1;6B", Key::CtrlShiftDown},
  {"\x1b[200~", Key::CtrlShiftInsert},
  {"\x1b[3;6C", Key::CtrlShiftDelete},
  {"\x1b[1;6H", Key::CtrlShiftHome},
  {"\x1b[1;6F", Key::CtrlShiftEnd},
  // {"", Key::CtrlShiftPageUp},
  // {"", Key::CtrlShiftPageDown},
  // {"", Key::CtrlShiftBackspace},
  // {"", Key::CtrlShiftEnter},
  // {"", Key::CtrlShiftTab},
  {"\x1bO6P", Key::CtrlShiftF1},
  {"\x1bO6Q", Key::CtrlShiftF2},
  {"\x1bO6R", Key::CtrlShiftF3},
  {"\x1bO6S", Key::CtrlShiftF4},
  {"\x1b[15;6~", Key::CtrlShiftF5},
  {"\x1b[17;6~", Key::CtrlShiftF6},
  {"\x1b[18;6~", Key::CtrlShiftF7},
  {"\x1b[19;6~", Key::CtrlShiftF8},
  {"\x1b[20;6~", Key::CtrlShiftF9},
  {"\x1b[21;5~", Key::CtrlShiftF10},
  {"\x1b[23;6~", Key::CtrlShiftF11},
  {"\x1b[24;6~", Key::CtrlShiftF12},
  {"\x1b\x1b[D", Key::AltLeft},
  {"\x1b\x1b[C", Key::AltRight},
  {"\x1b\x1b[A", Key::AltUp},
  {"\x1b\x1b[B", Key::AltDown},
  {"\x1b\x1b[2~", Key::AltInsert},
  {"\x1b[3;3~", Key::AltDelete},
  {"\x1b\x1b[H", Key::AltHome},
  {"\x1b\x1b[F", Key::AltEnd},
  {"\x1b\x1b[5~", Key::AltPageUp},
  {"\x1b\x1b[6~", Key::AltPageDown},
  {"\x1b\x7f", Key::AltBackspace},
  {"\x1b\x0d", Key::AltEnter},
  {"\x1b\x07", Key::AltTab},
  {"\x1b\x1bOP", Key::AltF1},
  {"\x1b\x1bOQ", Key::AltF2},
  {"\x1b\x1bOR", Key::AltF3},
  {"\x1b\x1bOS", Key::AltF4},
  {"\x1b\x1b[15~", Key::AltF5},
  {"\x1b\x1b[17~", Key::AltF6},
  {"\x1b\x1b[18~", Key::AltF7},
  {"\x1b\x1b[19~", Key::AltF8},
  {"\x1b\x1b[20~", Key::AltF9},
  {"\x1b\x1b[21~", Key::AltF10},
  {"\x1b\x1b[23~", Key::AltF11},
  {"\x1b\x1b[24~", Key::AltF12},
  {"\x1b[<", Key::Mouse}
};
} // namespace

//~Static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace
{
///
/// Load key translations
///
/// @param[in]  translations  The translations
///
/// @return     key prefix tree
auto loadKeyTranslations(const std::vector<pair<std::string, Unicode>> &translations) -> Keyboard::PrefixNode {
  Keyboard::PrefixNode result;
  for (const auto &[keys, key]: translations) {
    Keyboard::PrefixNode *node{&result};
    for (auto it = keys.begin(); it != keys.end(); ++it) {
      auto elem = node->nodes.find(*it);
      if (elem == node->nodes.end()) {
        auto insResult = node->nodes.insert(make_pair(*it, new Keyboard::PrefixNode));
        if (!insResult.second) {
          throw runtime_error(format("Couldn't create keyboard translation tree node for \"{}\"", keys));
        }
        node = insResult.first->second.get();
      } else {
        node = elem->second.get();
      }
    }
    node->key = key;
  }
  return result;
}
} // namespace

//~Keyboard~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Keyboard::Keyboard(int device):
keyPrefixes(loadKeyTranslations(keyTranslations)),
fd(device)
{
  raw();
}

Keyboard::~Keyboard() {
  try {
    reset();
  } catch (...) {
    cerr << "Error closing keyboard" << "\n";
  }
}

void Keyboard::raw() {
  if (bool(isatty(fd))) {
    termios old{};
    if (!originalState) {
      if (tcgetattr(fd, &old) != 0) {
        throw std::system_error(
          errno,
          std::system_category(),
          format("Could not get terminal state for fd {}: {}", fd, strerror(errno) /*NOLINT*/)
        );
      }
      originalState.emplace(old);
    }
    termios new_{old};
    // NOLINTBEGIN(hicpp-signed-bitwise)
    new_.c_iflag = new_.c_iflag & ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    new_.c_oflag = new_.c_oflag & ~OPOST;
    new_.c_lflag = new_.c_lflag & ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    new_.c_cflag = new_.c_cflag & ~(CSIZE | PARENB);
    new_.c_cflag = new_.c_cflag | CS8;
    // NOLINTEND(hicpp-signed-bitwise)
    if (tcsetattr(fd, TCSANOW, &new_) != 0) {
      throw std::system_error(
        errno, std::system_category(), format("Could not set terminal state for fd {}: {}", fd, std::strerror(errno)/*NOLINT*/)
      );
    }
  }
}

void Keyboard::reset() {
  if (originalState) {
    if (tcsetattr(fd, TCSANOW, &originalState.value()) != 0) {
      throw std::system_error(
        errno, std::system_category(), format("Could not set terminal state for fd {}: {}", fd, std::strerror(errno)/*NOLINT*/)
      );
    }
  }
}

Unicode Keyboard::key() {
  Unicode key{0};
  if (!keyBuffer.empty()) {
    key = keyBuffer.front();
    keyBuffer.pop_front();
    return key;
  }
  char inputKey = 0;
  auto *node{&keyPrefixes};
  // PrefixNode *previousNode;
  u32string inputBuffer;
  while (true) {
    ssize_t readLength = 0;
    steady_clock::duration readTime;
    auto onSublevel{node != &keyPrefixes}; // If true expect more keys in quick succession
    if (onSublevel) {
      auto st{fcntl(fd, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK)/*NOLINT*/};
      if (st != 0) {
        cerr << format("fcntl(F_SETFL, O_NONBLOCK) returned {}\n", st);
      }
    }
    auto start{steady_clock::now()};
    auto attempt{0};
    // Handle reattempts and simulated delays
    while (true) {
      readLength = read(fd, &inputKey, 1);
      if (readLength < 0) {
        if (errno != EAGAIN) {
          throw std::system_error(errno, std::system_category(), format("Could not get key: {}", fd, std::strerror(errno)/*NOLINT*/));
        }
        // Wait twice for a key, then give up
        if (attempt++ > 1) {
          break;
        }
        sleep_for(1ms); // TODO(j): make this much smaller (based on 19200 cps) and configurable
      } else if (readLength > 0 and inputKey == '\xff') {
        // '\xff' simulates a delay
        sleep_for(1ms);
      } else {
        break;
      }
    };
    readTime = steady_clock::now() - start;
    if (onSublevel) {
      auto st{fcntl(fd, F_SETFL, fcntl(0, F_GETFL) & ~O_NONBLOCK)/*NOLINT*/};
      if (st != 0) {
        cerr << format("fcntl(F_SETFL, 0) returned {}\n", st);
      }
    }
    if (readLength == 0) {
      inputKey = '\0';
    } else {
      inputBuffer.push_back(inputKey);
    }
    auto subnodeIt{node->nodes.find(inputKey)};
    if (onSublevel and readTime > 2ms or subnodeIt == node->nodes.end()) {
      copy(inputBuffer.begin(), inputBuffer.end(), back_inserter(keyBuffer));
      break;
    }
    node = subnodeIt->second.get();
    if (node->key != Key::None) {
      keyBuffer.push_back(node->key);
      break;
    }
  }
  if (keyBuffer.empty()) {
    throw runtime_error("Key buffer is empty");
  }
  key = keyBuffer.front();
  keyBuffer.pop_front();
  return key;
}

auto Keyboard::mouseReport() -> tuple<MouseButton, MouseModifiers, u2, u2, MouseAction> {
  static const basic_regex reportPattern(R"((\d+);(\d+);(\d+)([Mm]))");
  static const auto reportBufferSize{16}, MaxAscii{255};
  Unicode key_ = 0;
  string report;
  report.reserve(reportBufferSize);
  do {
    key_ = key();
    if (key_ > MaxAscii) {
      throw range_error("Got character > 127");
    }
    report += key_/*NOLINT*/;
  } while (key_ != 'M' and key_ != 'm');
  smatch subMatch;
  if (regex_match(report, subMatch, reportPattern)) {
    return {
      static_cast<MouseButton>((stoul(subMatch[1].str()) & 3U) + 1U),
      MouseModifiers{
        static_cast<u1>(stoul(subMatch[1].str()) & 4U),
        static_cast<u1>(stoul(subMatch[1].str()) & 16U),
        static_cast<u1>(stoul(subMatch[1].str()) & 8U)
      },
      stoi(subMatch[2].str()) - 1,
      u1(stoi(subMatch[3].str()) - 1),
      subMatch[3].str() == "M"? MouseAction::Down: MouseAction::Up
    };
  }
  throw runtime_error("Could not parse terminal mouse report");
}

auto Keyboard::event() -> InputEvent * {
  static const auto MouseMove{35};
  auto key_ = key();
  if (key_ == Mouse) {
    auto [button, modifiers, x, y, action]{mouseReport()};
    if (button == MouseButton(MouseMove)) {
      return new MouseMoveEvent{x, y};
    }
    return new MouseButtonEvent{button, modifiers, x, y, action};
  }
  return new KeyEvent{key_};
}

} // namespace jwezel
