#include "Event.hpp"

Event::Event() {
  id = ++globalId;
  tTime = time(0);
}

Event::~Event() {}

int Event::getId() { return id; }

int Event::globalId = 0;

