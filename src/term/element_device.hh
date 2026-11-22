#pragma once

#include "device.hh"
#include "surface.hh"
#include "text.hh"

namespace jwezel {

///
/// A surface element that is also a device
///
/// The elements on the device's own surface are drawn into it, and the device's own drawing is below
/// them: whatever they do not cover shows, also when they move away again. Everything the content
/// elements draw is clipped to the device's area, because it ends up in the device's own buffer.
///
/// Updates are passed on to the surface the device is on, so a window can show its widgets and a
/// widget tree can live inside a window without knowing about the terminal.
///
/// The device's own drawing is what its `TextElement` methods write. `text()` returns what the device
/// shows, which is its own drawing with the content elements on top of it.
struct ElementDevice: TextElement, Device {
  ///
  /// Create an element device
  ///
  /// @param[in]  surface     The surface the device is on
  /// @param[in]  area        The device's area on that surface
  /// @param[in]  background  The background of the device and of its content
  /// @param[in]  below       Optional element below which the device is placed
  ElementDevice(Surface *surface, const Rectangle &area, const Char &background=Space, TextElement *below=0);

  ElementDevice(const ElementDevice &) = delete;

  ElementDevice(ElementDevice &&) = delete;

  auto operator=(const ElementDevice &) -> ElementDevice & = delete;

  auto operator=(ElementDevice &&) -> ElementDevice & = delete;

  ~ElementDevice() override = default;

  ///
  /// Get the surface the content elements are on
  ///
  /// The coordinates of the elements are relative to the device, as is the device's own drawing.
  [[nodiscard]] auto content() -> Surface & {return content_;}

  ///
  /// Get the device's own drawing, without the content elements
  ///
  /// @param[in]  area  The area, in the device's coordinates
  [[nodiscard]] auto ownText(const Rectangle &area=RectangleMax) const -> Text;

  /// Draw the updates of the content elements into the device and pass them on
  void update(const Updates &updates) override;

  /// Pass on the changes of the device's own drawing
  void update(const vector<Rectangle> &areas) override;

  /// Pass on one change of the device's own drawing
  void update(const Rectangle &area) override;

  ///
  /// Get what the device shows: its own drawing with the content elements on top
  [[nodiscard]] auto text(const Rectangle &area=RectangleMax) const -> Text override;

  auto moveEvent(const Rectangle &area) -> bool override;

  ///
  /// Whether the owner is what shows at a position on the surface the device is on
  ///
  /// This follows nested devices down to the element that is visible there.
  [[nodiscard]] auto shows(const Vector &position, const Surface::Element *owner) const -> bool override;

  ///
  /// The bottom element of the content surface: it shows the device's own drawing
  struct Underlay: Surface::Element {
    explicit Underlay(Surface *surface, ElementDevice *device):
    Element{surface},
    device_{device}
    {
      surface->addElement(this);
    }

    [[nodiscard]] auto area() const -> Rectangle override {return Rectangle{Vector{0, 0}, VectorMax};}

    [[nodiscard]] auto text(const Rectangle &area) const -> Text override;

    void move(const Rectangle &/*area*/) override {}

    ElementDevice *device_;
  };

  private:
  Surface content_{this};
  Underlay underlay_{&content_, this};
};

} // namespace jwezel
