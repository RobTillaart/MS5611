#pragma once
// Can be set to iI2C or iSPI
// #define iI2C
//
//    FILE: MS5611.h
//  AUTHOR: Rob Tillaart
//          Erni - testing/fixes
//          Alfredo Colas(LyricPants66133)
// VERSION: 0.4.0
// PURPOSE: Arduino library for MS5611 temperature and pressure sensor
//     URL: https://github.com/RobTillaart/MS5611


#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"


#define MS5611_LIB_VERSION                    (F("0.4.0"))


#define MS5611_READ_OK                        0
#define MS5611_ERROR_2                        2         // low level I2C error
#define MS5611_NOT_READ                       -999

// datasheet page 10
#define MS5611_CMD_READ_ADC       0x00
#define MS5611_CMD_READ_PROM      0xA0
#define MS5611_CMD_RESET          0x1E
#define MS5611_CMD_CONVERT_D1     0x40
#define MS5611_CMD_CONVERT_D2     0x50

enum osr_t
{
    OSR_ULTRA_HIGH = 12, // 8.22 millis
    OSR_HIGH       = 11, //  4.1 millis
    OSR_STANDARD   = 10, //  2.1 millis
    OSR_LOW        = 9,  //  1.1 millis
    OSR_ULTRA_LOW  = 8   //  0.5 millis    Default = backwards compatible
};


class MS5611_base
{
public:
#if defined (ESP8266) || defined(ESP32)
  bool     begin(uint8_t sda, uint8_t scl, TwoWire *wire = &Wire);
#endif

  explicit MS5611_base() {};

  // reset command + get constants
  void     reset();

  // the actual reading of the sensor;
  // returns MS5611_READ_OK upon success
  int      read(uint8_t bits);
  inline int read() { return read( (uint8_t) _samplingRate); };  // uses the preset oversampling

  // sets oversampling to a value between 8 and 12
  void     setOversampling(osr_t samplingRate);

  // oversampling rate is in osr_t
  osr_t    getOversampling() const { return (osr_t) _samplingRate; };

  // temperature is in ²C
  float    getTemperature() const  { return _temperature * 0.01; };

  // pressure is in mBar
  float    getPressure() const     { return _pressure * 0.01; };

  // to check for failure
  int      getLastResult() const   { return _result; };

  // last time in millis() that the sensor has been read.
  uint32_t lastRead()              { return _lastRead; };

protected:
  // MS5611_protocol _protocol;
  SPISettings _spiSettings;
  uint8_t  _address;
  uint8_t  _samplingRate;
  int32_t  _temperature;
  int32_t  _pressure;
  int      _result;
  float    C[7];
  uint32_t _lastRead;

private:
  virtual void     convert(const uint8_t addr, uint8_t bits);
  virtual uint32_t readADC();
  virtual uint16_t readProm(uint8_t reg);
  virtual int      command(const uint8_t command);
};



class MS5611 : public MS5611_base
{
public:
  explicit MS5611(uint8_t deviceAddress);
  bool     begin(TwoWire *wire = &Wire);
  bool     isConnected();

protected:
  void     convert(const uint8_t addr, uint8_t bits);
  uint32_t readADC();
  uint16_t readProm(uint8_t reg);
  int      command(const uint8_t command);

private:
  TwoWire * _wire;
};


class MS5611_SPI : public MS5611_base
{
public:
  explicit MS5611_SPI(uint8_t deviceAddress, SPISettings uspiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0));
  bool     begin(SPIClass *SPI = &SPI);
  bool     isConnected();

protected:
  void     convert(const uint8_t addr, uint8_t bits);
  uint32_t readADC();
  uint16_t readProm(uint8_t reg);
  int      command(const uint8_t command);

private:
  SPIClass * _SPI;

};


// -- END OF FILE --

