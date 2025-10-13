#include "display.h"

namespace numbers {
  byte number[10][3] = {
    {B11111, B10001, B11111}, // 0
    {B00000, B00000, B11111}, // 1
    {B10111, B10101, B11101}, // 2
    {B10101, B10101, B11111}, // 3
    {B11100, B00100, B11111}, // 4
    {B11101, B10101, B10111}, // 5
    {B11111, B10101, B10111}, // 6
    {B10000, B10000, B11111}, // 7
    {B11111, B10101, B11111}, // 8
    {B11101, B10101, B11111}, // 9
  };

  void set_display(display::Display* disp, byte x) {
    if (x > 99) x = 99; // no support for >2 digits

    display::clear(disp);
    
    // 10s place
    byte tens = (x / 10) % 10;
    byte ones = x % 10;
    display::draw_bits(disp, number[tens], 3, 0, 0, 1);
    display::draw_bits(disp, number[ones], 3, 4, 0, 1<<2);
  }
}
