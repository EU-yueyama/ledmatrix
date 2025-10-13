#ifndef DISPLAY_H
#define DISPLAY_H

namespace display {
  typedef struct Display {
    byte buf[8][8];
    byte cycle;
  };
  typedef struct LEDMatrix {
    byte row_port[8]; // pin's port number
    byte row_mask[8]; // pin's mask for its port
    byte col_port[8];
    byte col_mask[8];
  };
  void set_pins(LEDMatrix* lm, byte* row_pins, byte* col_pins);
  void setup(LEDMatrix* lm);
  void clear(Display* disp);
  void draw_bits(Display* disp, byte* bits, byte n, byte v, byte c0, byte r0);
  void paint(LEDMatrix* lm, Display* disp);
}

#endif
