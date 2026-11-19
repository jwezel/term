#include <term/geometry.hh>
#include <term/term.hh>
#include <term/text.hh>
#include <ui/button.hh>
#include <ui/label.hh>
#include <ui/ui.hh>
#include <ui/window.hh>

int main() {
  jwezel::Terminal terminal;
  jwezel::ui::Ui ui{terminal};
  jwezel::ui::Window window{ui, jwezel::Space, jwezel::Rectangle{0, 0, 40, 10}, jwezel::Text{"Hello, term"}};
  jwezel::ui::Label hint{window, jwezel::Rectangle{2, 2, 36, 3}, jwezel::Text{"Press Enter or click Quit."}};
  jwezel::ui::Button quit{window, jwezel::Rectangle{2, 5, 12, 6}, jwezel::Text{"Quit"}};
  quit.onPress([&ui](const jwezel::ui::Element &, const jwezel::Event &) {
    ui.stop();
    return true;
  });
  ui.run();
}
