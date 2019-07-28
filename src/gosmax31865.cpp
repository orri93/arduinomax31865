#include <Arduino.h>

#ifndef GOS_ARDUINO_UNIT_TESTING
// include Playing With Fusion MAX31865 libraries
#include <PlayingWithFusion_MAX31865_STRUCT.h>       // struct library
#endif

#define GOS_SENSOR_GLOBAL
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

#define GOS_MAX_31865_ERRT_RTD_HITH  "RTD HiTh"
#define GOS_MAX_31865_ERRT_RTD_LOTH  "RTD LoTh"
#define GOS_MAX_31865_ERRT_RTD_REF   "REF High"
#define GOS_MAX_31865_ERRT_RTD_FORCE "FORCE"
#define GOS_MAX_31865_ERRT_RTD_VOLT  "Voltage"
#define GOS_MAX_31865_ERRT_RTD_UNKN  "Unknown"

static const char max31865errortext[MAX_31865_ERROR_COUNT][MAX_31865_ERROR_LENGTH] = {
  GOS_MAX_31865_ERRT_RTD_HITH,
  GOS_MAX_31865_ERRT_RTD_LOTH,
  GOS_MAX_31865_ERRT_RTD_REF,
  GOS_MAX_31865_ERRT_RTD_FORCE,
  GOS_MAX_31865_ERRT_RTD_FORCE,
  GOS_MAX_31865_ERRT_RTD_VOLT,
  GOS_MAX_31865_ERRT_RTD_UNKN
};

static const uint8_t max31865errorlength[MAX_31865_ERROR_COUNT] = {
  sizeof(GOS_MAX_31865_ERRT_RTD_HITH),
  sizeof(GOS_MAX_31865_ERRT_RTD_LOTH),
  sizeof(GOS_MAX_31865_ERRT_RTD_REF),
  sizeof(GOS_MAX_31865_ERRT_RTD_FORCE),
  sizeof(GOS_MAX_31865_ERRT_RTD_FORCE),
  sizeof(GOS_MAX_31865_ERRT_RTD_VOLT),
  sizeof(GOS_MAX_31865_ERRT_RTD_UNKN)
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
void setup(
  const uint8_t& pincs,                        /* PIN for Chip Select CS is often 10 */
  const uint8_t& rtdtype,                     /* RTD type either PT100 or PT1000 */
  const uint8_t& wires,                       /* Often 3 wires rather than 4 */
  const int& spispeed,                        /* SPI_CLOCK_DIV16 (1MHz) */
  const uint8_t& spidatamode) {               /* MAX31865 works in MODE1 or MODE3 */
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

const char* error(const uint8_t& status, const uint8_t& fault, uint8_t& length) {
  if ((Error = ::gos::sensor::error(status, length)) != nullptr) {
    return Error;
  } else {
    for (i = 0; i < MAX_31865_ERROR_COUNT; i++) {
      if (max31865errormasks[i] & fault) {
        length = max31865errorlength[i];
        return max31865errortext[i];
      }
    }
  }
}

}
}
