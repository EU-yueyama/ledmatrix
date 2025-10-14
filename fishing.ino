#include "fishing.h"

namespace fishing {

  enum Phase {
    CAST,
    FISH_APPROACH,
    REEL_NEUTRAL,
    REEL_IN,
    REEL_OUT,
    LINE_SNAP,
    CATCH,
  };

  typedef struct State {
    Phase phase = Phase::CAST;
    byte t = 0; // progress in phase
    byte line_v = 0; // fishing line state
  };

  byte fish_neut[] = {
    B011,
    B011,
    B111,
    B011,
    B010,
    B111
  };
  byte fish_up[] = {
    B11000,
    B11100,
    B01100,
    B01011,
    B00010
  };
  byte fish_down[] = {
    B00011,
    B01111,
    B00110,
    B11000,
    B01000
  };

  void draw_line(State* s, display::Display* d) {
    byte line = B11111000;
    display::draw_bits(d, &line, 1, s->line_v, 0, 0);
  }

  void tick_cast(State* s, display::Display* d) {
    // animate hook going down
    if (s->t < 6) {
      display::clear(d);
      byte line = 0xFF << 8 - s->t;
      display::draw_bits(d, &line, 1, 2, 0, 0);
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
    display::draw_bits(disp, fish_neut, 6, 32, 8-s->t, 3);
    if (s->t >= 7) {
      s->phase = Phase::REEL_NEUTRAL;
      s->t = 0;
    }
  }
  void tick_reel(State* s, display::Display* d) {
    // input -> reel direction
    if (analogRead(A7) > 600) {
      s->line_v -= min(s->line_v-1, map(analogRead(A7), 600, 1024, 1, 4));
    } else if (analogRead(A7) < 400) {
      s->line_v += min(16-s->line_v, map(analogRead(A7), 0, 400, 4, 1));
    }

    // fish movement -> modify line value
    switch (s->phase) {
    case REEL_IN:
      s->line_v ++; break;
    case REEL_OUT:
      s->line_v --; break;
    }
    // snap line if value falls too low
    if (s->line_v <= 1) {
      s->phase = Phase::LINE_SNAP;
      s->t = 0;
      return;
    }

    // draw
    display::clear(d);
    draw_line(s, d);
    switch (s->phase) {
    case REEL_NEUTRAL:
      display::draw_bits(d, fish_neut, 6, 8, 1, 3);
      break;
    case REEL_IN:
      display::draw_bits(d, fish_up, 5, 8, 1, 0);
      break;
    case REEL_OUT:
      display::draw_bits(d, fish_down, 5, 8, 1, 3);
      break;
    }
    
    // maybe change phase
    if (s->t >= random(10)+4) {
      // either catch fish, or move to other reel phase
      switch (random(4)) {
      case 0:
        s->phase = Phase::CAST; break;
      case 1:
        s->phase = Phase::REEL_NEUTRAL; break;
      case 2:
        s->phase = Phase::REEL_IN; break;
      case 3:
        s->phase = Phase::REEL_OUT; break;
      }
      s->t = 0;
    }
  }
  void tick_line_snap(State* s, display::Display* d) {
    display::clear(d);
    byte line = B11000000 | (B01110000 >> s->t);
    display::draw_bits(d, &line, 1, 8, 0, 0);
    display::draw_bits(d, fish_neut, 6, 32, 1, 3+1-s->t);

    if (s->t >= 6) {
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
    case REEL_NEUTRAL:
    case REEL_IN:
    case REEL_OUT:
      return tick_reel(s, disp);
    case LINE_SNAP:
      return tick_line_snap(s, disp);
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
        state.t ++;
        tick(&state, disp);
      }
      display::paint(lm, disp);
    }
  }
}
