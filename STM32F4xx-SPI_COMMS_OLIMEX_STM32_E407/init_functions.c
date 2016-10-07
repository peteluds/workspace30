/// @file init_functions.c
/// @brief Setup of ADF4159 and ADF4355 frequency synthesizer ICs
///
/// @author Peter Ludlow

#include <string.h>
#include <stdio.h>
#include "ch.h"
#include "hal.h"
#include "global.h"


/*===============================================================*/
/*Configuration Structures                                       */
/*===============================================================*/

/**
 *@brief  SPI Configuration
 *@note   SPI configuration for SPI1
 */

// fVCOCLK = (fPLL_CLK_IP x PLLN/PLLM)/PLLP = (12 MHz x 336/12)/2 = 168 MHz

/*
 * SPI configuration (SPI Speed = fVCO_CLK/4/128 = 328.125kHz, CPHA=0 [1st CLK transition is the 1st data capture edge], CPOL=0 [CLK to 0 when idle], MSB first).
*/

static const SPIConfig hs_spicfg = {
        NULL,
        GPIOA,
        GPIOA_SPI1_NSS,
        SPI_CR1_BR_2 | SPI_CR1_BR_1
};


/*
 * ADF4159/ADF4355 Frequency Synthesizer Register Values
 */

static uint8_t ADF4159_power_on_register_values_buf[11][4] = {
{0x00,0x00,0x00,0x07}, // Write to ADF4159 register 7
{0x00,0x00,0x3E,0x86}, // Write to ADF4159 register 6 [with STEP SEL = 0]
{0x00,0x80,0x3E,0x86}, // Write to ADF4159 register 6 [with STEP SEL = 1]
{0x00,0x12,0x8F,0x75}, // Write to ADF4159 register 5 [with DEV SEL = 0]
{0x00,0x92,0x8F,0x75}, // Write to ADF4159 register 5 [with DEV SEL = 1]
{0x00,0x18,0x00,0x84}, // Write to ADF4159 register 4 [with CLK DIV SEL = 0]
{0x00,0x18,0x00,0xC4}, // Write to ADF4159 register 4 [with CLK DIV SEL = 1]
{0x00,0x63,0x04,0xC3}, // Write to ADF4159 register 3
{0x10,0x40,0x01,0x92}, // Write to ADF4159 register 2
{0x00,0x00,0x00,0x01}, // Write to ADF4159 register 1
{0xB0,0x36,0x60,0x00}, // Write to ADF4159 register 0
};

static uint8_t ADF4159_register_values_buf[8][4] = {
{0x00,0x00,0x00,0x07}, // Write to ADF4159 register 7
{0x00,0x00,0x3E,0x86}, // Write to ADF4159 register 6
{0x00,0x12,0x8F,0x75}, // Write to ADF4159 register 5
{0x00,0x18,0x00,0x84}, // Write to ADF4159 register 4
{0x00,0x63,0x04,0xC3}, // Write to ADF4159 register 3
{0x10,0x40,0x01,0x92}, // Write to ADF4159 register 2
{0x00,0x00,0x00,0x01}, // Write to ADF4159 register 1
{0xB0,0x36,0x60,0x00}, // Write to ADF4159 register 0
};

static uint8_t ADF4355_register_values_buf[13][4] = {
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 0
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 1
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 2
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 3
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 4
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 5
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 6
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 7
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 8
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 9
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 10
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 11
{0x34,0x34,0x34,0x34}, // Write to ADF4355 register 12
};

/*
 *@brief  ADF4159 register setup - set operating frequency, FMCW sweep characteristics
 */

void ADF4159_init(void){

    int i;

    /*
     * Program ADF4159 with power-on register values, i.e. load registers from 7-0, load registers 6/5/4 twice
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    //Select multiplexed line
    palClearPad(GPIOG, GPIOG_SPI_NSS_S0);
    palClearPad(GPIOG, GPIOG_SPI_NSS_S1);

    // Configure ADF4159 registers
    for(i = 0; i < 11; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,11,ADF4159_power_on_register_values_buf[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

    /*
     * Program ADF4159 with desired register values
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    //Select multiplexed line
    palClearPad(GPIOG, GPIOG_SPI_NSS_S0);
    palClearPad(GPIOG, GPIOG_SPI_NSS_S1);

    // Configure ADF4159 registers
    for(i = 0; i < 8; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,8,ADF4159_register_values_buf[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}


void ADF4355_init(void){

    int i;

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    //Select multiplexed line
    palClearPad(GPIOG, GPIOG_SPI_NSS_S0);
    palSetPad(GPIOG, GPIOG_SPI_NSS_S1);

    // Configure ADF4355 registers
    for(i = 0; i < 13; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,13,ADF4355_register_values_buf[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}
