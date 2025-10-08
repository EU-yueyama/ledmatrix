namespace snake {
  enum Direction {
    U = 0,
    R = 1,
    D = 2,
    L = 3
  };

  typedef struct State {
    byte score;
    Direction head_d;
    byte head_r, head_c;
    byte segments[8][8];
    byte target_r, target_c;
  };

  void place_target(State *s) {
    s->target_r = random(8);
    s->target_c = random(8);
  }
  
  boolean tick(State *s) {
    // read input
    int read = analogRead(A7);
    // add +4 before modulo to avoid negative numbers
    if (read < 400) {
      s->head_d = (Direction)((s->head_d + 5) % 4);
    } else if (read > 600) {
      s->head_d = (Direction)((s->head_d + 3) % 4);
    }

    // move s->head
    switch(s->head_d) {
    case U: s->head_r --; break;
    case D: s->head_r ++; break;
    case L: s->head_c --; break;
    case R: s->head_c ++; break;
    }
    
    // check collisions
    // since byte is unsigned, assume overflow = collision
    if (s->head_r >= 8 || s->head_c >= 8 ||
        s->segments[s->head_r][s->head_c] > 0) {
      return false;
    }
    // check s->target
    if (s->head_r == s->target_r && s->head_c == s->target_c) {
      s->score ++;
      place_target(s);
    }

    // place new segment at s->head
    s->segments[s->head_r][s->head_c] = s->score + 3;
    
    // update s->segments and display
    for (byte c = 0; c < 8; c++) {
      // clear display for column
      display[c] = 0;
      for (byte r = 0; r < 8; r++) {
        // set display pixel if segment active
        if (s->segments[r][c] > 0) {
          display[c] |= 1 << r;
          // decay segment after display
          s->segments[r][c] --;
        }
      }
    }
    // display target
    display[s->target_c] |= 1 << s->target_r;

    return true;
  }

  void setup() {
    pinMode(A7, INPUT);
  }
  void loop() {
    State state = {};
    state.head_d = R;
    place_target(&state);
    while (tick(&state)) {
      paint_display(300);
    }
  }
}
