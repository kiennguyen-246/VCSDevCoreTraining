#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>

// Abstract class for events
class Event {
 public:
  /**
   * Default constructor.
   */
  Event();

  /**
   * Default destructor.
   */
  ~Event();

  /**
   * Abstract function to describe the event.
   */
  virtual std::string toString() = 0;

  /**
   * Get the accounted ID for the event.
   */
  int getId();

 protected:
  // Global accounted event ID
  static int globalId;

  // Local ID of the event
  int id;

  // Event timestamp
  time_t tTime;
};

#endif