#include "LCD_lib.h"
#include <Wire.h>
#include "BH1750_lib.h"
#define BTN_1 6
#define BTN_2 8
#define BTN_3 9
#define BTN_4 10
#define LUX A5
#define WARNING_LED 7
class click
{
 public:

 click(int pin):_pin(pin)
 {
 lastState = false;
 thisState = false;
 counter = 4;
 }
void clickerCounter()
 {
 thisState = digitalRead(_pin);
 if (thisState && !lastState)
 {
 counter++;
 }
 lastState = thisState;
 }

 int getCount()
 {
 return counter;
 }

 private:
 int counter;
 bool lastState;
 bool thisState;
 int _pin;
};
char getRoom(int type)
{
 char c;
 if (type % 5 == 0)
 {
 c = 'A';
 } else if (type % 5 == 1)
 {
 c = 'B';
 } if (type % 5 == 2)
 {
 c = 'C';
 } else if (type % 5 == 3)
 {
 c = 'D';
 } else if (type % 5 == 4)
 {
 c = 'E';
 }
 return c;
}
int measurementDelay(int thisDelay)
{
 int del;
 if (thisDelay < 0)
 {
 del = 60 + thisDelay;
 } else
 {
 del = thisDelay;
 }
 return del;
}
void lowLightnessWarning(int pin, char room, int res)
{
 int limit = 300;
 switch (room)

{
 case 'A':
 limit = 300;
 break;
 case 'B':
 limit = 500;
 break;
 case 'C':
 limit = 200;
 break;
 case 'D':
 limit = 50;
 break;
 case 'E':
 limit = 75;
 break;
 }
 if (res < limit)
{
 digitalWrite(pin, HIGH);

}
 else

{
 digitalWrite(pin, LOW);

}
}
BH1750_lib lightMeter(0x23);
LCD_lib lcd_1(12, 11, 5, 4, 3, 2);
click typeOfRoom(BTN_1);
click backMode(BTN_2);
click forwardMode(BTN_3);
click nowMeasure(BTN_4);
void setup()
{
 pinMode(BTN_1, INPUT_PULLUP);
 pinMode(BTN_2, INPUT_PULLUP);
 pinMode(BTN_3, INPUT_PULLUP);
 pinMode(BTN_4, INPUT_PULLUP);
 pinMode(LUX, INPUT);
 pinMode(WARNING_LED, OUTPUT);
 lcd_1.begin(16, 2);
 Wire.begin();
}
long timer = millis();
int numberOfClicks = nowMeasure.getCount();
void loop()
{

 int lux = lightMeter.read();
 typeOfRoom.clickerCounter();
 backMode.clickerCounter();
 forwardMode.clickerCounter();
 nowMeasure.clickerCounter();
 char thisRoom = getRoom(typeOfRoom.getCount());
 int thisMode = forwardMode.getCount() - backMode.getCount();
 byte del = measurementDelay(thisMode);
 lcd_1.setCursor(0, 0);
 lcd_1.print("LUX: ");
 if(millis() - timer > del * 1000 || nowMeasure.getCount() > numberOfClicks){
 lcd_1.setCursor(5, 0);
 lcd_1.print(" ");
 lcd_1.setCursor(5, 0);
 lcd_1.print(lux);
 timer = millis();
 numberOfClicks = nowMeasure.getCount();
 }
 lowLightnessWarning(WARNING_LED, thisRoom, lux);

 lcd_1.setCursor(10, 0);
 lcd_1.print("ROOM ");
lcd_1.setCursor(15, 0);
 lcd_1.print(thisRoom);
 lcd_1.setCursor(0, 1);
 lcd_1.print("Delay(s): ");
 lcd_1.setCursor(11, 1);
 lcd_1.print(" ");
 lcd_1.setCursor(11, 1);
 lcd_1.print(measurementDelay(thisMode));

}