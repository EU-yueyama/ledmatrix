namespace runner {
  typedef struct State {
    byte obst_top = 0;
    byte obst_bot = 8;
    byte obst_col = 8;

    byte player_r = 3;

    byte score = -1;
  };
  bool tick(State* s) {
    // game logic
    s->player_r = map(analogRead(A7), 0, 1024, 0, 7);
    s->obst_col ++;

    // check obstacle
    if (s->obst_col >= 8) {
      // check collision
      if (s->player_r < s->obst_top || s->player_r+1 >= s->obst_bot) {
        return false;
      }
      // replace
      const byte gap = 2;
      s->obst_col = 0;
      s->obst_top = random(0,8-gap);
      s->obst_bot = s->obst_top+gap+random(0, 8-gap-s->obst_top);
      s->score ++;
    }

    // render
    display[(s->obst_col + 7) % 8] = 0; // clear previously-painted column
    display[7] = B11 << s->player_r;
    for (byte i = 0; i < s->obst_top; i++) {
      display[s->obst_col] |= 1 << i;
    }
    for (byte i = s->obst_bot; i < 8; i++) {
      display[s->obst_col] |= 1 << i;
    }
    return true;
  }

  void setup() {
    pinMode(A7, INPUT);
  }
  void loop() {
    State state;

    clear_display();

    while(tick(&state)) {
      paint_display(200);
    }

    numbers::set_display(state.score);
    paint_display(2000);
  }
}
