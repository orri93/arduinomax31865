#ifndef _GOS_ARDUINO_LIBRARY_MAX_31865_H_
#define _GOS_ARDUINO_LIBRARY_MAX_31865_H_

#include <SPI.h>

#include <PlayingWithFusion_MAX31865.h>              // core library

#define RTD_TYPE_PT100         1 
#define RTD_TYPE_PT1000        2

namespace gos {
namespace max31865 {
void setup(
  const uint8_t& pincs = 10,                  /* PIN for Chip Select CS is often 10 */
  const uint8_t& rtdtype = RTD_TYPE_PT100,    /* RTD type either PT100 or PT1000 */
  const uint8_t& wires = RTD_3_WIRE,          /* Often 3 wires rather than 4 */
  const int& spispeed = SPI_CLOCK_DIV16,      /* SPI_CLOCK_DIV16 (1MHz) */
  const uint8_t& spidatamode = SPI_MODE3);    /* MAX31865 works in MODE1 or MODE3 */
double read(uint8_t& status, uint8_t& fault);
const char* error(const uint8_t& status, const uint8_t& fault);
}
}

#endif /* _GOS_ARDUINO_LIBRARY_MAX_31865_H_ */
