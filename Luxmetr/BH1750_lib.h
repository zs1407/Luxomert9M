//#define BH1750_h
#include <stdio.h>
#include <Arduino.h>
#include "Wire.h"

#if (ARDUINO >= 100)
#  define __wire_write(d) I2C->write(d)
#else
#  define __wire_write(d) I2C->send(d)
#endif

#if (ARDUINO >= 100)
#  define __wire_read() I2C->read()
#else
#  define __wire_read() I2C->receive()
#endif


#define BH1750_POWER_DOWN 0x00
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_DEFAULT_MTREG 69
#define BH1750_MTREG_MIN 31
#define BH1750_MTREG_MAX 254

#define UNCONFIGURED 0
#define CONTINUOUS_HIGH_RES_MODE 0x10
#define CONTINUOUS_HIGH_RES_MODE_2 0x11
#define CONTINUOUS_LOW_RES_MODE 0x13
#define ONE_TIME_HIGH_RES_MODE 0x20
#define ONE_TIME_HIGH_RES_MODE_2 0x21
#define ONE_TIME_LOW_RES_MODE 0x23
class BH1750_lib 
{
public:
    BH1750_lib(byte addr = 0x23) 
    {
        BH1750_I2CADDR = addr;
        I2C = &Wire;
    }
    bool begin(int mode = CONTINUOUS_HIGH_RES_MODE, byte addr = 0x23, TwoWire* i2c = nullptr) 
    {
        if (i2c) 
        {
            I2C = i2c;
        }
        if (addr) 
        {
            BH1750_I2CADDR = addr;
        }
        return (configure(mode) && setMTreg(BH1750_DEFAULT_MTREG));
    }
    bool configure(int mode) 
    {
        byte ack = 5;
        switch (mode)
        {

        case CONTINUOUS_HIGH_RES_MODE:
        case CONTINUOUS_HIGH_RES_MODE_2:
        case CONTINUOUS_LOW_RES_MODE:
        case ONE_TIME_HIGH_RES_MODE:
        case ONE_TIME_HIGH_RES_MODE_2:
        case ONE_TIME_LOW_RES_MODE:
            I2C->beginTransmission(BH1750_I2CADDR);
            __wire_write((uint8_t)mode);
            ack = I2C->endTransmission();
            _delay_ms(10);
            break;
        default:
            Serial.println(F("[BH1750] ERROR: Invalid mode"));
            break;
        switch (ack) {
        case 0:
            BH1750_MODE = mode;
            lastReadTimestamp = millis();
            return true;
        case 1:
            Serial.println(F("[BH1750] ERROR: too long for transmit buffer"));
            break;
        case 2:
            Serial.println(F("[BH1750] ERROR: received NACK on transmit of address"));
            break;
        case 3:
            Serial.println(F("[BH1750] ERROR: received NACK on transmit of data"));
            break;
        case 4:
            Serial.println(F("[BH1750] ERROR: other error"));
            break;
        default:
            Serial.println(F("[BH1750] ERROR: undefined error"));
            break;
        }
        return false;
    }

    
    bool measurementReady(bool maxWait = false) 
    {
        uint32_t delaytime = 0;
        switch (BH1750_MODE) 
        {
        case CONTINUOUS_HIGH_RES_MODE:
        case CONTINUOUS_HIGH_RES_MODE_2:
        case ONE_TIME_HIGH_RES_MODE:
        case ONE_TIME_HIGH_RES_MODE_2:
            maxWait ? delaytime = (180 * BH1750_MTreg / (byte)BH1750_DEFAULT_MTREG)
                : delaytime = (120 * BH1750_MTreg / (byte)BH1750_DEFAULT_MTREG);
            break;
        case CONTINUOUS_LOW_RES_MODE:
        case ONE_TIME_LOW_RES_MODE:
            maxWait ? delaytime = (24 * BH1750_MTreg / (byte)BH1750_DEFAULT_MTREG)
                : delaytime = (16 * BH1750_MTreg / (byte)BH1750_DEFAULT_MTREG);
            break;
        default:
            break;
        }
        u_int32 currentTimestamp = millis();
        if (currentTimestamp - lastReadTimestamp >= delaytime) 
            return true; 
        else
            return false;
    }
    bool setMTreg(byte MTreg) 
    {
      if (MTreg < BH1750_MTREG_MIN || MTreg > BH1750_MTREG_MAX) 
      {
        Serial.println(F("[BH1750] ERROR: MTreg out of range"));
        return false;
      }
      byte ack = 5;
      I2C->beginTransmission(BH1750_I2CADDR);
      __wire_write((0b01000 << 3) | (MTreg >> 5));
      ack = I2C->endTransmission();
      I2C->beginTransmission(BH1750_I2CADDR);
      __wire_write((0b011 << 5) | (MTreg & 0b11111));
      ack = ack | I2C->endTransmission();
      I2C->beginTransmission(BH1750_I2CADDR);
      __wire_write(BH1750_MODE);
      ack = ack | I2C->endTransmission();
      _delay_ms(10);
      switch (ack) 
      {
        case 0:
        BH1750_MTreg = MTreg;
        return true;
        case 1: // too long for transmit buffer
        Serial.println(F("[BH1750] ERROR: too long for transmit buffer"));
        break;
        case 2: // received NACK on transmit of address
        Serial.println(F("[BH1750] ERROR: received NACK on transmit of address"));
        break;
        case 3: // received NACK on transmit of data
        Serial.println(F("[BH1750] ERROR: received NACK on transmit of data"));
        break;
        case 4: // other error
        Serial.println(F("[BH1750] ERROR: other error"));
        break;
        default:
        Serial.println(F("[BH1750] ERROR: undefined error"));
        break;
      }
      return false;
    }
  

  
    float readLightLevel() 
    {

        if (BH1750_MODE == UNCONFIGURED) 
        {
            Serial.println(F("[BH1750] Device is not configured!"));
            return -2.0;
        }
 
        float level = -1.0;

        if (2 == I2C->requestFrom((int)BH1750_I2CADDR, (int)2)) 
        {
            u_int16 tmp = 0;
            tmp = __wire_read();
            tmp <<= 8;
            tmp |= __wire_read();
            level = tmp;
        }
        lastReadTimestamp = millis();

        if (level != -1.0) 
        {
            if (BH1750_MTreg != BH1750_DEFAULT_MTREG) 
            {
                level *= (float)((byte)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg);
            }
            if (BH1750_MODE == ONE_TIME_HIGH_RES_MODE_2 ||
                BH1750_MODE == CONTINUOUS_HIGH_RES_MODE_2) 
            {
                level /= 2;
            }

            level /= BH1750_CONV_FACTOR;
        }
        return level;
    }

private:
    byte BH1750_I2CADDR;
    byte BH1750_MTreg = (byte)BH1750_DEFAULT_MTREG;
    const float BH1750_CONV_FACTOR = 1.2;
    int BH1750_MODE = UNCONFIGURED;
    TwoWire* I2C;
    uint16_t lastReadTimestamp;
};

