#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>

class Event {
 public:
  Event();
  ~Event();

  virtual std::string toString() = 0;

  int getId();

 protected:
  static int globalId;

  int id;

  time_t tTime;
};

#endif