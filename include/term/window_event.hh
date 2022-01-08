#pragma once

#include "event.hh"

struct WindowEvent: public Event {
  CLASS_ID(WindowEvent);
};

struct WindowAreaChanged: public WindowEvent {
  CLASS_ID(WindowAreaChanged);
};

struct WindowVisibilityChanged: public WindowEvent {
  CLASS_ID(WindowVisibilityChanged);
};

struct WindowFocusChanged: public WindowEvent {
  CLASS_ID(WindowFocusChanged);
};
