#ifndef CMMC_NETPIE_INTERVAL_H
#define CMMC_NETPIE_INTERVAL_H
#include <Arduino.h>
#include <functional>

class CMMC_Interval
{
  private:
    unsigned long _prev_ms;
    unsigned long _now_ms;

    unsigned long _prev_us;
    unsigned long _now_us;


  public:
    typedef std::function<void(void)> void_callback_t;

    CMMC_Interval() {
      _prev_ms = millis();
      _now_ms  = millis();

      _prev_us = micros();
      _now_us  = micros();
    };

    ~CMMC_Interval() {
    };

    void every_ms(unsigned long ms, void_callback_t cb) {
      _now_ms  = millis();
      unsigned long diff = _now_ms - _prev_ms;
      if (diff >= ms) {
        _prev_ms = millis();
        cb();
      }
    }

    void every_us(unsigned long us, void_callback_t cb) {
      _now_us  = micros();
      unsigned long diff = _now_us - _prev_us;
      if (diff >= us) {
        _prev_us = micros();
        cb();
      }
    }
};


#endif
