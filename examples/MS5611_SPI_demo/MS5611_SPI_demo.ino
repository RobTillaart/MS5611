//
//    FILE: MS5611_minimal.ino
//  AUTHOR: Alfredo Colas
// PURPOSE: demo application
//    DATE: 2021-12-29
//     URL: https://github.com/RobTillaart/MS5611


#include "MS5611.h"

// To be able to use SPI interface, `#define iI2C` at the top of the MS5611.h file *must* be changed to `#define iSPI`
// first argument is the digital pin Chip Select is connected to
// Second argument is optoinal. Do not include it unless you know what you are doing.
// It defines SPI communiction settings. see https://www.arduino.cc/en/Reference/SPISettings for more info
// irst argument must NOT exceed 2000000. Second aregument can only be MSBFIRST. Third argument can only be SPI_MODE0 and SPI_MODE3
MS5611 MS5611(10, SPISettings(2000000, MSBFIRST, SPI_MODE0));
// The rest of the library operates exactly the same as with I2C communication.


void setup()
{
  Serial.begin(115200);
  Serial.print(__FILE__);
  Serial.print("MS5611_LIB_VERSION: ");
  Serial.println(MS5611_LIB_VERSION);

  if (MS5611.begin() == true)
  {
    Serial.println("MS5611 found.");
  }
  else
  {
    Serial.println("MS5611 not found. halt.");
    while(1);
  }
}


void loop()
{
  MS5611.read();           // note no error checking => "optimistic".
  Serial.print("\nT:\t");
  Serial.print(MS5611.getTemperature(), 2);
  Serial.print("\tP:\t");
  Serial.print(MS5611.getPressure(), 2);
  delay(1000);
}


// -- END OF FILE --
