namespace okmatrix {
  const byte ok_len = 17;
  const byte ok[] = {
    // O
    B01111110,
    B11111111,
    B11000011,
    B10000001,
    B10000001,
    B11000011,
    B11111111,
    B01111110,
    // K
    B00000000,
    B11111111,
    B11111111,
    B00010000,
    B00111000,
    B01101100,
    B11000110,
    B10000011,
    // padding
    B00000000
  };

  void alternate_ok() {
    // O
    memcpy(display, ok, 8);
    paint_display(1000);

    // K
    memcpy(display, ok+8, 8);
    paint_display(1000);

    // O invert
    for (byte i = 0; i < 8; i++) {
      display[i] = ~ok[i];
    }
    paint_display(1000);
  
    // K invert (offset 8)
    for (byte i = 0; i < 8; i++) {
      display[i] = ~ok[8+i];
    }
    paint_display(1000);
  }

  void scroll_ok() {
    for (byte i = 0; i < ok_len; i++) {
      // copy relevant O
      if (ok_len-i < 8) {
        byte o = ok_len-i; // number of bytes that can be copied from tail
        memcpy(display, ok+i, o); // copy o tail bytes
        memcpy(display+o, ok, 8-o); // copy 8-o head bytes
      } else {
        memcpy(display, ok+i, 8);
      }
      paint_display(200);
    }
  }

}
