#pragma once

namespace jwezel::ui {

struct Element
{

  ///
  /// Create Element
  ///
  /// @param      parent  The parent
  explicit Element(Element *parent=0);

  Element(const Element &) = default;

  Element(Element &&) = delete;

  auto operator=(const Element &) -> Element & = default;

  auto operator=(Element &&) -> Element & = delete;

  virtual ~Element() = default;

  [[nodiscard]] inline virtual auto window() /*NOLINT(misc-no-recursion)*/ -> Element * {
    return parent_ ? parent_->window() : 0;
  }

  auto parent() {return parent_;}

  private:
  struct Element *parent_;
};

} // namespace jwezel::ui
