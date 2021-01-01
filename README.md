
[![Arduino CI](https://github.com/RobTillaart/ML8511/workflows/Arduino%20CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci)
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/RobTillaart/ML8511/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/RobTillaart/ML8511.svg?maxAge=3600)](https://github.com/RobTillaart/ML8511/releases)

# MS5611

Arduino library for MS5611 temperature and pressure sensor

## Description

The MS5611 is a high resolution temperature and pressure sensor.

Temperature is expressed in 1/100 of degrees, so to print it
one has to multiply it by 0.01.

Pressure is expressed in 1/100 of mBar, so to print it one has to multiply it 
by 0.01 too.

This high resolution is made possible by oversampling (many times).

Maybe the values should be fixed in the future to be more "user friendly"
so a float representing Celsius and Bar.

## Interface

- **MS5611(uint8_t deviceAddress)** constructor
- **bool begin()** initializes internals
- **bool isConnected()** checks availability of deviceaddress on the I2C bus
- **int read(uint8_t bits = 8)** the actual reading of the sensor. Returns MS5611_READ_OK upon success
- **int32_t getTemperature()** returns temperature. Subsequent calls will return same value until a new **read()** is called.
- **int32_t getPressure()** pressure is in 1/100 of a mBar. Multiply by 0.01 to get mBar. Subsequent calls will return same value until a new **read()** is called.
- **int getLastResult()** checks last I2C communication (replace with more informative error handling?)
- **uint32_t lastRead()** last time in millis() **read()** was done


## Disclaimer

The library is experimental. As I have no such sensor the quality is hard to test.
So if you happen to have such a sensor, please give it a try and let me know.


## TODO

- TODO's to issues.
- proper error handling
- redo lower level functions?
- check address range in constructor.
- Wire.begin must be called in begin()...
- float for temperature and pressure..

## Operation

See examples
