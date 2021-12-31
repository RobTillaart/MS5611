//
//    FILE: MS5611.cpp
//  AUTHOR: Rob Tillaart
//          Erni - testing/fixes
//          Alfredo Colas(LyricPants66133)
// VERSION: 0.4.0
// PURPOSE: MS5611 Temperature & Humidity library for Arduino
//     URL: https://github.com/RobTillaart/MS5611
//
//  HISTORY:
//  0.4.0   2021-12-29  Add SPI Interface
//                      Split MS5611 class into 3 classes
//  0.3.4   2021-12-29  fix #16 compilation for MBED
//  0.3.3   2021-12-25  Update oversampling timings to reduce time spent waiting
//  0.3.2   2021-12-24  add get/set oversampling, read() (thanks to LyricPants66133)
//  0.3.1   2021-12-21  update library.json, readme, license, minor edits
//  0.3.0   2021-01-27  fix #9 math error (thanks to Emiel Steerneman)
//                      add Wire1..WireN support (e.g. teensy)
//                      changed getTemperature() and getPressure()
//                      add reset()
//
//  0.2.2   2021-01-01  add Arduino-CI + unit tests + isConnected()
//  0.2.1   2020-06-28  fix #1 min macro compile error
//  0.2.0   2020-06-21  refactor; #pragma once; 
//
//  0.1.8               fix #109 incorrect constants (thanks to flauth)
//  0.1.7               revert double to float (issue 33)
//  0.1.6   2015-07-12  refactor
//  0.1.05  moved 6 float multiplies to init  [adds ~70 bytes !!!]
//          moved the MS5611_LIB_VERSION to PROGMEM
//  0.1.04  changed float to double (for platforms which support it)
//          changed divisions in multiplications
//          fixed uint32_t readADC()
//          reduced size of C array by 1 float
//          added second order temperature compensation
//  0.1.03  changed math to float [test version]
//  0.1.02  fixed bug return value read()
//          fixed bug #bits D2
//          added MS5611_READ_OK
//          added inline getters for temp & pres & lastResult.
//          adjusted delay's based on datasheet
//          merged convert functions
//          fixed offset in readProm()
//  0.1.01  small refactoring
//  0.1.00  added temperature and Pressure code
//  0.0.00  initial version by Rob Tillaart (15-okt-2014)


#include "MS5611.h"


/////////////////////////////////////////////////////
//
// PUBLIC - I2C
//
MS5611::MS5611(uint8_t deviceAddress)
{
  _address      = deviceAddress;
  _samplingRate = OSR_ULTRA_LOW;
  _temperature  = MS5611_NOT_READ;
  _pressure     = MS5611_NOT_READ;
  _result       = MS5611_NOT_READ;
  _lastRead     = 0;
}


#if defined (ESP8266) || defined(ESP32)
bool MS5611_I2C::begin(uint8_t dataPin, uint8_t clockPin, TwoWire * wire)
{
  if ((_address < 0x76) || (_address > 0x77)) return false;

  _wire = wire;
  if ((dataPin < 255) && (clockPin < 255))
  {
    _wire->begin(dataPin, clockPin);
  } else {
    _wire->begin();
  }
  if (! isConnected()) return false;

  reset();
  return true;
}
#endif


bool MS5611::begin(TwoWire * wire)
{
  if ((_address < 0x76) || (_address > 0x77)) return false;
  _wire = wire;
  _wire->begin();
  if (! isConnected()) return false;

  reset();
  return true;
}


bool MS5611::isConnected() // Operational
{
  _wire->beginTransmission(_address);
  _result = _wire->endTransmission();
  return (_result == 0);
}


/////////////////////////////////////////////////////
//
// PUBLIC - SPI
//
MS5611_SPI::MS5611_SPI(uint8_t deviceAddress, SPISettings uspiSettings)
{
  _spiSettings  = uspiSettings;
  _address      = deviceAddress;  // SPI selected, represents CS pin
  _samplingRate = OSR_ULTRA_LOW;
  _temperature  = MS5611_NOT_READ;
  _pressure     = MS5611_NOT_READ;
  _result       = MS5611_NOT_READ;
  _lastRead     = 0;
}


bool MS5611_SPI::begin(SPIClass * SPI)
{
  _SPI = SPI;
  _SPI->begin();
  if (! isConnected()) return false;
  
  MS5611_base::reset();
  return true;
}


bool MS5611_SPI::isConnected() // Operational
{
  return (command(0x00) == MS5611_READ_OK); // 0X00 asks for a byte
}


/////////////////////////////////////////////////////
//
// PUBLIC - base
//
void MS5611_base::reset() // Operational
{
  command(MS5611_CMD_RESET);
  delayMicroseconds(2800);
  // constants that were multiplied in read()
  // do this once and you save CPU cycles
  C[0] = 1;
  C[1] = 32768L;          // SENSt1 = C[1] * 2^15
  C[2] = 65536L;          // OFFt1 = C[2] * 2^16
  C[3] = 3.90625E-3;      // TCS = C[3] / 2^6
  C[4] = 7.8125E-3;       // TCO = C[4] / 2^7
  C[5] = 256;             // Tref = C[5] * 2^8
  C[6] = 1.1920928955E-7; // TEMPSENS = C[6] / 2^23
  // read factory calibrations from EEPROM.
  for (uint8_t reg = 0; reg < 7; reg++)
  {
    // used indices match datasheet.
    // C[0] == manufacturer - read but not used;
    // C[7] == CRC - skipped.
    C[reg] *= readProm(reg);
  }
}


int MS5611_base::read(uint8_t bits) // operational
{
  // VARIABLES NAMES BASED ON DATASHEET
  // ALL MAGIC NUMBERS ARE FROM DATASHEET

  convert(MS5611_CMD_CONVERT_D1, bits);
  // if (_result) return _result;
  // NOTE: D1 and D2 seem reserved in MBED (NANO BLE)
  uint32_t _D1 = readADC();
  // if (_result) return _result;

  convert(MS5611_CMD_CONVERT_D2, bits);
  // if (_result) return _result;
  uint32_t _D2 = readADC();
  // if (_result) return _result;
  
  //  TEST VALUES - comment lines above
  // uint32_t D1 = 9085466;
  // uint32_t D2 = 8569150;

  // TEMP & PRESS MATH - PAGE 7/20
  float dT = _D2 - C[5];
  _temperature = 2000 + dT * C[6];

  float offset =  C[2] + dT * C[4];
  float sens = C[1] + dT * C[3];

  // SECOND ORDER COMPENSATION - PAGE 8/20
  // COMMENT OUT < 2000 CORRECTION IF NOT NEEDED
  // NOTE TEMPERATURE IS IN 0.01 C
  if (_temperature < 2000)
  {
    float T2 = dT * dT * 4.6566128731E-10;
    float t = (_temperature - 2000) * (_temperature - 2000);
    float offset2 = 2.5 * t;
    float sens2 = 1.25 * t;
    // COMMENT OUT < -1500 CORRECTION IF NOT NEEDED
    if (_temperature < -1500)
    {
      t = (_temperature + 1500) * (_temperature + 1500);
      offset2 += 7 * t;
      sens2 += 5.5 * t;
    }
    _temperature -= T2;
    offset -= offset2;
    sens -= sens2;
  }
  // END SECOND ORDER COMPENSATION

  _pressure = (_D1 * sens * 4.76837158205E-7 - offset) * 3.051757813E-5;

  _lastRead = millis();
  return MS5611_READ_OK;
}


void MS5611_base::setOversampling(osr_t samplingRate)
{
  _samplingRate = (uint8_t) samplingRate;
}


/////////////////////////////////////////////////////
//
// PRIVATE - I2C
//
void MS5611::convert(const uint8_t addr, uint8_t bits)
{
  //Values from page 2 datasheet
  uint16_t del[5] = {500, 1100, 2100, 4100, 8220};

  bits = constrain(bits, 8, 12); //works //works
  uint8_t offset = (bits - 8) * 2;
  command(addr + offset);
  delayMicroseconds(del[offset/2]);
}


uint16_t MS5611::readProm(uint8_t reg)
{
  // last EEPROM register is CRC - Page13 datasheet.
  uint8_t promCRCRegister = 7;
  if (reg > promCRCRegister) return 0;

  uint8_t offset = reg * 2;

  command(MS5611_CMD_READ_PROM + offset); //send read command
  if (_result == 0)
  {
    int nr = _wire->requestFrom(_address, (uint8_t)2);
    if (nr >= 2)
    {
      uint16_t val = _wire->read() * 256;
      val += _wire->read();
      return val;
    }
    return 0;
  }
  return 0;
}


uint32_t MS5611::readADC() // Operational
{
  command(MS5611_CMD_READ_ADC);
  if (_result == 0)
  {
    int nr = _wire->requestFrom(_address, (uint8_t)3);
    if (nr >= 3)
    {
      uint32_t val = _wire->read() * 65536UL;
      val += _wire->read() * 256UL;
      val += _wire->read();
      return val;
    }
    return 0UL;
  }
  return 0UL;
}


// Consider adding seperate spi with a byte and delay argument
int MS5611::command(const uint8_t command) //Operational
{
  yield();

  _wire->beginTransmission(_address);
  _wire->write(command);
  _result = _wire->endTransmission();

  return _result;
}


/////////////////////////////////////////////////////
//
// PRIVATE - SPI
//
void MS5611_SPI::convert(const uint8_t addr, uint8_t bits)
{
  //Values from page 2 datasheet
  uint16_t del[5] = {500, 1100, 2100, 4100, 8220};

  bits = constrain(bits, 8, 12); //works //works
  uint8_t offset = (bits - 8) * 2;

  _SPI->beginTransaction(_spiSettings); // start SPI
  digitalWrite(_address, LOW);          // select device
  _SPI->transfer(addr + offset);        // send command
  delayMicroseconds(del[offset/2]);
  digitalWrite(_address, HIGH);         // de-select device
  _SPI->endTransaction();               // end SPI
}


uint16_t MS5611_SPI::readProm(uint8_t reg)
{
  // last EEPROM register is CRC - Page13 datasheet.
  uint8_t promCRCRegister = 7;
  if (reg > promCRCRegister) return 0;

  uint8_t offset = reg * 2;

  _SPI->beginTransaction(_spiSettings);                    // start SPI
  digitalWrite(_address, LOW);                             // select device
  _SPI->transfer(MS5611_CMD_READ_PROM + offset); // send command

  uint16_t val = _SPI->transfer(0x00) * 256;             // Get first byte
  val += _SPI->transfer(0x00);                           // Get second byte

  digitalWrite(_address, HIGH);                          // de-select device
  _SPI->endTransaction();                                // end SPI
  
  return val;
}


uint32_t MS5611_SPI::readADC() // Operational
{
  _SPI->beginTransaction(_spiSettings);           // start SPI
  digitalWrite(_address, LOW);                    // select device
  
  _SPI->transfer(MS5611_CMD_READ_ADC);           // send command

  uint32_t val = _SPI->transfer(0x00) * 65536UL; // Get first byte
  val += _SPI->transfer(0x00) * 256UL;           // Get second byte
  val += _SPI->transfer(0x00);                   // Get third byte

  digitalWrite(_address, HIGH);                  // de-select device
  _SPI->endTransaction();                        // end SPI
  return val;
}


int MS5611_SPI::command(const uint8_t command) //Operational
{
  yield();

  _SPI->beginTransaction(_spiSettings); // start SPI
  digitalWrite(_address, LOW);          // select device
  _SPI->transfer(command);              // send command
  _result = _SPI->transfer(0x00);       // receive first byte
  digitalWrite(_address, HIGH);         // de-select device
  _SPI->endTransaction();               // end SPI

  return _result;
}


// -- END OF FILE --

