#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>

#include <sys/stat.h>
#include <unistd.h>

///
/// A terminal for tests: input that is scripted, and output that can be inspected
///
/// The terminal replies to cursor position reports from what it was given, more input can be added at any
/// time, and everything that was written to it can be read back.
struct FakeTerminal {
  explicit FakeTerminal(const std::string &input = {}):
  input_{temporaryFile()},
  output_{temporaryFile()}
  {
    feed(input);
  }

  FakeTerminal(const FakeTerminal &) = delete;

  FakeTerminal(FakeTerminal &&) = delete;

  auto operator=(const FakeTerminal &) -> FakeTerminal & = delete;

  auto operator=(FakeTerminal &&) -> FakeTerminal & = delete;

  ~FakeTerminal() {
    close(input_);
    close(output_);
  }

  /// File descriptor to read the input from
  [[nodiscard]] auto input() const -> int {return input_;}

  /// File descriptor to write the output to
  [[nodiscard]] auto output() const -> int {return output_;}

  /// Add to the input
  void feed(const std::string &text) const {
    struct stat status{};
    (void)fstat(input_, &status);
    (void)!pwrite(input_, text.data(), text.size(), status.st_size);
  }

  /// Everything that was written since the start or since `clear`
  [[nodiscard]] auto written() const -> std::string {
    struct stat status{};
    (void)fstat(output_, &status);
    std::string result(static_cast<std::size_t>(status.st_size), '\0');
    (void)!pread(output_, result.data(), result.size(), 0);
    return result;
  }

  /// Forget what was written
  void clear() const {
    (void)!ftruncate(output_, 0);
    (void)lseek(output_, 0, SEEK_SET);
  }

  private:
  static auto temporaryFile() -> int {
    char name[]{"/tmp/jwezel-test-XXXXXX"};
    const auto result{mkstemp(name)};
    unlink(name); // it goes away when it is closed
    return result;
  }

  int input_;
  int output_;
};
