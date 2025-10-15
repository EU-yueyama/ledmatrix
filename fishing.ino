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

  enum Fish {
    NEUT,
    UP,
    DOWN,
  };
  void draw_fish(State *s, display::Display* d, Fish f, char c0, char r0) {
    byte* fish; byte n, v, r1;
    switch (f) {
    case NEUT:
      fish = fish_neut;
      n = 6;
      r1 = 3;
      break;
    case UP:
      fish = fish_up;
      n = 5;
      r1 = 0;
      break;
    case DOWN:
      fish = fish_down;
      n = 5;
      r1 = 3;
      break;
    }
    switch (s->phase) {
    case FISH_APPROACH:
    case LINE_SNAP:
    case FISH_ESCAPE:
      v = 1 << 4; break;
    case REEL_NEUTRAL:
    case REEL_IN:
    case REEL_OUT:
      v = 1 << 2; break;
    case FISH_CAUGHT:
      v = 1 << 0; break;
    }
    display::draw_bits(d, fish, n, v, c0+1, r0+r1+s->fish_v);
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
    draw_fish(s, disp, Fish::NEUT, 7-s->t, 0);
    if (s->t >= 7) {
      s->phase = Phase::REEL_NEUTRAL;
      s->t = 0;
      s->progress = 0;
    }
  }
  void tick_reel(State* s, display::Display* d) {
    char dlv = 0; // track fish+input change to line_v
    // input -> reel direction
    if (analogRead(A7) > 600) {
      // reel in
      char x = map(analogRead(A7), 600, 1024, 1, 4);
      dlv -= x;
      s->progress += x;
    } else if (analogRead(A7) < 400) {
      // reel out
      char x = map(analogRead(A7), 0, 400, 4, 1);
      dlv += x;
    }
    
    // fish movement -> modify line value
    // difficulty -> more likely to move up/down
    bool move_check = random(s->score*s->score + 10) >= 10;
    switch (s->phase) {
    case REEL_NEUTRAL: // neutral -> return to center
      if (s->fish_v > 0) s->fish_v --;
      if (s->fish_v < 0) s->fish_v ++;
      // return to middle line_v during neutral
      if (s->line_v > 16) dlv --;
      if (s->line_v < 16) dlv ++;
      break;
    case REEL_IN:
      if (s->fish_v < 0 || (s->fish_v < 2 && move_check)) s->fish_v ++;
      dlv += max(0, s->fish_v) + 1;
      break;
    case REEL_OUT:
      if (s->fish_v > 0 || (s->fish_v > -2 && move_check)) s->fish_v --;
      dlv += min(0, s->fish_v) - 1; // fish-v is negative
      break;
    }
    
    // constrain line_v range
    dlv = (dlv < 0) ? max(-s->line_v, dlv) : min(32-s->line_v, dlv);
    s->line_v += dlv;

    // draw
    display::clear(d);
    draw_line(s, d);
    switch (s->phase) {
    case REEL_NEUTRAL:
      draw_fish(s, d, Fish::NEUT, 0, 0);
      break;
    case REEL_IN:
      draw_fish(s, d, Fish::UP, 0, 0);
      break;
    case REEL_OUT:
      draw_fish(s, d, Fish::DOWN, 0, 0);
      break;
    }

    // catch progress
    // difficulty -> require more progress for catch
    if (s->progress > 10+s->score*s->score*2) {
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
    // difficulty -> faster to switch
    else if (random(s->t) > 50/(s->score*s->score+10)) {
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
    draw_fish(s, d, Fish::NEUT, 0, 1-s->t);

    if (s->t >= 6) {
      s->phase = Phase::SHOW_SCORE;
      s->score = 0;
      s->t = 0;
    }
  }
  void tick_fish_escape(State *s, display::Display* d) {
    display::clear(d);
    draw_line(s, d);
    draw_fish(s, d, Fish::NEUT, s->t, 0);
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
    draw_fish(s, d, Fish::NEUT, 0, s->t-1);

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
