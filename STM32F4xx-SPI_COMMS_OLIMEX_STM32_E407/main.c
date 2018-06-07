/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

// @file main.c
/// @brief The main process and executable.
///
/// @author Peter Ludlow

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ch.h"
#include "hal.h"
#include "global.h"
#include "init_functions.h"



/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Setup of ADF4159/ADF4355/ADA8282 register values
   */
  chThdSleepMilliseconds(1000);
  ADF4159_init();
  chThdSleepMilliseconds(1000);
  ADF4355_init();
  chThdSleepMilliseconds(1000);
  ADA8282_init();
  chThdSleepMilliseconds(1000);

  //AD9648_init();
//  chThdSleepMilliseconds(1000);


  /*
   * Normal main() thread activity, the LED on the PCB blinks on and off at 0.1 second intervals
   */
  while (true) {


//   ADF4159_init();
//     chThdSleepMilliseconds(50);
//   ADF4355_init();
//    chThdSleepMilliseconds(50);
//    ADA8282_init();
//     chThdSleepMilliseconds(50);

//    AD9648_write_func();
//    chThdSleepMilliseconds(50);
//    AD9648_read_func();

    palSetPad(GPIOC, GPIOC_LED_SPI);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC, GPIOC_LED_SPI);
    chThdSleepMilliseconds(500);
  }
}
