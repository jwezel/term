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
#include <ui/list_box.hh>
#include <ui/menu.hh>
#include <ui/text_area.hh>
#include <ui/ui.hh>
#include <ui/window.hh>

#include <deque>
#include <algorithm>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using
  jwezel::Char,
  jwezel::CharAttributes,
  jwezel::Dim,
  jwezel::Event,
  jwezel::Key,
  jwezel::KeyEvent,
  jwezel::i1,
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
  jwezel::ui::ListBox,
  jwezel::ui::Menu,
  jwezel::ui::MenuItem,
  jwezel::ui::TextArea,
  jwezel::ui::Ui,
  jwezel::ui::Window;

namespace {

//~Colors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// A color style is a constant table of named roles. What a role does not say is taken from what is there: the
// window background shines through wherever a role has no background.

// Dark blue main window
constexpr ColorRoles<21> mainRoles{
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
  {"list", CharAttributes{RgbWhite, RgbBlue2}},
  {"list.focus", CharAttributes{RgbWhite, RgbBlue2}},
  {"list.selected", CharAttributes{RgbBlack, RgbCyan7, bold}},
  {"list.current", CharAttributes{RgbWhite, RgbBlue7, bold}},
  {"list.current.selected", CharAttributes{RgbBlack, RgbYellow, bold}},
  {"list.disabled", CharAttributes{RgbGray6}},
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

/// Value-owned data collected by the address-book dialog.
struct PersonRecord {
  std::string title;
  std::string firstName;
  std::string middleName;
  std::string lastName;
  std::string preferredName;
  std::string pronouns;
  std::string birthDate;
  std::string gender;
  std::string email;
  std::string phone;
  std::string mobile;
  std::string website;
  std::string company;
  std::string jobTitle;
  std::string addressLine1;
  std::string addressLine2;
  std::string city;
  std::string region;
  std::string postalCode;
  std::string country;
  std::string emergencyContact;
  std::string emergencyPhone;
  std::string notes;

  [[nodiscard]] auto displayName() const -> std::string {
    auto result{title};
    for (const auto &part: {firstName, middleName, lastName}) {
      if (!part.empty()) {
        result += (result.empty()? "": " ") + part;
      }
    }
    if (result.empty()) {
      result = "Unnamed contact";
    }
    if (!preferredName.empty()) {
      result += std::format(" ({})", preferredName);
    }
    return result;
  }

  [[nodiscard]] auto summary() const -> std::string {
    auto result{displayName()};
    for (const auto &part: {city, email}) {
      if (!part.empty()) {
        result += std::format("  ·  {}", part);
      }
    }
    return result;
  }
};

//~Dialog~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

auto leftLabel(Dim row) -> Rectangle {return Rectangle{3, row, 13, static_cast<Dim>(row + 1)};}

auto leftInput(Dim row) -> Rectangle {return Rectangle{13, row, 34, static_cast<Dim>(row + 1)};}

auto rightLabel(Dim row) -> Rectangle {return Rectangle{35, row, 45, static_cast<Dim>(row + 1)};}

auto rightInput(Dim row) -> Rectangle {return Rectangle{45, row, 64, static_cast<Dim>(row + 1)};}

/// A compact, consistently aligned label and single-line editor.
struct TextField {
  TextField(
    Window &parent,
    const Rectangle &labelArea,
    const Rectangle &inputArea,
    std::string_view caption,
    std::string_view initial={}
  ):
  label{parent, labelArea, Text{caption}},
  input{parent, inputArea, initial}
  {}

  [[nodiscard]] auto value() const -> std::string {return input.contents();}

  Label label;
  LineEdit input;
};

/// A label and a choice list, for values that should come from a known set.
struct ChoiceField {
  ChoiceField(
    Window &parent,
    const Rectangle &labelArea,
    const Rectangle &inputArea,
    std::string_view caption,
    const std::vector<Text> &options,
    Dim selected=0
  ):
  label{parent, labelArea, Text{caption}},
  input{parent, inputArea, options, selected}
  {}

  [[nodiscard]] auto value() const -> std::string {
    const auto selected{input.selected()};
    if (selected < 0) {
      return {};
    }
    auto result{plain(input.items()[static_cast<size_t>(selected)])};
    return result == "Select"? std::string{}: result;
  }

  Label label;
  DropDown input;
};

///
/// A detailed person and address form. It uses the same ordinary widgets as the rest of the demo: line edits for
/// free-form values, drop-downs for choices, and a small multi-line notes field.
///
/// The dialog owns its window and widgets as plain members; they are destroyed in reverse order, widgets first.
/// The result is reported to `done`, which must not destroy the dialog right away: the dialog is in the middle of
/// handling an event then. The owner destroys it when it is back in its loop.
struct Dialog {
  using Done = std::function<void(bool accepted, const PersonRecord &person)>;

  static constexpr Dim width{66};
  static constexpr Dim height{21};

  Dialog(Ui &ui, const Vector &position, const ColorStyle *colors, const Done &done):
  window_{
    ui, dialogBackground,
    Rectangle{position, Vector{static_cast<Dim>(position.x() + width), static_cast<Dim>(position.y() + height)}},
    Text{" New person & address "}, nullptr, colors
  },
  identityHeader_{
    window_, Rectangle{3, 1, 64, 2}, Text{"PERSONAL  /  BIRTH DATE: YYYY-MM-DD", RgbBlue5, RgbNone, bold}
  },
  title_{
    window_, leftLabel(2), leftInput(2), "Title",
    {Text{"Select"}, Text{"Mx."}, Text{"Ms."}, Text{"Mr."}, Text{"Dr."}, Text{"Prof."}}
  },
  firstName_{window_, rightLabel(2), rightInput(2), "First name"},
  middleName_{window_, leftLabel(3), leftInput(3), "Middle"},
  lastName_{window_, rightLabel(3), rightInput(3), "Last name"},
  preferredName_{window_, leftLabel(4), leftInput(4), "Preferred"},
  pronouns_{
    window_, rightLabel(4), rightInput(4), "Pronouns",
    {Text{"Select"}, Text{"she/her"}, Text{"he/him"}, Text{"they/them"}, Text{"use name"}}
  },
  birthDate_{window_, leftLabel(5), leftInput(5), "Birth date"},
  gender_{window_, rightLabel(5), rightInput(5), "Gender", {
    Text{"Select"}, Text{"Woman"}, Text{"Man"}, Text{"Non-binary"}, Text{"Self-describe"}, Text{"Prefer not to say"}
  }},
  contactHeader_{window_, Rectangle{3, 6, 64, 7}, Text{"CONTACT  /  HOW TO REACH THEM", RgbBlue5, RgbNone, bold}},
  email_{window_, leftLabel(7), leftInput(7), "Email"},
  phone_{window_, rightLabel(7), rightInput(7), "Phone"},
  mobile_{window_, leftLabel(8), leftInput(8), "Mobile"},
  website_{window_, rightLabel(8), rightInput(8), "Website"},
  organization_{window_, leftLabel(9), leftInput(9), "Company"},
  jobTitle_{window_, rightLabel(9), rightInput(9), "Job title"},
  addressHeader_{window_, Rectangle{3, 10, 64, 11}, Text{"ADDRESS  /  WHERE THEY LIVE", RgbBlue5, RgbNone, bold}},
  addressLine1_{window_, Rectangle{3, 11, 13, 12}, Rectangle{13, 11, 64, 12}, "Street"},
  addressLine2_{window_, Rectangle{3, 12, 13, 13}, Rectangle{13, 12, 64, 13}, "Apt / unit"},
  city_{window_, leftLabel(13), leftInput(13), "City"},
  region_{window_, rightLabel(13), rightInput(13), "Region"},
  postalCode_{window_, leftLabel(14), leftInput(14), "Post code"},
  country_{window_, rightLabel(14), rightInput(14), "Country", {
    Text{"United States"}, Text{"Canada"}, Text{"United Kingdom"}, Text{"Australia"}, Text{"New Zealand"},
    Text{"France"}, Text{"Germany"}, Text{"India"}, Text{"Japan"}, Text{"Other"}
  }},
  emergencyHeader_{window_, Rectangle{3, 15, 64, 16}, Text{"IN CASE OF EMERGENCY", RgbBlue5, RgbNone, bold}},
  emergencyName_{window_, leftLabel(16), leftInput(16), "Contact"},
  emergencyPhone_{window_, rightLabel(16), rightInput(16), "Phone"},
  notesLabel_{window_, Rectangle{3, 17, 13, 19}, Text{"Notes"}},
  notes_{window_, Rectangle{13, 17, 64, 19}, ""},
  hint_{window_, Rectangle{3, 19, 42, 20}, Text{"Tab moves  ·  Esc cancels", RgbGray4}},
  ok_{window_, Rectangle{43, 19, 52, 20}, Text{"Save"}},
  cancel_{window_, Rectangle{54, 19, 64, 20}, Text{"Cancel"}},
  accept_{[this, done] {done(true, person());}, {Key::Enter}},
  reject_{[done] {done(false, {});}, {Key::Escape, '\x03'}}
  // Ctrl-C comes as a key as well, the terminal is in raw mode
  {
    // Enter and Escape are actions of the whole window. Line edits leave Enter for the default action; the notes
    // field consumes it as a newline. The buttons trigger the same actions, and Tab follows field creation order.
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
  [[nodiscard]] auto person() const -> PersonRecord {
    return PersonRecord{
      .title = title_.value(),
      .firstName = firstName_.value(),
      .middleName = middleName_.value(),
      .lastName = lastName_.value(),
      .preferredName = preferredName_.value(),
      .pronouns = pronouns_.value(),
      .birthDate = birthDate_.value(),
      .gender = gender_.value(),
      .email = email_.value(),
      .phone = phone_.value(),
      .mobile = mobile_.value(),
      .website = website_.value(),
      .company = organization_.value(),
      .jobTitle = jobTitle_.value(),
      .addressLine1 = addressLine1_.value(),
      .addressLine2 = addressLine2_.value(),
      .city = city_.value(),
      .region = region_.value(),
      .postalCode = postalCode_.value(),
      .country = country_.value(),
      .emergencyContact = emergencyName_.value(),
      .emergencyPhone = emergencyPhone_.value(),
      .notes = notes_.contents()
    };
  }

  Window window_;
  Label identityHeader_;
  ChoiceField title_;
  TextField firstName_;
  TextField middleName_;
  TextField lastName_;
  TextField preferredName_;
  ChoiceField pronouns_;
  TextField birthDate_;
  ChoiceField gender_;
  Label contactHeader_;
  TextField email_;
  TextField phone_;
  TextField mobile_;
  TextField website_;
  TextField organization_;
  TextField jobTitle_;
  Label addressHeader_;
  TextField addressLine1_;
  TextField addressLine2_;
  TextField city_;
  TextField region_;
  TextField postalCode_;
  ChoiceField country_;
  Label emergencyHeader_;
  TextField emergencyName_;
  TextField emergencyPhone_;
  Label notesLabel_;
  TextArea notes_;
  Label hint_;
  Button ok_;
  Button cancel_;
  Action accept_; // the default action
  Action reject_; // the cancel action
};

/// Lazy view of the saved contacts. It deliberately implements only `operator[]` to demonstrate the source concept.
struct ContactSource {
  const std::vector<PersonRecord> &contacts;

  // The demo caps saved contacts at 127, so i1 (int8_t) can represent the count and every row index.
  [[nodiscard]] auto size() const -> i1 {return static_cast<i1>(contacts.size());}

  [[nodiscard]] auto operator[](i1 index) const -> Text {
    return Text{contacts[static_cast<std::size_t>(index)].summary()};
  }
};

/// Separate window that showcases a lazy, byte-indexed, multi-selection contact list.
struct ContactsWindow {
  using Log = std::function<void(std::string)>;
  using Done = std::function<void()>;

  ContactsWindow(Ui &ui, const Rectangle &area, std::vector<PersonRecord> &contacts, const Log &log, const Done &done):
  contacts_{contacts},
  window_{ui, mainBackground, area, Text{" Saved contacts "}, nullptr, &mainColors},
  heading_{window_, Rectangle{2, 1, 46, 2}, Text{"Space selects  Drag selects range"}},
  source_{contacts},
  list_{window_, Rectangle{2, 2, 46, 10}, jwezel::ui::listSource(source_)},
  help_{window_, Rectangle{2, 10, 46, 11}, Text{"Delete removes 0 selected"}},
  footer_{window_, Rectangle{2, 11, 46, 12}, Text{"Ctrl-drag extends  ·  Esc clears/closes"}},
  removeSelected_{[this, log] {remove(log);}, {Key::Delete}},
  close_{[done] {done();}, {Key::Escape}}
  {
    window_.addAction(removeSelected_);
    window_.addAction(close_);
    list_.onSelection([this, log](const jwezel::ui::Element &, const Event &event) {
      const auto &selection{event.as<jwezel::ui::SelectionEvent>()};
      log(std::format(
        "contacts: {} #{} {}",
        selection.selected()? "selected": "deselected",
        selection.index(), plain(selection.item())
      ));
      updateHelp();
      return true;
    });
    list_.onActivate([log](const jwezel::ui::Element &, const Event &event) {
      const auto &activated{event.as<jwezel::ui::ActivateEvent>()};
      log(std::format("contacts: activated #{} {}", activated.index(), plain(activated.item())));
      return true;
    });
  }

  ContactsWindow(const ContactsWindow &) = delete;
  ContactsWindow(ContactsWindow &&) = delete;
  auto operator=(const ContactsWindow &) -> ContactsWindow & = delete;
  auto operator=(ContactsWindow &&) -> ContactsWindow & = delete;

  private:
  void updateHelp() {
    help_.content(Text{std::format("Delete removes {} selected", list_.selection().size())});
  }

  void remove(const Log &log) {
    std::vector<i1> indices;
    for (const auto index: list_.selection()) {
      indices.push_back(index);
    }
    if (indices.empty()) {
      return;
    }
    list_.clearSelection();
    for (auto index = indices.rbegin(); index != indices.rend(); ++index) {
      contacts_.erase(contacts_.begin() + *index);
    }
    list_.refresh();
    updateHelp();
    log(std::format("contacts: removed {}", indices.size()));
  }

  std::vector<PersonRecord> &contacts_;
  Window window_;
  Label heading_;
  ContactSource source_;
  ListBox<i1> list_;
  Label help_;
  Label footer_;
  Action removeSelected_;
  Action close_;
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
  openDialog_{[this] {openDialog();}},
  openContacts_{[this] {openContacts();}},
  toggleColorsAction_{[this] {toggleColors();}},
  exit_{[this] {quit();}},
  quit_{[this] {quit();}, {'\x03'}}, // Ctrl-C comes as a key, the terminal is in raw mode
  menu_{
    window_, Rectangle{2, 1, 24, 2},
    std::vector<MenuItem>{
      {Text{"Address..."}, &openDialog_},
      {Text{"Contacts..."}, &openContacts_},
      {Text{"Toggle colors"}, &toggleColorsAction_},
      {Text{"Exit"}, &exit_}
    }
  },
  title_{window_, Rectangle{22, 1, 62, 2}, Text{"Everything you see is a widget"}},
  notesLabel_{window_, Rectangle{2, 3, 62, 4}, Text{"Notes"}},
  notes_{
    window_, Rectangle{2, 4, 62, 10},
    "This is a text area.\nType, move with the arrow keys,\nor click to put the cursor somewhere."
  },
  eventsLabel_{window_, Rectangle{2, 11, 62, 12}, Text{"Events"}},
  events_{window_, Rectangle{2, 12, 62, 17}, Text{}},
  hint_{window_, Rectangle{2, 18, 62, 19}, Text{"Tab: next  Space: menu  Esc: close  Ctrl-C: quit"}}
  {
    window_.addAction(quit_); // Tab and Shift-Tab go from the menu to the notes and back, without anybody saying so
    menu_.caption(Text{"Menu"}); // a menu shows what it is called, not what was chosen from it
    log("started");
  }

  Demo(const Demo &) = delete;

  Demo(Demo &&) = delete;

  auto operator=(const Demo &) -> Demo & = delete;

  auto operator=(Demo &&) -> Demo & = delete;

  ~Demo() = default;

  void run() {
    ui_.run();
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

  void quit() {
    ui_.post([this] {ui_.stop();});
  }

  void openDialog() {
    if (dialog_) {
      log("dialog: already open");
      return;
    }
    const auto area{window_.area()};
    const Vector position{
      static_cast<Dim>(area.x1() + (width - Dialog::width) / 2), static_cast<Dim>(area.y1() + (height - Dialog::height) / 2)
    };
    // Modal windows take the input exclusively while they are open
    dialog_.emplace(
      ui_, position, colored_? &dialogColors: nullptr,
        [this](bool accepted, const PersonRecord &person) {
        if (accepted) {
          if (contacts_.size() < 127) {
            contacts_.push_back(person);
            log(std::format("saved: {}", person.summary()));
          } else {
            log("contacts: limit reached (127)");
          }
        } else {
          log("contact: cancelled");
        }
        ui_.post([this] {
          dialog_.reset();
          terminal_.focus(&window_);
        });
      }
    );
    dialog_->window().modal(true);
    log("dialog: opened");
  }

  void openContacts() {
    if (contactsWindow_) {
      log("contacts: already open");
      return;
    }
    const auto size{terminal_.display().size()};
    const auto width{std::min(Dim{50}, static_cast<Dim>(size.x() - 2))};
    const auto height{std::min(Dim{14}, static_cast<Dim>(size.y() - 2))};
    const auto x{static_cast<Dim>((size.x() - width) / 2)};
    const auto y{static_cast<Dim>((size.y() - height) / 2)};
    contactsWindow_.emplace(
      ui_, Rectangle{x, y, static_cast<Dim>(x + width), static_cast<Dim>(y + height)}, contacts_,
      [this](std::string message) {log(message);},
      [this] {
        ui_.post([this] {
          contactsWindow_.reset();
          terminal_.focus(&window_);
        });
      }
    );
    log(std::format("contacts: opened ({} saved)", contacts_.size()));
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
  Action openDialog_;
  Action openContacts_;
  Action toggleColorsAction_;
  Action exit_;
  Action quit_;
  Menu menu_;
  Label title_;
  Label notesLabel_;
  TextArea notes_;
  Label eventsLabel_;
  Label events_;
  Label hint_;
  std::deque<std::string> lines_;
  std::vector<PersonRecord> contacts_;
  bool colored_{true};
  std::optional<Dialog> dialog_; // last: destroyed first
  std::optional<ContactsWindow> contactsWindow_;
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
