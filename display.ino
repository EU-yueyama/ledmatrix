#include "display.h"

namespace display {
  // stores pin port and shift
  void get_pin_port(byte pin, byte *port, byte *mask) {
    // port D: 0-7
    if (pin < 8) {
      *port = 0;
      *mask = 1 << pin;
    }
    // port B: 8-13
    else if (pin < 14) {
      *port = 1;
      *mask = 1 << (pin - 8);
    }
    // port C: 14-19
    else if (pin < 20) {
      *port = 2;
      *mask = 1 << (pin - 14);
    }
  }
  
  void set_pins(LEDMatrix* lm, byte* row_pins, byte* col_pins) {
    byte port, mask;
    for (byte i = 0; i < 8; i++) {
      // row i
      get_pin_port(row_pins[i], &port, &mask);
      lm->row_port[i] = port;
      lm->row_mask[i] = mask;
      // col i
      get_pin_port(col_pins[i], &port, &mask);
      lm->col_port[i] = port;
      lm->col_mask[i] = mask;
    }
  }

  void setup(LEDMatrix* lm) {
    volatile uint8_t *ddr[] = {&DDRD, &DDRB, &DDRC};
    // set each row to OUTPUT mode
    for (int r = 0; r < 8; r++) {
      *ddr[lm->row_port[r]] |= lm->row_mask[r];
    }
  }

  void clear(Display* disp) {
    for (byte c = 0; c < 8; c++) {
      for (byte r = 0; r < 8; r++) {
        disp->buf[c][r] = 0;
      }
    }
  }

  // draws display from array of bytes that code a whole column
  // n = number of columns; c0,r0 = offset; v = value
  void draw_bits(Display* disp, byte* bits, byte n, byte v, byte c0, byte r0) {
    for (byte c = 0; c < n && c < 8-c0; c++) {
      // decode byte at column
      for (byte r = 0; r < 8-r0; r++) {
        disp->buf[c+c0][r+r0] = (bits[c] & (1 << r)) ? v : 0;
      }
    }
  }

  void paint(LEDMatrix* lm, Display* disp) {
    volatile uint8_t *ddr[] = {&DDRD, &DDRB, &DDRC};
    volatile uint8_t *port[] = {&PORTD, &PORTB, &PORTC};
    for (byte c = 0; c < 8; c++) {
      // set appropriate row pins to HIGH
      for (byte r = 0; r < 8; r++) {
        // paint (c,r) every buf[c][r] cycles
        if (disp->buf[c][r] && disp->cycle % disp->buf[c][r] == 0) {
          *port[lm->row_port[r]] |= lm->row_mask[r];
        } else {
          *port[lm->row_port[r]] &= ~(lm->row_mask[r]);
        }
      }

      // set (only) drawing column to OUTPUT then INPUT mode.
      *ddr[lm->col_port[c]] |= lm->col_mask[c];
      delayMicroseconds(50); // delay improves visual brightness
      *ddr[lm->col_port[c]] &= ~(lm->col_mask[c]);
    }
    // completed one display
    disp->cycle ++;
  }
}
