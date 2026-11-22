#include <ext_ui/list_editor.hh>

#include <term/geometry.hh>
#include <term/keyboard.hh>
#include <term/term.hh>
#include <ui/ui.hh>
#include <ui/window.hh>

#include <doctest/doctest.h>

#include <cstdio>
#include <string_view>
#include <unistd.h>
#include <variant>
#include <vector>

using jwezel::operator""_C;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(readability-magic-numbers)

TEST_CASE("List editor") {
  int input[2];
  REQUIRE_EQ(pipe(input), 0);
  const auto sendInput{
    [&input](std::string_view text) {
      const auto written{write(input[1], text.data(), text.size())};
      REQUIRE_EQ(written, static_cast<ssize_t>(text.size()));
    }
  };
  sendInput("\x1b[1;1R\x1b[1;1R\x1b[30;40R");

  auto *output{tmpfile()};
  REQUIRE_NE(output, nullptr);
  {
    jwezel::Terminal terminal('.'_C, jwezel::VectorMin, jwezel::VectorMin, jwezel::VectorMax, fileno(output), input[0]);
    jwezel::ui::Ui ui{terminal};
    auto window{jwezel::ui::Window(ui, ' '_C, jwezel::Rectangle{0, 0, 32, 12}, jwezel::Text{"Traits"})};
    auto parent{jwezel::ui::VerticalBox(window, jwezel::Rectangle{1, 1, 30, 10})};
    auto editor{
      jwezel::ui::ListEditor{parent, {jwezel::Text{"Carnivore"}, jwezel::Text{"Herbivore"}, jwezel::Text{"Omnivore"}}}
    };
    std::vector<jwezel::ui::ListEditorEvent> changes;
    editor.onChange(
      [&changes](const jwezel::ui::Element &, const jwezel::Event &event) {
        changes.push_back(event.as<jwezel::ui::ListEditorEvent>());
        return true;
      }
    );
    const auto send{
      [&terminal, &sendInput](std::string_view text) {
        sendInput(text);
        terminal.runEvent();
      }
    };

    CHECK_EQ(
      editor.choices(),
      std::vector<jwezel::Text>{jwezel::Text{"Carnivore"}, jwezel::Text{"Herbivore"}, jwezel::Text{"Omnivore"}}
    );
    CHECK(editor.selected().empty());
    window.focus(&editor.list());
    send(" ");
    REQUIRE_EQ(changes.size(), 1);
    CHECK_EQ(changes[0].change(), jwezel::ui::ListEditorEvent::Change::selection);
    CHECK_EQ(changes[0].selected(), std::vector<jwezel::Text>{jwezel::Text{"Carnivore"}});
    CHECK(editor.list().selected(0));

    CHECK(editor.add(jwezel::Text{"Insectivore"}));
    CHECK_FALSE(editor.add(jwezel::Text{"Insectivore"}));
    CHECK_FALSE(editor.add(jwezel::Text{}));
    CHECK_EQ(editor.selected(), std::vector<jwezel::Text>{jwezel::Text{"Carnivore"}});
    CHECK_EQ(changes.size(), 1); // programmatic changes do not emit user events

    editor.list().clearSelection();
    editor.list().addSelection(1);
    CHECK(editor.editSelected(jwezel::Text{"Frugivore"}));
    CHECK_FALSE(editor.editSelected(jwezel::Text{"Carnivore"})); // duplicate
    CHECK_EQ(editor.choices()[1], jwezel::Text{"Frugivore"});
    CHECK_EQ(editor.selected(), std::vector<jwezel::Text>{jwezel::Text{"Frugivore"}});
    CHECK(editor.removeSelected());
    CHECK_FALSE(editor.removeSelected());
    CHECK_EQ(
      editor.choices(),
      std::vector<jwezel::Text>{jwezel::Text{"Carnivore"}, jwezel::Text{"Omnivore"}, jwezel::Text{"Insectivore"}}
    );
    CHECK(editor.selected().empty());
    CHECK_EQ(changes.size(), 1);

    window.focus(&editor.list());
    send("\t");
    send("N");
    send("e");
    send("w");
    send("\t");
    send("\r");
    REQUIRE_EQ(changes.size(), 2);
    CHECK_EQ(changes[1].change(), jwezel::ui::ListEditorEvent::Change::choices);
    CHECK_EQ(
      changes[1].choices(),
      std::vector<jwezel::Text>{
        jwezel::Text{"Carnivore"}, jwezel::Text{"Omnivore"}, jwezel::Text{"Insectivore"}, jwezel::Text{"New"}
      }
    );
    CHECK(changes[1].selected().empty());
    CHECK(std::holds_alternative<jwezel::KeyEvent>(changes[1].cause()));
  }
  REQUIRE_FALSE(fclose(output));
  REQUIRE_EQ(close(input[0]), 0);
  REQUIRE_EQ(close(input[1]), 0);
}

// NOLINTEND(readability-magic-numbers)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
