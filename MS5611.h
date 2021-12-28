#pragma once
// Can be set to I2C or SPI
#define iSPI
//
//    FILE: MS5611.h
//  AUTHOR: Rob Tillaart
//          Erni - testing/fixes
// VERSION: 0.3.3
// PURPOSE: Arduino library for MS5611 temperature and pressure sensor
//     URL: https://github.com/RobTillaart/MS5611


#include "Arduino.h"

#ifdef iI2C
#include "Wire.h"
#endif

#ifdef iSPI
#include "SPI.h"
#endif


#define MS5611_LIB_VERSION                    (F("0.3.3"))


#ifdef iI2C
#define MS5611_READ_OK                        0
#endif
#ifdef iSPI
#define MS5611_READ_OK                        254
#endif

#define MS5611_ERROR_2                        2         // low level I2C error
#define MS5611_NOT_READ                       -999


enum osr_t
{
    OSR_ULTRA_HIGH = 12, // 10 millis
    OSR_HIGH       = 11, //  5 millis
    OSR_STANDARD   = 10, //  3 millis
    OSR_LOW        = 9,  //  2 millis
    OSR_ULTRA_LOW  = 8   //  1 millis    Default = backwards compatible
};


class MS5611
{
public:
#if defined (ESP8266) || defined(ESP32)
  bool     begin(uint8_t sda, uint8_t scl, TwoWire *wire = &Wire);
#endif

//ensures that only one library needs to be included
#ifdef iI2C
  explicit MS5611(uint8_t deviceAddress);
  bool     begin(TwoWire *wire = &Wire);
#endif

#ifdef iSPI
  explicit MS5611(uint8_t deviceAddress, SPISettings uspiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0)); // sensor does up to 20 MHZ, must be MSBFIRST, sensor supports mode0 and mode3
  bool     begin(SPIClass *spi = &SPI);
#endif

  bool     isConnected();

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


private:
  void     convert(const uint8_t addr, uint8_t bits);
  uint32_t readADC();
  uint16_t readProm(uint8_t reg);
  int      command(const uint8_t command);

  uint8_t  _address;
  uint8_t  _samplingRate;
  int32_t  _temperature;
  int32_t  _pressure;
  int      _result;
  float    C[7];
  uint32_t _lastRead;

  #ifdef iI2C
  TwoWire * _wire;
  #endif
  
  #ifdef iSPI
  SPIClass * _SPI;
  SPISettings _spiSettings;
  #endif
};


// -- END OF FILE --

