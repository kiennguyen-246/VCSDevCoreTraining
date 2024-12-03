#include "Event.hpp"

Event::Event() { id = ++globalId; }

Event::~Event() {}

int Event::getId() { return id; }

int Event::globalId = 0;

