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
    byte t = 0; // progress in phase
    byte line_v = 0; // fishing line state
  };

  byte fish[] = {
    B010,
    B111,
    B111,
    B111,
    B010,
    B111
  };

  void draw_line(State* s, display::Display* d) {
    byte line = B11111;
    display::draw_bits(d, &line, 1, s->line_v, 0, 0);
  }

  void tick_cast(State* s, display::Display* disp) {
    // animate hook going down
    if (s->t < 5) {
      display::clear(disp);
      byte line = (1 << s->t + 1) - 1;
      display::draw_bits(disp, &line, 1, 2, 0, 0);
    }
    // randomly transition to FISH_APPROACH
    else if (random(s->t-5) > 1) {
      s->phase = Phase::FISH_APPROACH;
      s->t = 0;
      s->line_v = 12;
    }
  }
  void tick_fish_approach(State *s, display::Display* disp) {
    display::clear(disp);
    draw_line(s, disp);
    display::draw_bits(disp, fish, min(6, s->t), 32, 8-s->t, 3);
    if (s->t >= 7) {
      s->phase = Phase::REEL_IN;
      s->t = 0;
    }
  }
  void tick_reel(State* s, display::Display* d) {
    // input -> reel direction
    if (analogRead(A7) > 600) {
      s->line_v --; // reel line in
    } else if (analogRead(A7) < 400) {
      s->line_v ++;
    }

    // line logic
    draw_line(s, d);
    
    // maybe change phase
    if (s->t >= random(20)+10) {
      s->phase = Phase::CAST;
      s->t = 0;
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
        s->t ++;
        tick(&state, disp);
      }
      display::paint(lm, disp);
    }
  }
}
