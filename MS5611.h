#pragma once
//
//    FILE: MS5611.h
//  AUTHOR: Rob Tillaart
//          Erni - testing/fixes
// VERSION: 0.3.8
// PURPOSE: Arduino library for MS5611 temperature and pressure sensor
//     URL: https://github.com/RobTillaart/MS5611


#include "Arduino.h"
#include "Wire.h"

//  BREAKOUT  MS5611  aka  GY63 - see datasheet
//
//  SPI    I2C
//              +--------+
//  VCC    VCC  | o      |
//  GND    GND  | o      |
//         SCL  | o      |
//  SDI    SDA  | o      |
//  CSO         | o      |
//  SDO         | o L    |   L = led
//          PS  | o    O |   O = opening  PS = protocol select
//              +--------+
//
//  PS to VCC  ==>  I2C  (GY-63 board has internal pull up, so not needed)
//  PS to GND  ==>  SPI
//  CS to VCC  ==>  0x76
//  CS to GND  ==>  0x77


#define MS5611_LIB_VERSION                    (F("0.3.8"))

#ifndef MS5611_DEFAULT_ADDRESS
#define MS5611_DEFAULT_ADDRESS                0x77
#endif

#define MS5611_READ_OK                        0
#define MS5611_ERROR_2                        2         // low level I2C error
#define MS5611_NOT_READ                       -999


enum osr_t
{
    OSR_ULTRA_HIGH = 12,        // 10 millis
    OSR_HIGH       = 11,        //  5 millis
    OSR_STANDARD   = 10,        //  3 millis
    OSR_LOW        = 9,         //  2 millis
    OSR_ULTRA_LOW  = 8          //  1 millis    Default = backwards compatible
};


class MS5611
{
public:
  explicit MS5611(uint8_t deviceAddress = MS5611_DEFAULT_ADDRESS);

#if defined (ESP8266) || defined(ESP32)
  bool     begin(uint8_t sda, uint8_t scl, TwoWire *wire = &Wire);
#endif
  bool     begin(TwoWire *wire = &Wire);
  bool     isConnected();

  // reset command + get constants
  // returns false if ROM constants == 0;
  bool     reset();

  // the actual reading of the sensor;
  // returns MS5611_READ_OK upon success
  int      read(uint8_t bits);
  // wrapper, uses the preset oversampling rate.
  inline int read() { return read( (uint8_t) _samplingRate); };

  // sets oversampling to a value between 8 and 12
  void     setOversampling(osr_t samplingRate);

  // oversampling rate is in osr_t
  osr_t    getOversampling() const { return (osr_t) _samplingRate; };

  // temperature is in ²C
  float    getTemperature() const;

  // pressure is in mBar
  float    getPressure() const;

  //  OFFSET - 0.3.6
  void     setPressureOffset(float offset = 0)    { _pressureOffset = offset; };
  float    getPressureOffset()    { return _pressureOffset; };
  void     setTemperatureOffset(float offset = 0) { _temperatureOffset = offset; };
  float    getTemperatureOffset() { return _temperatureOffset; };

  // to check for failure
  int      getLastResult() const   { return _result; };

  // last time in millis() when the sensor has been read.
  uint32_t lastRead() const        { return _lastRead; };

  uint32_t getDeviceID() const     { return _deviceID; };

  void     setCompensation(bool flag = true) { _compensation = flag; };
  bool     getCompensation() { return _compensation; };

  // develop functions.
  /*
  void     setAddress(uint8_t address) { _address = address; };  // RANGE CHECK + isConnected() !
  uint8_t  getAddress() const          { return _address; };
  uint8_t  detectAddress() { todo };  // works with only one on the bus?
  */


private:
  void     convert(const uint8_t addr, uint8_t bits);
  uint32_t readADC();
  uint16_t readProm(uint8_t reg);
  int      command(const uint8_t command);

  uint8_t  _address;
  uint8_t  _samplingRate;
  int32_t  _temperature;
  int32_t  _pressure;
  float    _pressureOffset;
  float    _temperatureOffset;
  int      _result;
  float    C[7];
  uint32_t _lastRead;
  uint32_t _deviceID;
  bool     _compensation;

  TwoWire * _wire;
};


// -- END OF FILE --

