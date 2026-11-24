#pragma once

#include <term/device.hh>
#include <term/geometry.hh>
#include <term/text.hh>

#include <initializer_list>

namespace jwezel {

using std::initializer_list, std::pair;

struct Window;

///
/// Surface of overlapping elements
///
/// The elements are kept in z-order, bottom to top. Each element has fragments: the rectangles
/// of its area that are visible, so not covered by any element above it. The fragments of an element are
/// disjoint, and every visible cell belongs to exactly one fragment.
///
/// The fragments are not recomputed when the surface changes, but updated where it does: an element that is
/// added covers what is below it, an element that is removed or moves away uncovers what it showed for the
/// elements below it, and reordering only concerns the elements that are passed. The device is told about all
/// areas that have to be painted again.
struct Surface {
  struct Element;

  /// A visible rectangular portion of an element after higher z-order elements cover it.
  struct Fragment {
    Fragment(const Rectangle &area, struct Element *element):
    area(area),
    element(element)
    {}

    explicit operator string() const;

    auto operator ==(const Surface::Fragment &other) const -> bool = default;

    // xNOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Rectangle area;
    struct Element *element{};
    // xNOLINTEND(misc-non-private-member-variables-in-classes)
  };

  ///
  /// Abstract Element
  ///
  /// The following method are to be overridden:
  ///
  /// - area
  /// Abstract drawable whose visible fragments are maintained by a `Surface`.
  struct Element {
    /// Attach an element to a surface.
    /// @param surface Surface that tracks the element; the element does not own it.
    explicit Element(Surface *surface);

    Element() = default;

    virtual ~Element();

    Element(const Element &) = default;

    Element(Element &&) = default;

    auto operator=(const Element &) -> Element & = default;

    auto operator=(Element &&) -> Element & = default;

    ///
    /// Get text rectangle
    ///
    /// @param[in]  area  The area
    ///
    /// @return     Text of the specified rectangle
    [[nodiscard]] virtual auto text(const Rectangle &area=RectangleMax) const -> Text = 0;

    ///
    /// Get area
    ///
    /// @return     The area of the element
    [[nodiscard]] virtual auto area() const -> Rectangle = 0;

    virtual void move(const Rectangle &area);

    ///
    /// Called when the element is moved; the default handler does not consume the event.
    virtual auto moveEvent(const Rectangle &/*area*/) -> bool {return false;}

    virtual void update(const vector<Rectangle> &areas);

    virtual void update(const Rectangle &area_);

    ///
    /// Get the window that input directed at this element belongs to
    ///
    /// @return     The window, or nothing if the element is not part of a window
    [[nodiscard]] virtual auto inputWindow() -> Window * {return nullptr;}

    ///
    /// Whether the owner is what shows at a position on the surface this element is on
    ///
    /// This is what the caret uses to find out whether the element that owns it is covered. An
    /// element device follows its content down to the element that is visible there.
    ///
    /// @param[in]  position  The position on the surface
    /// @param[in]  owner     The element the caret belongs to
    ///
    /// @return     True if the owner is visible at the position
    [[nodiscard]] virtual auto shows(const Vector &position, const Element *owner) const -> bool {
      (void)position;
      return this == owner;
    }

    [[nodiscard]] auto fragments() -> auto & {return fragments_;}

    [[nodiscard]] auto fragments() const -> const auto & {return fragments_;}

    [[nodiscard]] auto surface() const -> auto *{return surface_;}

    private:
    vector<Fragment> fragments_;
    size_t canonicalFragments_{0}; // number of fragments the last time they were created from scratch
    Surface *surface_{0};
    friend struct Surface;
  };

  /// Create a surface that sends changed regions to a device.
  /// @param device Non-owning output device; it must outlive the surface.
  /// @param elements Initial elements in bottom-to-top order.
  explicit Surface(Device *device, initializer_list<Element *> elements={});

  Surface() = default;

  Surface(const Surface &) = default;
  Surface(Surface &&) = delete;
  Surface & operator=(const Surface &) = default;
  Surface & operator=(Surface &&) = delete;
  virtual ~Surface() = default;

  void update(const vector<Fragment> &updates);

  virtual void addElement(Element *element, Element *below=0);

  virtual void deleteElement(Element *element, Element *destination=0);

  virtual void reshapeElement(Element *element, const Rectangle &area);

  ///
  /// Move an element above another one
  ///
  /// The backdrop, the lowest element, cannot be moved. Nothing happens if the element is above the
  /// destination already.
  ///
  /// @param      element      The element
  /// @param      destination  The element to get above; nothing stands for the position beyond the top, so the
  ///                          element ends up on top
  void above(Surface::Element *element, Surface::Element *destination=0);

  ///
  /// Move an element below another one
  ///
  /// The backdrop, the lowest element, cannot be moved, and nothing can get below it. Nothing happens if the
  /// element is below the destination already.
  ///
  /// @param      element      The element
  /// @param      destination  The element to get below. Nothing stands for the position beyond the top, so the
  ///                          element ends up on top. The backdrop means the bottom, just above the backdrop.
  void below(Surface::Element *element, Surface::Element *destination=0);

  ///
  /// Get minimum size to accomodate all elements
  ///
  /// @param      exclude  Last element to exclude
  ///
  /// @return     Minimum size
  auto minSize(const Element *exclude=0) const -> Vector;

  ///
  /// Find the visible fragment at a position
  ///
  /// @param[in]  position  The position
  ///
  /// @return     The fragment of the element that is visible at the position, nullptr if there is none.
  ///             The pointer is valid until the surface changes.
  [[nodiscard]] auto find(const Vector &position) const -> Fragment *;

  [[nodiscard]] auto position(Element *element, int default_=-1) -> int;

  [[nodiscard]]  auto zorder() const -> const auto & {return zorder_;}

  private:
  void reorder(int source, int destination);

  ///
  /// Create the fragments of the element at a z-order position from scratch
  void cover(long pos);

  ///
  /// Create the fragments again from scratch where they have fallen apart
  ///
  /// Incremental changes join what they can, but the fragments of an element can still multiply over time. An
  /// element with more than twice the fragments it had when they were last created from scratch gets them
  /// created again, which does not change what is visible, only how it is divided.
  ///
  /// @param[in]  from  z-order position of the first element to consider
  /// @param[in]  to    z-order position after the last element to consider
  void tidy(long from, long to);

  ///
  /// Hand regions that are no longer shown by an element to the elements below
  ///
  /// The regions are given to the elements below `above` down to `lowest`, topmost first, each getting what
  /// lies in its area.
  ///
  /// @param      regions  The regions (disjoint rectangles)
  /// @param[in]  above    z-order position of the element that uncovers the regions
  /// @param[in]  lowest   z-order position of the lowest element that is considered
  /// @param      updates  Receives what has to be painted again
  void expose(vector<Rectangle> regions, long above, long lowest, vector<Fragment> &updates);

  ///
  /// Whether an element is the backdrop
  ///
  /// The backdrop is the lowest element. It is the terminal background itself, so the surface does not paint
  /// it.
  ///
  /// @param[in]  element  The element
  ///
  /// @return     True if the element is the backdrop
  [[nodiscard]] auto isBackdrop(const Element *element) const -> bool;

  vector<Element *> zorder_;
  Device *device_{};
};

/// A surface element backed by a rectangular buffer of terminal cells.
///
/// Writes and fills update the element's cell buffer; the surface then composites only the portions that are visible.
struct TextElement: Surface::Element {
  ///
  /// Constructor
  ///
  /// @param[in]  surface     The surface this element is drawn on
  /// @param[in]  area        The element's area on the surface
  /// @param[in]  background  The element's background cell
  /// @param[in]  below       Optional element below which this element is placed
  ///
  TextElement(Surface *surface, const Rectangle &area, const Char &background=Space, TextElement *below=0);

  TextElement() = default;

  TextElement(const TextElement &) = default;

  TextElement(TextElement &&) = default;

  auto operator=(const TextElement &) -> TextElement & = default;

  auto operator=(TextElement &&) -> TextElement & = default;

  ///
  /// Destroy TextElement
  ~TextElement() override;

  /// String representation of TextElement
  explicit operator string() const;

  ///
  /// Write string to element
  ///
  /// @param[in]  position  The position
  /// @param[in]  str       The text
  auto write(const Vector &position, const string_view &str) -> TextElement &;

  ///
  /// Write text to text element
  ///
  /// @param[in]  position  The position
  /// @param[in]  txt_      The text
  auto write(const Vector &position, const Text &txt_) -> TextElement &;

  ///
  /// Fill text element with Char
  ///
  /// @param[in]  fillChar  The fill character
  /// @param[in]  area      The area
  auto fill(const Char &fillChar=Space, const Rectangle &area=RectangleMax) -> TextElement &;

  ///
  /// Draw line
  ///
  /// @param[in]  line            Line
  /// @param[in]  strength        Line strength
  /// @param[in]  dash            Dash mode
  /// @param[in]  roundedCorners  Whether to round corners
  ///
  /// @return     Rectangle
  auto line(const Line &line, u1 strength=1, u1 dash=0, bool roundedCorners=false) -> TextElement &;

  ///
  /// Draw box
  ///
  /// @param[in]  box   The box
  ///
  /// @return     vector of rectangles
  auto box(const Box &box = Box{}) -> TextElement &;

  auto moveEvent(const Rectangle &/*area*/) -> bool override;

  ///
  /// Move above an element, or one up if there is none
  ///
  /// @return     False if there was nothing to do because the element is on top already
  auto above(TextElement *element=0) -> bool;

  ///
  /// Move below an element, or one down if there is none
  ///
  /// @return     False if there was nothing to do because the element is at the bottom already
  auto below(TextElement *element=0) -> bool;

  ///
  /// Move above the element at a z-order position (negative: counted from the top)
  ///
  /// @throws     std::out_of_range if there is no such position
  auto above(int position=0) -> bool;

  ///
  /// Move below the element at a z-order position (negative: counted from the top)
  ///
  /// @throws     std::out_of_range if there is no such position
  auto below(int position=-1) -> bool;

  [[nodiscard]]  auto position() const {return position_;}

  void position(const Vector &position);

  [[nodiscard]]  auto background() const {return background_;}

  ///
  /// Get text element area
  ///
  /// @return     TextElement area
  [[nodiscard]] auto area() const -> Rectangle override;

  ///
  /// Get area of text element text
  ///
  /// @param[in]  area  The area
  ///
  /// @return     text
  [[nodiscard]] auto text(const Rectangle &area = RectangleMax) const -> Text override;

  private:
  Vector position_;
  Char background_;
  Text text_;
};

} //namespace jwezel
