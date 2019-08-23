#include <Arduino.h>

#include "gosmax31865.h"

#define MAX_31865_OK           0
#define MAX_31865_ERROR_COUNT  7
#define MAX_31865_ERROR_LENGTH 8 /* 7 character plus '\0' */

#define GOS_MAX_31865_ERRT_RTD_HITH  "RTD HTh"
#define GOS_MAX_31865_ERRT_RTD_LOTH  "RTD LTh"
#define GOS_MAX_31865_ERRT_RTD_REF   "REF Hi"
#define GOS_MAX_31865_ERRT_RTD_FORCE "FORCE"
#define GOS_MAX_31865_ERRT_RTD_VOLT  "Volt"
#define GOS_MAX_31865_ERRT_RTD_UNKN  "Unkn"

namespace gos {

namespace max31865 {
namespace error {
static const char text[MAX_31865_ERROR_COUNT][MAX_31865_ERROR_LENGTH] = {
  GOS_MAX_31865_ERRT_RTD_HITH,
  GOS_MAX_31865_ERRT_RTD_LOTH,
  GOS_MAX_31865_ERRT_RTD_REF,
  GOS_MAX_31865_ERRT_RTD_FORCE,
  GOS_MAX_31865_ERRT_RTD_FORCE,
  GOS_MAX_31865_ERRT_RTD_VOLT,
  GOS_MAX_31865_ERRT_RTD_UNKN
};
static const uint8_t length[MAX_31865_ERROR_COUNT] = {
  sizeof(GOS_MAX_31865_ERRT_RTD_HITH),
  sizeof(GOS_MAX_31865_ERRT_RTD_LOTH),
  sizeof(GOS_MAX_31865_ERRT_RTD_REF),
  sizeof(GOS_MAX_31865_ERRT_RTD_FORCE),
  sizeof(GOS_MAX_31865_ERRT_RTD_FORCE),
  sizeof(GOS_MAX_31865_ERRT_RTD_VOLT),
  sizeof(GOS_MAX_31865_ERRT_RTD_UNKN)
};
static const uint8_t mask[MAX_31865_ERROR_COUNT] =
{
    0x80,
    0x40,
    0x20,
    0x10,
    0x08,
    0x04,
    0xff
};
}
}

Max31865::Max31865(
  const uint8_t&  pincs,    /* PIN for Chip Select CS is often 10 */
  const uint32_t& clock,    /* SPI CLOCK 1000000 (1MHz) */
  const uint8_t&  order,    /* SPI bit order MSB FIRST */
  const uint8_t&  mode) :   /* MAX6677 works in MODE1 (or MODE3?) */
  Status(0), pincs_(pincs), spisettings_(clock, order, mode) {
}

void Max31865::initialize(
  const uint8_t& rtdtype,   /* RTD type either PT100 or PT1000 */
  const uint8_t& wires) {   /* Often 3 wires rather than 4 */

  wires_ = wires;

  pinMode(pincs_, OUTPUT);

#ifdef DELAY_INITIALIZE_MAX_31865
  // immediately pull CS pin high to avoid conflicts on SPI bus
  digitalWrite(pincs_, HIGH);
  delay(DELAY_INITIALIZE_MAX_31865);
#endif

  SPI.beginTransaction(spisettings_);

  // take the chip select low to select the device:
  digitalWrite(pincs_, LOW);

  configure();

  // take the chip select high to de-select, finish config write
  digitalWrite(pincs_, HIGH);

  SPI.endTransaction();

  rtdtype_ = rtdtype;
}

bool Max31865::read(double& value) {
  SPI.beginTransaction(spisettings_);

  // must set CS low to start operation
  digitalWrite(pincs_, LOW);

  // Write command telling IC that we want to 'read' and start at register 0
  SPI.transfer(0x00);         // plan to start read at the config register

  //
  // read registers in order
  //

  // configuration
  conf_reg_ = SPI.transfer(0x00);         // read 1st 8 bits

#ifndef SPI_NOT_USE_16_MAX_31865
  rtd_res_raw_ = SPI.transfer16(0x0000);  // RTD MSBs and LSBs
#else
  rtd_res_raw_  = SPI.transfer(0x00);     // RTD MSBs
  rtd_res_raw_ <<= 8;
  rtd_res_raw_ |= SPI.transfer(0x00);     // RTD LSBs
#endif
  rtd_res_ = rtd_res_raw_ >> 1;           // store data after 1-bit right shift

#ifndef SPI_NOT_USE_16_MAX_31865
  hft_val_ = SPI.transfer16(0x0000);      // High Fault Threshold MSB and LSB
#else
  hft_val_ = SPI.transfer(0x00);          // High Fault Threshold MSB
  hft_val_ <<= 8;
  hft_val_ |= SPI.transfer(0x00);         // High Fault Threshold LSB
#endif
  hft_val_ >>= 1;                         // store data after 1-bit right shift

#ifndef SPI_NOT_USE_16_MAX_31865
  lft_val_ = SPI.transfer16(0x0000);      // Low Fault Threshold MSB and LSB  
#else
  lft_val_  = SPI.transfer(0x00);         // Low Fault Threshold MSB
  lft_val_ <<= 8;
  lft_val_ |= SPI.transfer(0x00);         // Low Fault Threshold MSB
#endif
  lft_val_ >>= 1;                         // store data after 1-bit right shift

  Status = SPI.transfer(0x00);            // Fault Status read 8th 8 bits

  if (rtd_res_raw_ && Status == MAX_31865_OK) {
    // Calculate deg C
    value = (((double)rtd_res_) / 32.0) - 256.0;
  } else {
    configure();
  }

  // set CS high to finish read
  digitalWrite(pincs_, HIGH);

  SPI.endTransaction();

  return rtd_res_raw_ && Status == MAX_31865_OK;
}

#ifndef SPI_NO_ERROR_HANDLING_MAX_31865
const char* Max31865::error(uint8_t& length) {
  if (Status != MAX_31865_OK) {
    for (uint8_t i = 0; i < MAX_31865_ERROR_COUNT; i++) {
      if (::gos::max31865::error::mask[i] & Status) {
        length = ::gos::max31865::error::length[i];
        return ::gos::max31865::error::text[i];
      }
    }
  }
  length = 0;
  return nullptr;
}
#endif

void Max31865::configure() {
  // Write config to MAX31865 chip
  SPI.transfer(0x80);           // Send config register location to chip
                                // 0x8x to specify 'write register value' 
                                // 0xx0 to specify 'configuration register'
  SPI.transfer(0xC2 | wires_);  // Write config to IC
                                // bit 7: Vbias -> 1 (ON)
                                // bit 6: conversion mode -> 1 (AUTO)
                                // bit 5: 1-shot -> 0 (off)
                                // bit 4: 3-wire select -> 0 (2/4 wire config)
                                // bit 3-2: fault detection cycle -> 0 (none)
                                // bit 1: fault status clear -> 1 (clear any fault)
                                // bit 0: 50/60 Hz filter select -> 0 (60 Hz)

    // code if you need to configure High and Low fault threshold registers (4 total registers)
//  SPI.transfer(0x83);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0xFF);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0xFF);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0x00);  // write cmd, start at HFT MSB reg (0x83)
//  SPI.transfer(0x00);  // write cmd, start at HFT MSB reg (0x83)
}

}
