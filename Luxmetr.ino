#include <stdio.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <Wire.h>

// Пины для работы кнопок.
#define BTN_1 6
#define BTN_2 8
#define BTN_3 9
#define BTN_4 10

// Индикатор низкого освещения. 
#define WARNING_LED 7

// Адресс датчика освещения.
#define BH1750_ADDRESS 0x23

// Режим работы датчика освещения.
#define ONE_TIME_HIGH_RES_MODE 0x20

// Адреса комманд LCD-дисплея.
#define LCD_ENTRYMODESET 0x04
#define LCD_CLEARDISPLAY 0x01
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET 0x20 
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80 
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02 
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00 
#define LCD_DISPLAYON 0x04 
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00 
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00 
#define LCD_8BITMODE 0x10 
#define LCD_4BITMODE 0x00 
#define LCD_2LINE 0x08 
#define LCD_1LINE 0x00 
#define LCD_5x10DOTS 0x04 
#define LCD_5x8DOTS 0x00 

// Класс LCD-Дисплея.
class LCD_lib : public Print 
{
public:
    LCD_lib(uint8_t rs, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
    {
        init(0, rs, enable, d0, d1, d2, d3, d4, d5, d6, d7);
    }

    void init(uint8_t fourbitmode, uint8_t rs, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) 
    {
        _rs_pin = rs;
        _enable_pin = enable;
        _data_pins[0] = d0;
        _data_pins[1] = d1;
        _data_pins[2] = d2;
        _data_pins[3] = d3;
        _data_pins[4] = d4;
        _data_pins[5] = d5;
        _data_pins[6] = d6;
        _data_pins[7] = d7;

        if (fourbitmode)
            _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
        else
            _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
        begin(16, 1);
    }
    
    void begin(uint8_t cols, uint8_t lines, uint8_t dotsize = LCD_5x8DOTS)
    {
        if (lines > 1) 
        {
            _displayfunction |= LCD_2LINE;
        }

        _numlines = lines;
        setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

        if ((dotsize != LCD_5x8DOTS) && (lines == 1)) 
        {
            _displayfunction |= LCD_5x10DOTS;
        }

        pinMode(_rs_pin, OUTPUT);
        
        for (int i = 0; i < ((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i)
        {
            pinMode(_data_pins[i], OUTPUT);
        }

        delayMicroseconds(50000);
        digitalWrite(_rs_pin, LOW);
        digitalWrite(_enable_pin, LOW);

        if (!(_displayfunction & LCD_8BITMODE)) 
        {
            write4bits(0x03);
            delayMicroseconds(4500); 
            write4bits(0x03);
            delayMicroseconds(4500); 
            write4bits(0x03);
            delayMicroseconds(150);
            write4bits(0x02);
        }
        else 
        {
            command(LCD_FUNCTIONSET | _displayfunction);
            delayMicroseconds(4500);
            command(LCD_FUNCTIONSET | _displayfunction);
            delayMicroseconds(150);
            command(LCD_FUNCTIONSET | _displayfunction);
        } 

        command(LCD_FUNCTIONSET | _displayfunction);
        _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
        display();
        clear();
        _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
        command(LCD_ENTRYMODESET | _displaymode);
	  }

    void clear() 
    {
        command(LCD_CLEARDISPLAY); 
        delayMicroseconds(2000);  
    }

    void display() 
    {
      _displaycontrol |= LCD_DISPLAYON;
      command(LCD_DISPLAYCONTROL | _displaycontrol);
    }

    void setRowOffsets(int row0, int row1, int row2, int row3) 
    {
        _row_offsets[0] = row0;
        _row_offsets[1] = row1;
        _row_offsets[2] = row2;
        _row_offsets[3] = row3;
    }
    void setCursor(uint8_t col, uint8_t row)
    {
        const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
        if (row >= max_lines)
        {
            row = max_lines - 1;
        }
        if (row >= _numlines)
        {
            row = _numlines - 1;
        }
        command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
    }
    virtual size_t write(uint8_t value) 
    {
        send(value, HIGH);
        return 1;
    }
    void command(uint8_t value) 
    {
        send(value, LOW);
    }
  
    using Print::write;
private:
    void send(uint8_t value, uint8_t mode)
    {
        digitalWrite(_rs_pin, mode);

        if (_displayfunction & LCD_8BITMODE)
        {
            write8bits(value);
        }
        else
        {
            write4bits(value >> 4);
            write4bits(value);
        }
    }
    void write4bits(uint8_t value)
    {
        for(int i = 0; i < 4; i++) {
            digitalWrite(_data_pins[i], (value >> i) & 0x01);
        }
        pulseEnable();
    }
    void write8bits(uint8_t value)
    {
        for (int i = 0; i < 8; i++) 
        {
            digitalWrite(_data_pins[i], (value >> i) & 0x01);
        }
        pulseEnable();
    }
    void pulseEnable() 
    {
        digitalWrite(_enable_pin, LOW);
        delayMicroseconds(1);
        digitalWrite(_enable_pin, HIGH);
        delayMicroseconds(1);    
        digitalWrite(_enable_pin, LOW);
        delayMicroseconds(100);  
    }

    uint8_t _rs_pin;
    uint8_t _enable_pin;
    uint8_t _data_pins[8];
    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;
    uint8_t _initialized;
    uint8_t _numlines;
    uint8_t _row_offsets[4];
};

// Класс датчика освещения.
class BH1750_lib 
{
public:
  BH1750_lib(int add):address(add) 
  {
    Wire.beginTransmission(add);
    Wire.write(0x10);
    Wire.endTransmission();
  }
  int read() 
  {
    int value = 0;
    int i=0;
    Wire.beginTransmission(address);
    Wire.requestFrom(address, 2);
    while(Wire.available()) 
    {
      buff[i] = Wire.read();
      i++;
    }
    Wire.endTransmission();
    if(2==i)
    {
      value=((buff[0]<<8)|buff[1])/meaurmentConst;
    }
    return value;
  } 
 
private:
  const int mode = 0x10;
	
  // Константа для расчета значения.
  const float meaurmentConst = 1.2;
  int address;
  byte buff[2];

};

// Класс кнопки (Предназначен для расчёта количества нажатий).
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

// Функция для определения типа комнаты.
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

// Функция для установки задержки.
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

// Функция для оповещения о низком уровне освещения.
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

BH1750_lib lightMeter(BH1750_ADDRESS);
LCD_lib lcd_1(12, 11,
		 0, 0, 0, 0,
		 5, 4, 3, 2);
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
 // Выводится уровень освещённости в зависимости от текущей задержки.
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
