#include "fishing.h"

namespace fishing {

  enum Phase {
    CAST,
    FISH_APPROACH,
    REEL_IN,
    CATCH,
  };

  typedef struct State {
    Phase phase = Phase::CAST;
    byte value = 0; // progress in phase
  };

  byte fish[] = {
    B010,
    B111,
    B111,
    B111,
    B010,
    B111
  };

  void tick_cast(State* s, display::Display* disp) {
    s->value ++;
    // animate hook going down
    if (s->value < 5) {
      display::clear(disp);
      byte line = (1 << s->value + 1) - 1;
      display::draw_bits(disp, &line, 1, 2, 0, 0);
    }
    // randomly transition to FISH_APPROACH
    else if (random(s->value-5) > 1) {
      s->phase = Phase::FISH_APPROACH;
      s->value = 0;
    }
  }
  void tick_fish_approach(State *s, display::Display* disp) {
    s->value ++;
    display::clear(disp);
    byte line = B11111; display::draw_bits(disp, &line, 1, 2, 0, 0);
    display::draw_bits(disp, fish, min(6, s->value), 32, 8-s->value, 3);
    if (s->value >= 7) {
      s->phase = Phase::REEL_IN;
      s->value = 0;
    }
  }
  void tick_reel(State* s, display::Display* disp) {
    // check reeling direction
    // value = line tension
    s->value++;
    if (s->value >= random(20)) {
      s->phase = Phase::CAST;
      s->value = 0;
    }
  }
  void tick_catch(State* s, display::Display* disp) {
    
  }
  
  void tick(State* s, display::Display* disp) {
    switch (s->phase) {
    case CAST:
      return tick_cast(s, disp);
    case FISH_APPROACH:
      return tick_fish_approach(s, disp);
    case REEL_IN:
      return tick_reel(s, disp);
    case CATCH:
      return tick_catch(s, disp);
    }
  };

  void loop(display::LEDMatrix* lm, display::Display* disp) {
    State state;
    unsigned long t = 0;
    while (true) {
      if (millis() >= t) {
        t = millis() + 500;
        tick(&state, disp);
      }
      display::paint(lm, disp);
    }
  }
}
