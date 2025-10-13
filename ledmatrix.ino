#include<avr/io.h>
#include "display.h"
#include "numbers.h"

/*
  matrix pin connections:
  1..6: A0..A5
  7..16: D2..D11

  row (vcc): D4 D9 D3 D7 A0 D2 A1 A4
  col (gnd): D8 A2 A3 D5 A5 D6 D10 D11

  D (D0..D7): - - r6 r3 r1 c4 c6 r4
  B (D8..D13): c1 r2 c7 c8 - -
  C (A0..A5): r5 r7 c2 c3 r8 c5
*/

display::LEDMatrix lm;
display::Display disp;

void setup() {
  // start with everything as high impedence input (for fun)
  DDRD = B00000000;
  DDRB = B00000000;
  DDRC = B00000000;
  PORTD = B00000000;
  PORTB = B00000000;
  PORTC = B00000000;

  byte col_pins[] = {A4, A1, 2, A0, 7, 3, 9, 4}; // reversed to be left→right
  byte row_pins[] = {8, A2, A3, 5, A5, 6, 10, 11};
  display::set_pins(&lm, col_pins, row_pins);
  display::setup(&lm);
}

void loop() {
}
