#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
const int rs = 39, en = 40, d4 = 31, d5 = 11, d6 = 12, d7 = 13 ;
//        PF3       PF2      PB7     PF0    PE0   PB2
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const byte ROWS = 4; 
const byte COLS = 4; 
char tecla;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'F'},
  {'4', '5', '6', 'E'},
  {'7', '8', '9', 'D'},
  {'A', '0', 'B', 'C'}
};

const int rowPins[ROWS] = {34, 33, 32, 19}; 
const int colPins[COLS] = {38, 37, 36, 35}; 
const int Y1 = 34;

void setup(){
    // set up the LCD's number of columns and rows: 
  lcd.begin(20, 1);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  pinMode(Y1,OUTPUT);
  pinMode(33,OUTPUT);
  pinMode(32,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(38,INPUT);
  pinMode(37,INPUT);
  pinMode(36,INPUT);
  pinMode(35,INPUT);
  Serial.begin(9600);
}
  
void loop(){
  for(int i = 0; i < 4; i++)    //Scan columns loop
    {
      digitalWrite(rowPins[i], HIGH);
      delay(200);
      for(int j = 0; j < 4; j++)  //Scan rows
      {
        if(colPins[j] == HIGH)
          tecla = hexaKeys[j][i];
      }
      digitalWrite(rowPins[i],LOW);
    }
      // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
 lcd.setCursor(0, 0);
 // print the number of seconds since reset:
 lcd.print(tecla);
  Serial.println(tecla);
}
