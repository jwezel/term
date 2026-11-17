///
/// Demo of the UI library
///
/// A main window with a menu (a drop down), a text area and an event log, and a dialog window that the menu opens.
/// It is a show case: look at the TODO(feature) notes for what a real application would miss.
///
/// Keys: Tab / Shift-Tab move the focus, Space / Enter / Down open the menu, Esc closes the dialog, Ctrl-C quits.
/// The mouse works as well: click to focus, click a menu entry, drag from the menu to an entry and let go.
///
/// Like the other examples, the demo draws where the cursor is, scrolls the terminal if there is not enough room below
/// it, and leaves the screen to the shell afterwards. With --fullscreen it uses the alternate screen instead.

#include <term/display.hh>
#include <term/event.hh>
#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <term/term.hh>
#include <term/text.hh>
#include <ui/action.hh>
#include <ui/button.hh>
#include <ui/color_style.hh>
#include <ui/drop_down.hh>
#include <ui/label.hh>
#include <ui/line_edit.hh>
#include <ui/text_area.hh>
#include <ui/ui.hh>
#include <ui/window.hh>

#include <deque>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using
  jwezel::Char,
  jwezel::CharAttributes,
  jwezel::Dim,
  jwezel::Event,
  jwezel::Key,
  jwezel::KeyEvent,
  jwezel::Rectangle,
  jwezel::Rgb,
  jwezel::RgbBlack,
  jwezel::RgbBlue2,
  jwezel::RgbBlue4,
  jwezel::RgbBlue5,
  jwezel::RgbBlue6,
  jwezel::RgbBlue7,
  jwezel::RgbCyan5,
  jwezel::RgbCyan7,
  jwezel::RgbGray1,
  jwezel::RgbGray4,
  jwezel::RgbGray6,
  jwezel::RgbGray7,
  jwezel::RgbGray8,
  jwezel::RgbGray9,
  jwezel::RgbNone,
  jwezel::RgbWhite,
  jwezel::RgbYellow,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin,
  jwezel::bold,
  jwezel::ui::Action,
  jwezel::ui::Button,
  jwezel::ui::ColorRoles,
  jwezel::ui::ColorStyle,
  jwezel::ui::DropDown,
  jwezel::ui::Label,
  jwezel::ui::LineEdit,
  jwezel::ui::TextArea,
  jwezel::ui::Ui,
  jwezel::ui::Window;

namespace {

//~Colors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// A color style is a constant table of named roles. What a role does not say is taken from what is there: the
// window background shines through wherever a role has no background.

// Dark blue main window
constexpr ColorRoles<15> mainRoles{
  {"frame", CharAttributes{RgbGray6}},
  {"frame.active", CharAttributes{RgbCyan7}},
  {"frame.inactive", CharAttributes{RgbGray4}},
  {"title", CharAttributes{RgbGray8}},
  {"title.active", CharAttributes{RgbYellow, RgbNone, bold}},
  {"title.inactive", CharAttributes{RgbGray6}},
  {"label", CharAttributes{RgbGray9}},
  {"button", CharAttributes{RgbBlack, RgbGray7}},
  {"button.focus", CharAttributes{RgbBlack, RgbCyan7}},
  {"input", CharAttributes{RgbWhite, RgbBlue4}},
  {"input.focus", CharAttributes{RgbWhite, RgbBlue6}},
  {"dropdown", CharAttributes{RgbBlack, RgbGray7}},
  {"dropdown.focus", CharAttributes{RgbBlack, RgbCyan7}},
  {"popup", CharAttributes{RgbBlack, RgbGray8}},
  {"popup.highlight", CharAttributes{RgbWhite, RgbBlue7, bold}},
};
constexpr ColorStyle mainColors{mainRoles};

// Light dialog; it only has to name what is different from the main window, but it is a different table
constexpr Rgb pale{0.88, 0.93, 1.0};
constexpr ColorRoles<13> dialogRoles{
  {"frame", CharAttributes{RgbBlue5}},
  {"frame.inactive", CharAttributes{RgbGray4}},
  {"title", CharAttributes{RgbBlack, RgbNone, bold}},
  {"title.inactive", CharAttributes{RgbGray6}},
  {"label", CharAttributes{RgbBlack}},
  {"button", CharAttributes{RgbWhite, RgbBlue5}},
  {"button.focus", CharAttributes{RgbWhite, RgbBlue7}},
  {"input", CharAttributes{RgbBlack, RgbWhite}},
  {"input.focus", CharAttributes{RgbBlack, pale}},
  {"dropdown", CharAttributes{RgbWhite, RgbBlue5}},
  {"dropdown.focus", CharAttributes{RgbWhite, RgbBlue7}},
  {"popup", CharAttributes{RgbBlack, RgbGray9}},
  {"popup.highlight", CharAttributes{RgbWhite, RgbBlue7, bold}},
};
constexpr ColorStyle dialogColors{dialogRoles};

const Char desktopBackground{' ', RgbNone, RgbGray1};
const Char mainBackground{' ', RgbNone, RgbBlue2};
const Char dialogBackground{' ', RgbNone, RgbGray9};

//~Helpers~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Text without the line ends
auto plain(const Text &text) -> std::string {
  auto result{std::string(text)};
  while (!result.empty() and result.back() == '\n') {
    result.pop_back();
  }
  return result;
}

//~Dialog~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// Dialog to enter an address, with OK and Cancel
///
/// The dialog owns its window and widgets as plain members; they are destroyed in reverse order, widgets first.
/// The result is reported to `done`, which must not destroy the dialog right away: the dialog is in the middle of
/// handling an event then. The owner destroys it when it is back in its loop.
struct Dialog {
  using Done = std::function<void(bool accepted, const std::string &address)>;

  static constexpr Dim width{46};
  static constexpr Dim height{11};

  Dialog(Ui &ui, const Vector &position, const ColorStyle *colors, const Done &done):
  window_{
    ui, dialogBackground, Rectangle{position, Vector{static_cast<Dim>(position.x() + width), static_cast<Dim>(position.y() + height)}},
    Text{" New address "}, nullptr, colors
  },
  prompt_{window_, Rectangle{3, 2, 43, 3}, Text{"Street, number, postal code and city:"}, dialogBackground},
  entry_{window_, Rectangle{3, 3, 43, 4}, Text{}, dialogBackground},
  hint_{window_, Rectangle{3, 5, 43, 6}, Text{"Enter accepts, Esc cancels, Tab moves on"}, dialogBackground},
  ok_{window_, Rectangle{24, 8, 30, 9}, Text{"OK"}, dialogBackground},
  cancel_{window_, Rectangle{32, 8, 42, 9}, Text{"Cancel"}, dialogBackground},
  accept_{[this, done] {done(true, plain(entry_.content()));}, {Key::Enter}},
  reject_{[done] {done(false, {});}, {Key::Escape, '\x03'}} // Ctrl-C comes as a key as well, the terminal is in raw mode
  {
    // Enter and Escape are actions of the whole window. Nothing has to be said about them in the widgets: an entry
    // that has no use for Enter leaves it to the window, and the buttons trigger the very same actions when they
    // are pressed. The order of the focus is the order in which the widgets were created.
    window_.addAction(accept_);
    window_.addAction(reject_);
    ok_.addAction(accept_);
    cancel_.addAction(reject_);
  }

  Dialog(const Dialog &) = delete;

  Dialog(Dialog &&) = delete;

  auto operator=(const Dialog &) -> Dialog & = delete;

  auto operator=(Dialog &&) -> Dialog & = delete;

  ~Dialog() = default;

  [[nodiscard]] auto window() -> Window & {return window_;}

  private:
  Window window_;
  Label prompt_;
  LineEdit entry_;
  Label hint_;
  Button ok_;
  Button cancel_;
  Action accept_; // the default action
  Action reject_; // the cancel action
};

//~Demo~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// The demo application: a main window with a menu
struct Demo {
  static constexpr Dim width{64};
  static constexpr Dim height{20};
  static constexpr Dim margin{3};

  explicit Demo(Terminal &terminal):
  terminal_{terminal},
  ui_{terminal},
  window_{ui_, mainBackground, Rectangle{margin, 1, static_cast<Dim>(margin + width), static_cast<Dim>(1 + height)}, Text{" jwezel UI demo "}, nullptr, &mainColors},
  menu_{window_, Rectangle{2, 1, 20, 2}, std::vector<Text>{Text{"Menu"}, Text{"Address..."}, Text{"Toggle colors"}, Text{"Exit"}}, 0, mainBackground},
  title_{window_, Rectangle{22, 1, 62, 2}, Text{"Everything you see is a widget"}, mainBackground},
  notesLabel_{window_, Rectangle{2, 3, 62, 4}, Text{"Notes"}, mainBackground},
  notes_{
    window_, Rectangle{2, 4, 62, 10},
    Text{"This is a text area.\nType, move with the arrow keys,\nor click to put the cursor somewhere."}, mainBackground
  },
  eventsLabel_{window_, Rectangle{2, 11, 62, 12}, Text{"Events"}, mainBackground},
  events_{window_, Rectangle{2, 12, 62, 17}, Text{}, mainBackground},
  hint_{window_, Rectangle{2, 18, 62, 19}, Text{"Tab: next  Space: menu  Esc: close dialog  Ctrl-C: quit"}, mainBackground},
  quit_{[this] {done_ = true;}, {'\x03'}} // Ctrl-C comes as a key, the terminal is in raw mode
  {
    // The menu is an ordinary drop down. Choosing an entry emits a select event; the event itself does not say
    // which entry, the handler has to ask the widget.
    // TODO(feature): events with a payload (the index and item of a selection, the old and new state of a toggle ...)
    menu_.onSelect(
      [this](const jwezel::ui::Element &, const Event &event) {
        menuSelected(event);
        return true;
      }
    );
    window_.addAction(quit_); // Tab and Shift-Tab go from the menu to the notes and back, without anybody saying so
    log("started");
  }

  Demo(const Demo &) = delete;

  Demo(Demo &&) = delete;

  auto operator=(const Demo &) -> Demo & = delete;

  auto operator=(Demo &&) -> Demo & = delete;

  ~Demo() = default;

  void run() {
    while (!done_) {
      terminal_.runEvent();
      // A dialog is not destroyed by its own handlers but here, after the event has been handled.
      // TODO(feature): a way to post work to the event loop (Ui::post), and focus that returns to the window below.
      if (closeDialog_) {
        closeDialog_ = false;
        dialog_.reset();
        terminal_.focus(&window_);
      }
    }
  }

  private:
  void log(const std::string &line) {
    lines_.push_back(line);
    while (lines_.size() > 5) {
      lines_.pop_front();
    }
    std::string joined;
    for (const auto &item: lines_) {
      joined += (joined.empty()? "": "\n") + item;
    }
    events_.content(Text{joined});
  }

  void menuSelected(const Event &event) {
    const auto index{menu_.selected()};
    log(std::format("menu: select #{} \"{}\" ({})", index, plain(menu_.items()[static_cast<size_t>(index)]), event.typeName()));
    menu_.select(0); // a menu shows its caption, not the last choice
    switch (index) {
      case 1:
      openDialog();
      break;

      case 2:
      toggleColors();
      break;

      case 3:
      done_ = true;
      break;

      default:
      break;
    }
  }

  void openDialog() {
    if (dialog_) {
      log("dialog: already open");
      return;
    }
    // TODO(feature): modal windows, which the others do not get input while they are open
    const auto area{window_.area()};
    const Vector position{
      static_cast<Dim>(area.x1() + (width - Dialog::width) / 2), static_cast<Dim>(area.y1() + (height - Dialog::height) / 2)
    };
    dialog_.emplace(
      ui_, position, colored_? &dialogColors: nullptr,
      [this](bool accepted, const std::string &address) {
        log(accepted? std::format("dialog: OK \"{}\"", address): "dialog: cancelled");
        closeDialog_ = true;
      }
    );
    log("dialog: opened");
  }

  void toggleColors() {
    colored_ = !colored_;
    window_.colors(colored_? &mainColors: nullptr);
    if (dialog_) {
      dialog_->window().colors(colored_? &dialogColors: nullptr);
    }
    log(colored_? "colors: demo theme": "colors: default style");
  }

  Terminal &terminal_;
  Ui ui_;
  Window window_;
  DropDown menu_;
  Label title_;
  Label notesLabel_;
  TextArea notes_;
  Label eventsLabel_;
  Label events_;
  Label hint_;
  Action quit_;
  std::deque<std::string> lines_;
  bool colored_{true};
  bool done_{false};
  bool closeDialog_{false};
  std::optional<Dialog> dialog_; // last: destroyed first
};

} // namespace

auto main(int argc, char *argv[]) -> int {
  try {
    const auto fullscreen{argc > 1 and std::string_view{argv[1]} == "--fullscreen"};
    Terminal terminal{
      desktopBackground, VectorMin, VectorMin, VectorMax, 1, 0, true, false,
      fullscreen? jwezel::Display::Screen::alternate: jwezel::Display::Screen::main
    };
    terminal.display().mouseMode(jwezel::Display::MouseMode::anything);
    const auto needed{Vector{Demo::width + 2 * Demo::margin, Demo::height + 2}};
    if (terminal.display().maxSize().x() < needed.x() or terminal.display().maxSize().y() < needed.y()) {
      throw std::runtime_error(std::format("The terminal needs to be at least {} x {}", needed.x(), needed.y()));
    }
    Demo demo{terminal};
    demo.run();
  } catch (const std::exception &error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
