#ifndef TIME_H
#define TIME_H

#include <time.h>

class Time;

class TimeLength {
public:
  double s;

  inline TimeLength()
    : s(0) {
  }

  inline TimeLength(double s)
    : s(s) {
  }

  inline static TimeLength inMilliseconds(double s) {
    return TimeLength(s * 1e-3);
  }

  inline static TimeLength inSeconds(double s) {
    return TimeLength(s);
  }

  inline double seconds() const {
    return s;
  }

  inline bool operator>(const TimeLength& o) {
    return s > o.s;
  }

  inline bool operator<(const TimeLength& o) {
    return s < o.s;
  }
};

class Time {
public:
  timeval t;

  inline Time() {
    gettimeofday(&t, NULL);
  }

  inline Time(const timeval& t)
    : t(t) {
  }

  inline Time operator+(const TimeLength& o) {
    timeval nt = t;

    nt.tv_sec += floor(o.s);
    nt.tv_usec += fmod(o.s, 1) * 1e6;

    while(t.tv_usec < 0) {
      nt.tv_sec -= 1;
      nt.tv_usec += 1e6;
    }

    while(t.tv_usec > 1e6) {
      nt.tv_usec += 1;
      nt.tv_usec -= 1e6;
    }

    return Time(nt);
  }

  inline TimeLength operator-(const Time& o) {
    long sec = t.tv_sec - o.t.tv_sec;
    long usec = t.tv_usec - o.t.tv_usec;
    return TimeLength((double)sec + usec * 1e-6);
  }
};

#endif
