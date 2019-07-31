#ifndef _GOS_ARDUINO_LIBRARY_MAX_31865_H_
#define _GOS_ARDUINO_LIBRARY_MAX_31865_H_

#include <Arduino.h>

#include <SPI.h>

#define SPI_CLOCK_MAX_31865        1000000 /* (1MHz) */
#define SPI_MODE_MAX_31865       SPI_MODE3
#define SPI_BIT_ORDER_MAX_31865   MSBFIRST

#define RTD_TYPE_PT100                   1
#define RTD_TYPE_PT1000                  2

#define RTD_2_WIRE		                0x00
#define	RTD_3_WIRE		                0x10
#define	RTD_4_WIRE		                0x00

//#define DELAY_INITIALIZE_MAX_31865     100

namespace gos {
class Max31865 {
public:
  Max31865(
    const uint8_t&  pincs = 10,                       /* PIN for Chip Select CS is often 10 */
    const uint32_t& clock = SPI_CLOCK_MAX_31865,      /* SPI CLOCK 1000000 (1MHz) */
    const uint8_t&  order = SPI_BIT_ORDER_MAX_31865,  /* SPI bit order MSB FIRST */
    const uint8_t&  mode = SPI_MODE_MAX_31865);       /* MAX31865 works in MODE1 or MODE3 */
  void initialize(
    const uint8_t& rtdtype = RTD_TYPE_PT100,          /* RTD type either PT100 or PT1000 */
    const uint8_t& wires = RTD_3_WIRE);               /* Often 3 wires rather than 4 */
  bool read(double& value);
#ifndef SPI_NO_ERROR_HANDLING_MAX_31865
  const char* error(uint8_t& length);
#else
  const uint8_t& status();
#endif
private:
  uint8_t pincs_;
  SPISettings spisettings_;

  uint8_t rtdtype_;       // RTD type. 1 = PT100; 2 = PT1000
  uint8_t wires_;         // # of wires. 2/4-wire: 0x00, 3-wire: 0x10

  uint8_t  conf_reg_;			// Configuration register readout
  uint16_t rtd_res_raw_;  // RTD IC raw resistance register
  uint16_t rtd_res_;      // RTD IC resistance
  int16_t  hft_val_;			// High fault threshold register readout
  int16_t  lft_val_;			// Low fault threshold register readout
  uint8_t  status_;			  // RTD status - full status code

  void configure();
};

}

#endif /* _GOS_ARDUINO_LIBRARY_MAX_31865_H_ */
