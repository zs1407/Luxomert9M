#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
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
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LCD_lib : public Print 
{
public:
    LiquidCrystal(uint8_t rs, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

    void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) 
    {
        _rs_pin = rs;
        _rw_pin = rw;
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
        
        if (_rw_pin != 255) 
        {
            pinMode(_rw_pin, OUTPUT);
        }
        pinMode(_enable_pin, OUTPUT);

        
        for (int i = 0; i < ((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i)
        {
            pinMode(_data_pins[i], OUTPUT);
        }

        delayMicroseconds(50000);
        digitalWrite(_rs_pin, LOW);
        digitalWrite(_enable_pin, LOW);

        if (_rw_pin != 255) 
        {
            digitalWrite(_rw_pin, LOW);
        }
      
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

        if (_rw_pin != 255) 
        {
            digitalWrite(_rw_pin, LOW);
        }

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
    uint8_t _rw_pin;
    uint8_t _enable_pin;
    uint8_t _data_pins[8];
    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;
    uint8_t _initialized;
    uint8_t _numlines;
    uint8_t _row_offsets[4];
};
