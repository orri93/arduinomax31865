#include <Arduino.h>

#ifndef GOS_ARDUINO_UNIT_TESTING
// include Playing With Fusion MAX31865 libraries
#include <PlayingWithFusion_MAX31865_STRUCT.h>       // struct library
#endif

#include <arduinosensor.h>

#include "gosmax31865.h"

#define MAX_31865_ERROR_COUNT  7
#define MAX_31865_ERROR_LENGTH 9 /* 8 character plus '\0' */

namespace gos {
namespace max31865 {

#ifndef GOS_ARDUINO_UNIT_TESTING
// structures for holding RTD values
static PWFusion_MAX31865_RTD* pwf_max_rtd;
struct var_max31865 max_rtd_values;
#endif

static const char max31865errortext[MAX_31865_ERROR_COUNT][MAX_31865_ERROR_LENGTH] = {
  "RTD HiTh",
  "RTD LoTh" ,
  "REF High",
  "FORCE",
  "FORCE",
  "Voltage",
  "Unknown"
};
static const uint8_t max31865errormasks[MAX_31865_ERROR_COUNT] =
{
    0x80,
    0x40,
    0x20,
    0x10,
    0x08,
    0x04,
    0xff
};

static const char* Error;
static uint8_t i;

#ifndef GOS_ARDUINO_UNIT_TESTING
bool setup(
  const uint8_t& pincs,
  const uint8_t& rtdtype,
  const uint8_t& wires,
  const int& spispeed,
  const uint8_t& spidatamode) {
  // setup for the the SPI library:
  SPI.begin();                            // begin SPI
  SPI.setClockDivider(spispeed);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
  SPI.setDataMode(spidatamode);             // MAX31865 works in MODE1 or MODE3

    // initalize the chip select pin
  pinMode(pincs, OUTPUT);

  pwf_max_rtd = new PWFusion_MAX31865_RTD(pincs);

  // configure channel 0
  pwf_max_rtd->MAX31865_config(&max_rtd_values, wires);
  max_rtd_values.RTD_type = rtdtype;
}

double read(uint8_t& status, uint8_t& fault) {
  /* Read max */
  pwf_max_rtd->MAX31865_full_read(&max_rtd_values);
  if (max_rtd_values.status == 0) {
    gos::sensor::value = (((double)max_rtd_values.rtd_res_raw) / 32.0) - 256.0;
    status = gos::sensor::range::check(reading);
    return gos::sensor::value;
  } else {
    status = SENSOR_STATUS_ERROR;
    fault = max_rtd_values.status;
  }
}
#endif

const char* error(const uint8_t& status, const uint8_t& fault) {
  if ((Error = ::gos::sensor::error(status)) != nullptr) {
    return Error;
  } else {
    for (i = 0; i < MAX_31865_ERROR_COUNT; i++) {
      if (max31865errormasks[i] & fault) {
        return max31865errortext[i];
      }
    }
  }
}

}
}