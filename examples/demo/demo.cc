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

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

#include <ext_ui/list_editor.hh>
#include <term/display.hh>
#include <term/event.hh>
#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <term/term.hh>
#include <term/text.hh>
#include <ui/action.hh>
#include <ui/box.hh>
#include <ui/button.hh>
#include <ui/color_style.hh>
#include <ui/drop_down.hh>
#include <ui/horizontal_box.hh>
#include <ui/label.hh>
#include <ui/line_edit.hh>
#include <ui/list_box.hh>
#include <ui/menu.hh>
#include <ui/text_area.hh>
#include <ui/ui.hh>
#include <ui/vertical_box.hh>
#include <ui/window.hh>

#include <algorithm>
#include <deque>
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
  jwezel::i1,
  jwezel::Rectangle,
  jwezel::Rgb,
  jwezel::RgbBlack,
  jwezel::RgbBlue2,
  jwezel::RgbBlue4,
  jwezel::RgbBlue5,
  jwezel::RgbBlue6,
  jwezel::RgbBlue7,
  jwezel::RgbCyan7,
  jwezel::RgbGray1,
  jwezel::RgbGray4,
  jwezel::RgbGray6,
  jwezel::RgbGray7,
  jwezel::RgbGray8,
  jwezel::RgbGray9,
  jwezel::RgbNone,
  jwezel::RectangleDefault,
  jwezel::RgbWhite,
  jwezel::RgbYellow,
  jwezel::Terminal,
  jwezel::Text,
  jwezel::Vector,
  jwezel::VectorMax,
  jwezel::VectorMin,
  jwezel::bold,
  jwezel::ui::Action,
  jwezel::ui::BoxLayout,
  jwezel::ui::Button,
  jwezel::ui::Container,
  jwezel::ui::ContentAlignment,
  jwezel::ui::ColorRoles,
  jwezel::ui::ColorStyle,
  jwezel::ui::DropDown,
  jwezel::ui::HorizontalBox,
  jwezel::ui::LayoutSize,
  jwezel::ui::Label,
  jwezel::ui::LineEdit,
  jwezel::ui::ListBox,
  jwezel::ui::Menu,
  jwezel::ui::MenuItem,
  jwezel::ui::TextArea,
  jwezel::ui::Ui,
  jwezel::ui::UiStyle,
  jwezel::ui::VerticalBox,
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

// A buddy label's punctuation is shared by the whole demo rather than included in each caption.
constexpr UiStyle demoStyle{.labelSuffix = ":"};

const Char desktopBackground{' ', RgbNone, RgbGray1};
const Char mainBackground{' ', RgbNone, RgbBlue2};
const Char dialogBackground{' ', RgbNone, RgbGray9};

//~Helpers~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Text without the line ends
auto plain(const Text &text) -> std::string {
  auto result{std::string(text)};
  while (not result.empty() and result.back() == '\n') {
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
  std::vector<std::string> traits;
  std::vector<std::string> preferredTraits;
  std::string notes;

  [[nodiscard]] auto displayName() const -> std::string {
    auto result{title};
    for (const auto &part: {firstName, middleName, lastName}) {
      if (not part.empty()) {
        result += (result.empty()? "": " ") + part;
      }
    }
    if (result.empty()) {
      result = "Unnamed contact";
    }
    return result;
  }

  [[nodiscard]] auto summary() const -> std::string {
    auto result{displayName()};
    for (const auto &part: {city, email}) {
      if (not part.empty()) {
        result += std::format("  ·  {}", part);
      }
    }
    for (const auto &trait: traits) {
      const auto preferred{std::find(preferredTraits.begin(), preferredTraits.end(), trait) != preferredTraits.end()};
      result += std::format("  ·  {}{}", trait, preferred? " *": "");
    }
    return result;
  }
};

//~Dialog~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// A parallel label/editor column. The buddy relationship makes corresponding rows share their preferred heights.
struct FieldColumn {
  FieldColumn(Container &parent, size_t rows, bool stretchRows=false):
  layout{
    parent, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = {LayoutSize::fit, LayoutSize::stretch},
      .alignments = {},
      .spacing = 1,
    }
  },
  labels{
    layout, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = std::vector<LayoutSize>(rows, stretchRows? LayoutSize::stretch: LayoutSize::fit),
      .alignments = {},
    }
  },
  editors{
    layout, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = std::vector<LayoutSize>(rows, stretchRows? LayoutSize::stretch: LayoutSize::fit),
      .alignments = {},
    }
  }
  {}

  HorizontalBox layout;
  VerticalBox labels;
  VerticalBox editors;
};

/// Two field columns arranged side by side.
struct FieldPair {
  FieldPair(Container &parent, size_t rows):
  layout{
    parent, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = {LayoutSize::stretch, LayoutSize::stretch},
      .alignments = {},
      .spacing = 2,
    }
  },
  left{layout, rows},
  right{layout, rows}
  {}

  HorizontalBox layout;
  FieldColumn left;
  FieldColumn right;
};

/// A compact, consistently aligned label and single-line editor.
struct TextField {
  TextField(
    Container &labelParent,
    Container &inputParent,
    std::string_view caption,
    std::string_view initial={}
  ):
  input{inputParent, initial},
  label{labelParent, Text{caption}}
  {
    label.buddy(input);
    label.verticalAlignment(ContentAlignment::center);
  }

  [[nodiscard]] auto value() const -> std::string {return input.contents();}

  LineEdit input;
  Label label;
};

/// A label and a choice list, for values that should come from a known set.
struct ChoiceField {
  ChoiceField(
    Container &labelParent,
    Container &inputParent,
    std::string_view caption,
    const std::vector<Text> &options,
    Dim selected=0
  ):
  input{inputParent, options, selected},
  label{labelParent, Text{caption}}
  {
    label.buddy(input);
    label.verticalAlignment(ContentAlignment::center);
  }

  [[nodiscard]] auto value() const -> std::string {
    const auto selected{input.selected()};
    if (selected < 0) {
      return {};
    }
    auto result{plain(input.items()[static_cast<size_t>(selected)])};
    return result == "Select"? std::string{}: result;
  }

  DropDown input;
  Label label;
};

/// A label and a multi-line editor; the containing row may stretch both to the same height.
struct NotesField {
  NotesField(Container &labelParent, Container &inputParent):
  input{inputParent, ""},
  label{labelParent, Text{"Notes"}}
  {
    label.buddy(input);
    label.verticalAlignment(ContentAlignment::center);
  }

  [[nodiscard]] auto value() const -> std::string {return input.contents();}

  TextArea input;
  Label label;
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
  static constexpr Dim height{22};

  Dialog(Ui &ui, const Vector &position, const ColorStyle *colors, const UiStyle *style, const Done &done):
  window_{
    ui, dialogBackground,
    Rectangle{position, Vector{static_cast<Dim>(position.x() + width), static_cast<Dim>(position.y() + height)}},
    Text{" New person & address "}, nullptr, colors, style
  },
  body_{
    window_, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = {
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::fit,
        LayoutSize::stretch,
        LayoutSize::fit,
      },
      .alignments = {},
      .padding = 1,
    }
  },
  identityHeader_{
    body_, Text{"PERSONAL  /  BIRTH DATE: YYYY-MM-DD", RgbBlue5, RgbNone, bold}
  },
  personal_{body_, 3},
  title_{
    personal_.left.labels, personal_.left.editors, "Title",
    {Text{"Select"}, Text{"Mx."}, Text{"Ms."}, Text{"Mr."}, Text{"Dr."}, Text{"Prof."}}
  },
  firstName_{personal_.right.labels, personal_.right.editors, "First name"},
  middleName_{personal_.left.labels, personal_.left.editors, "Middle"},
  lastName_{personal_.right.labels, personal_.right.editors, "Last name"},
  birthDate_{personal_.left.labels, personal_.left.editors, "Birth date"},
  gender_{personal_.right.labels, personal_.right.editors, "Gender", {
    Text{"Select"}, Text{"Woman"}, Text{"Man"}, Text{"Non-binary"}, Text{"Self-describe"}, Text{"Prefer not to say"},
  }},
  traitsRow_{
    body_, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = {LayoutSize::fit, LayoutSize::stretch},
      .alignments = {jwezel::ui::LayoutAlign::center, jwezel::ui::LayoutAlign::stretch},
      .spacing = 1,
    }
  },
  traitsLabel_{traitsRow_, Text{"Traits"}},
  traits_{traitsRow_},
  contactHeader_{body_, Text{"CONTACT  /  HOW TO REACH THEM", RgbBlue5, RgbNone, bold}},
  contact_{body_, 3},
  email_{contact_.left.labels, contact_.left.editors, "Email"},
  phone_{contact_.right.labels, contact_.right.editors, "Phone"},
  mobile_{contact_.left.labels, contact_.left.editors, "Mobile"},
  website_{contact_.right.labels, contact_.right.editors, "Website"},
  organization_{contact_.left.labels, contact_.left.editors, "Company"},
  jobTitle_{contact_.right.labels, contact_.right.editors, "Job title"},
  addressHeader_{body_, Text{"ADDRESS  /  WHERE THEY LIVE", RgbBlue5, RgbNone, bold}},
  address_{body_, 3},
  addressLine1_{address_.left.labels, address_.left.editors, "Street"},
  addressLine2_{address_.right.labels, address_.right.editors, "Apt / unit"},
  city_{address_.left.labels, address_.left.editors, "City"},
  region_{address_.right.labels, address_.right.editors, "Region"},
  postalCode_{address_.left.labels, address_.left.editors, "Post code"},
  country_{address_.right.labels, address_.right.editors, "Country", {
    Text{"United States"},
    Text{"Canada"},
    Text{"United Kingdom"},
    Text{"Australia"},
    Text{"New Zealand"},
    Text{"France"},
    Text{"Germany"},
    Text{"India"},
    Text{"Japan"},
    Text{"Other"},
  }},
  notes_{body_, 1, true},
  notesField_{notes_.labels, notes_.editors},
  footer_{
    body_, RectangleDefault,
    BoxLayout{
      .weights = {},
      .sizes = {LayoutSize::stretch, LayoutSize::fit, LayoutSize::fit},
      .alignments = {},
      .spacing = 1,
    }
  },
  hint_{footer_, Text{"Tab moves  ·  Ctrl-click marks preferred  ·  Esc cancels", RgbGray4}},
  ok_{footer_, Text{"Save"}},
  cancel_{footer_, Text{"Cancel"}},
  accept_{[this, done] {done(true, person());}, {Key::Enter}},
  reject_{[done] {done(false, {});}, {Key::Escape, '\x03'}}
  // Ctrl-C comes as a key as well, the terminal is in raw mode
  {
    traitsLabel_.verticalAlignment(ContentAlignment::center);
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
    auto traits{std::vector<std::string>{}};
    for (const auto &trait: traits_.choices()) {
      traits.push_back(plain(trait));
    }
    auto preferredTraits{std::vector<std::string>{}};
    for (const auto &trait: traits_.selected()) {
      preferredTraits.push_back(plain(trait));
    }
    return PersonRecord{
      .title = title_.value(),
      .firstName = firstName_.value(),
      .middleName = middleName_.value(),
      .lastName = lastName_.value(),
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
      .traits = std::move(traits),
      .preferredTraits = std::move(preferredTraits),
      .notes = notesField_.value(),
    };
  }

  Window window_;
  VerticalBox body_;
  Label identityHeader_;
  FieldPair personal_;
  ChoiceField title_;
  TextField firstName_;
  TextField middleName_;
  TextField lastName_;
  TextField birthDate_;
  ChoiceField gender_;
  HorizontalBox traitsRow_;
  Label traitsLabel_;
  jwezel::ui::ListEditor traits_;
  Label contactHeader_;
  FieldPair contact_;
  TextField email_;
  TextField phone_;
  TextField mobile_;
  TextField website_;
  TextField organization_;
  TextField jobTitle_;
  Label addressHeader_;
  FieldPair address_;
  TextField addressLine1_;
  TextField addressLine2_;
  TextField city_;
  TextField region_;
  TextField postalCode_;
  ChoiceField country_;
  FieldColumn notes_;
  NotesField notesField_;
  HorizontalBox footer_;
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
        selection.index(),
        plain(selection.item())
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
  window_{
    ui_, mainBackground, Rectangle{margin, 1, static_cast<Dim>(margin + width), static_cast<Dim>(1 + height)},
    Text{" jwezel UI demo "}, nullptr, &mainColors, &demoStyle
  },
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
      static_cast<Dim>(area.x1() + (width - Dialog::width) / 2),
      static_cast<Dim>(area.y1() + (height - Dialog::height) / 2)
    };
    // Modal windows take the input exclusively while they are open
    dialog_.emplace(
      ui_, position, colored_? &dialogColors: nullptr, &demoStyle,
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
    colored_ = not colored_;
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
      desktopBackground,
      VectorMin,
      VectorMin,
      VectorMax,
      1,
      0,
      true,
      false,
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

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
