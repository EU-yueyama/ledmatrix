#include<avr/io.h>

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
void paint_col(short col, byte data) {
  // DDR*: 0 = input, 1 = output
  // PORT*: 0 = high impedence/low
  byte ddrd, ddrb, ddrc, portd, portb, portc;

  // set all row pins to output mode
  ddrd = B10011100;
  ddrb = B00000010;
  ddrc = B00010011;
  // reset vcc and gnd
  portd = 0;
  portb = 0;
  portc = 0;
  
  // set specified col to output so that it can receive current
  switch (col) {
  case 0: ddrb |= B00000001; break;
  case 1: ddrc |= B00000100; break;
  case 2: ddrc |= B00001000; break;
  case 3: ddrd |= B00100000; break;
  case 4: ddrc |= B00100000; break;
  case 5: ddrd |= B01000000; break;
  case 6: ddrb |= B00000100; break;
  case 7: ddrb |= B00001000; break;
  default: return; // shouldn't happen
  }

  // paint columns
  if (data & B00000001) { portd |= B00010000; }
  if (data & B00000010) { portb |= B00000010; }
  if (data & B00000100) { portd |= B00001000; }
  if (data & B00001000) { portd |= B10000000; }
  if (data & B00010000) { portc |= B00000001; }
  if (data & B00100000) { portd |= B00000100; }
  if (data & B01000000) { portc |= B00000010; }
  if (data & B10000000) { portc |= B00010000; }

  // update avr ports
  DDRD = ddrd;
  DDRB = ddrb;
  DDRC = ddrc;
  PORTD = portd;
  PORTB = portb;
  PORTC = portc;
}

// display: a global byte array
byte display[8];
void paint_display(int ms) {
  unsigned long t = millis() + ms;
  while (millis() < t) {
    for (int i = 0; i < 8; i++) {
      paint_col(i, display[i]);
      delay(2);
    }
  }
}

void setup() {
  // start with everything as high impedence input (for fun)
  DDRD = B00000000;
  DDRB = B00000000;
  DDRC = B00000000;
  PORTD = B00000000;
  PORTB = B00000000;
  PORTC = B00000000;
}

void loop() {
}
