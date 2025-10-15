#include "numbers.h"
#include "fishing.h"

namespace fishing {

  enum Phase {
    CAST,
    FISH_APPROACH,
    REEL_NEUTRAL,
    REEL_IN,
    REEL_OUT,
    LINE_SNAP,
    FISH_ESCAPE,
    FISH_CAUGHT,
    SHOW_SCORE,
  };

  typedef struct State {
    Phase phase = Phase::SHOW_SCORE;
    byte t = 0; // progress in phase
    byte line_v = 0; // fishing line state
    byte progress = 0; // how close to catch?
    char fish_v = 0; // how much the fish changes line tension (-2 ~ +2)
    byte score = 0;
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

  void draw_line_m(State* s, display::Display* d, byte line) {
    // line_v <= 32, but we still want ->4 even before 32->
    byte v = 1 << map(s->line_v, 0, 33, 0, 5);
    byte shifted;
    if (s->fish_v < 0) {
      shifted = line >> -s->fish_v;
      // extend line to top of screen (if MSB==1)
      if (line & 0x80) {
        shifted |= 0xFF << 8+s->fish_v;
      }
    } else {
      shifted = line << s->fish_v;
    }
    display::draw_bits(d, &shifted, 1, v, 0, 0);
  }
  void draw_line(State *s, display::Display* d) {
    draw_line_m(s,d,B11111000);
  }

  void tick_cast(State* s, display::Display* d) {
    // animate hook going down
    if (s->t < 6) {
      display::clear(d);
      draw_line_m(s,d,0xFF << 8 - s->t);
    }
    // randomly transition to FISH_APPROACH
    else if (random(s->t-5) > 1) {
      s->phase = Phase::FISH_APPROACH;
      s->t = 0;
    }
  }
  void tick_fish_approach(State *s, display::Display* disp) {
    display::clear(disp);
    draw_line(s, disp);
    display::draw_bits(disp, fish_neut, 6, 1<<4, 8-s->t, 3);
    if (s->t >= 7) {
      s->phase = Phase::REEL_NEUTRAL;
      s->t = 0;
      s->progress = 0;
    }
  }
  void tick_reel(State* s, display::Display* d) {
    // fish movement -> modify line value
    switch (s->phase) {
    case REEL_NEUTRAL: // neutral -> return to center
      if (s->fish_v > 0) s->fish_v --;
      if (s->fish_v < 0) s->fish_v ++;
      break;
    case REEL_IN:
      if (s->fish_v < 0 || random(3-s->fish_v) > 0) s->fish_v ++;
      s->line_v += 1 + s->fish_v;
      break;
    case REEL_OUT:
      if (s->fish_v > 0 || random(3+s->fish_v) > 0) s->fish_v --;
      s->line_v -= 1 + s->fish_v;
      break;
    }

    // input -> reel direction
    if (analogRead(A7) > 600) {
      // reel in
      byte dlv = min(s->line_v, map(analogRead(A7), 600, 1024, 1, 3));
      s->line_v -= dlv;
      s->progress += dlv + dlv; // 2 steps forward
    } else if (analogRead(A7) < 400) {
      // reel out
      byte dlv = min(32-s->line_v, map(analogRead(A7), 0, 400, 3, 1));
      s->line_v += dlv;
      s->progress -= min(s->progress, dlv); // 1 step back
    }

    // draw
    display::clear(d);
    draw_line(s, d);
    switch (s->phase) {
    case REEL_NEUTRAL:
      display::draw_bits(d, fish_neut, 6, 1<<2, 1, 3+s->fish_v);
      break;
    case REEL_IN:
      display::draw_bits(d, fish_up, 5, 1<<2, 1, 0+s->fish_v);
      break;
    case REEL_OUT:
      display::draw_bits(d, fish_down, 5, 1<<2, 1, 3+s->fish_v);
      break;
    }

    // catch progress
    if (random(s->progress) > 20) {
      s->phase = Phase::FISH_CAUGHT;
      s->t = 0;
    }
    // snap line if line is too tight
    else if (s->line_v <= 0) {
      s->line_v = 0;
      s->phase = Phase::LINE_SNAP;
      s->t = 0;
    }
    // fish escapes if line is too loose
    else if (s->line_v >= 32) {
      s->line_v = 32;
      s->phase = Phase::FISH_ESCAPE;
      s->t = 0;
    }
    // maybe change reeling phase
    else if (s->t >= random(10)+4) {
      // either catch fish, or move to other reel phase
      switch (random(1,4)) {
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
    draw_line_m(s,d, B11000000 | (B01110000 >> s->t));
    display::draw_bits(d, fish_neut, 6, 1<<4, 1, 3+1-s->t+s->fish_v);

    if (s->t >= 6) {
      s->phase = Phase::SHOW_SCORE;
      s->score = 0;
      s->t = 0;
    }
  }
  void tick_fish_escape(State *s, display::Display* d) {
    display::clear(d);
    draw_line(s, d);
    display::draw_bits(d, fish_neut, 6, 1<<4, s->t, 3+s->fish_v);
    if (s->t >= 7) {
      s->phase = Phase::SHOW_SCORE;
      s->score = 0;
      s->t = 0;
    }
  }
  // animate fish being reeled up out of screen
  void tick_fish_caught(State* s, display::Display* d) {
    display::clear(d);
    draw_line_m(s,d, B01111100 << s->t);
    display::draw_bits(d, fish_neut, 6, 1<<0, 1, 3-1+s->t+s->fish_v);

    if (s->t >= 6) {
      s->phase = Phase::SHOW_SCORE;
      s->score ++;
      s->t = 0;
    }
  }

  void tick_show_score(State* s, display::Display* d) {
    if (s->t == 1) {
      // display score
      numbers::set_display(d, s->score);
    }
    else if (s->t >= 10) {
      // initialise game
      s->phase = Phase::CAST;
      s->t = 0;
      s->line_v = 16;
      s->fish_v = 0;
    }
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
    case FISH_ESCAPE:
      return tick_fish_escape(s, disp);
    case FISH_CAUGHT:
      return tick_fish_caught(s, disp);
    case SHOW_SCORE:
      return tick_show_score(s, disp);
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
