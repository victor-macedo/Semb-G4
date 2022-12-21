e// Including LCD library
#include<LiquidCrystal.h>

// Setting (RS,E,D4,D5,D6,D7) for particular pin number
const int rs = 39, en = 40, d4 = 31, d5 = 11, d6 = 12, d7 = 13 ;
//        PF3       PF2      PB7     PF0    PE0   PB2
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Define variable c1,c2,c3,r1,r2,r3,r4 and del
int c1 = 38, c2 = 37, c3 = 36, c4 = 35;
int r1 = 34, r2 = 33, r3 = 32, r4 = 19;
int del = 100;

void setup() {
  lcd.begin(20, 1);
  lcd.setCursor(0, 0);
  lcd.print("Hello !");
  delay(1000);
  lcd.clear();

  /* use input pull-up feature of arduino, so that input c1 to c3 remains HIGH in absence input
    signal
  */
  pinMode(c1, INPUT_PULLUP);
  pinMode(c2, INPUT_PULLUP);
  pinMode(c3, INPUT_PULLUP);
  pinMode(c4, INPUT_PULLUP);

  // Set r1,r2,r3,r4 pins to OUTPUT mode
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(r4, OUTPUT);
}

void loop() {
  // Calling user defined function row1( ), row2( ), row3( ) and row4( ) next to each other
  row4():
  row3();
  row2();
  row1();
  lcd.clear();
}

// Logic to function row1( )
void row1() {
  /* This the same condition we understand in topic 9.2 i.e. one row should be Low and others should be High to distinguish between other rows and to print row 1 data
  */
  digitalWrite(r1, LOW);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);

  /* We know high signal becomes low when it gets a direct path to ground */
  /* Reading state of c1 and checking if it is LOW then print ‘1’ and if c2 is LOW print ‘2’ and if c3 is LOW print ‘3’
  */
  if (digitalRead(c1) == LOW) {
    lcd.print("1"); delay(del);
  }
  else if (digitalRead(c2) == LOW) {
    lcd.print("2"); delay(del);
  }
  else if (digitalRead(c3) == LOW) {
    lcd.print("3"); delay(del);
  }
  else if (digitalRead(c4) == LOW) {
    lcd.print("F"); delay(del);
  }
  /*why do we not set single delay here instead three in if blocks? Because if we give delay in
     if block then delay will executed only if any key has been pressed but if we give delay
     here each time either key pressed or not delay will be executed which slows the
     programming process.
  */
}

/* Similarly as we set logic for row1 we can set logic for other rows.
   Keep row LOW which we want to set logic and others to be HIGH
   But this time we will print ‘4’, ‘5’, ‘6’ on column key pressed instead of ‘1’, ‘2’, ‘3’.
*/
void row2() {
  /* This the same condition we understand in topic 9.2 i.e. one row should be Low and others should be High to distinguish between other rows and to print row 1 data
  */
  digitalWrite(r1, HIGH);
  digitalWrite(r2, LOW);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);

  /* We know high signal becomes low when it gets a direct path to ground */
  /* Reading state of c1 and checking if it is LOW then print ‘1’ and if c2 is LOW print ‘2’ and if c3 is LOW print ‘3’
  */
  if (digitalRead(c1) == LOW) {
    lcd.print("4"); delay(del);
  }
  else if (digitalRead(c2) == LOW) {
    lcd.print("5"); delay(del);
  }
  else if (digitalRead(c3) == LOW) {
    lcd.print("6"); delay(del);
  }
  else if (digitalRead(c4) == LOW) {
    lcd.print("E"); delay(del);
  }
//delay(del);
  /*why do we not set single delay here instead three in if blocks? Because if we give delay in
     if block then delay will executed only if any key has been pressed but if we give delay
     here each time either key pressed or not delay will be executed which slows the
     programming process.
  */
}
/* In this we will print ‘*’, ‘0’, ‘#’ on column key pressed.
*/void row3() {
  /* This the same condition we understand in topic 9.2 i.e. one row should be Low and others should be High to distinguish between other rows and to print row 1 data
  */
 digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, LOW);
  digitalWrite(r4, HIGH);

  /* We know high signal becomes low when it gets a direct path to ground */
  /* Reading state of c1 and checking if it is LOW then print ‘1’ and if c2 is LOW print ‘2’ and if c3 is LOW print ‘3’
  */
  if (digitalRead(c1) == LOW) {
    lcd.print("7"); delay(del);
  }
  else if (digitalRead(c2) == LOW) {
    lcd.print("8"); delay(del);
  }
  else if (digitalRead(c3) == LOW) {
    lcd.print("9"); delay(del);
  }
  else if (digitalRead(c4) == LOW) {
    lcd.print("D"); delay(del);
  }
//delay(del);
  /*why do we not set single delay here instead three in if blocks? Because if we give delay in
     if block then delay will executed only if any key has been pressed but if we give delay
     here each time either key pressed or not delay will be executed which slows the
     programming process.
  */
}
void row4() {
  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, LOW);

  if (digitalRead(c1) == LOW) {
    lcd.print("A"); delay(del);
  }
  else if (digitalRead(c2) == LOW) {
    lcd.print("0"); delay(del);
  }
  else if (digitalRead(c3) == LOW) {
    lcd.print("B"); delay(del);
  }
  else if (digitalRead(c4) == LOW) {
  lcd.print("C"); delay(del);
  }
}
