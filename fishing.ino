#include "fishing.h"

namespace fishing {

  enum Phase {
    CAST,
    REEL_IN,
    CATCH,
  };

  typedef struct State {
    Phase phase = Phase::CAST;
    int next_action = 0; // time left in phase
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
    // animate hook going down
    if (s->value < 5) {
      s->value ++;
    }
    byte line = (1 << s->value + 1) - 1;
    display::draw_bits(disp, &line, 1, 2, 0, 0);
  }
  void tick_reel(State* s, display::Display* disp) {
    // check reeling direction
    // value = line tension
    byte line = (1 << 6) - 1;
    display::draw_bits(disp, &line, 1, 2, 0, 0);
    display::draw_bits(disp, fish, 6, 1, 1, 3);
  }
  void tick_catch(State* s, display::Display* disp) {
    
  }

  void new_phase(State* s) {
    switch (s->phase) {
    case CAST: // cast -> reel
      s->phase = REEL_IN;
      s->next_action = random(2,10);
      break;
    case REEL_IN: // reel -> reel OR result
      s->phase = CATCH;
      s->next_action = 20;
      break;
    case CATCH:
      s->phase = CAST;
      s->next_action = 10;
      break;
    }
  }
  
  void tick(State* s, display::Display* disp) {
    if (s->next_action == 0) {
      new_phase(s);
    }
    s->next_action --;
    switch (s->phase) {
    case CAST:
      return tick_cast(s, disp);
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
