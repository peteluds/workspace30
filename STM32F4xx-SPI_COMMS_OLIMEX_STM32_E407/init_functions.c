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
 * ADF4159 Frequency Synthesizer Register Values
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

/*
 * ADF4355 Frequency Synthesizer Register Values
 */

static uint8_t ADF4355_power_on_register_values_buf1[12][4] = {
{0x00,0x01,0x04,0x1C}, // Write to ADF4355 register 12
{0x00,0x61,0x30,0x0B}, // Write to ADF4355 register 11
{0x00,0xC0,0x3E,0xBA}, // Write to ADF4355 register 10
{0x3F,0x40,0x2C,0x89}, // Write to ADF4355 register 9
{0x10,0x2D,0x04,0x28}, // Write to ADF4355 register 8
{0x10,0x00,0x00,0x17}, // Write to ADF4355 register 7
{0x15,0x1F,0xE0,0x76}, // Write to ADF4355 register 6
{0x00,0x80,0x00,0x25}, // Write to ADF4355 register 5
{0x30,0x01,0x09,0x84}, // Write to ADF4355 register 4 [R divider output set to output half fPFD]
{0x00,0x00,0x00,0x03}, // Write to ADF4355 register 3
{0x00,0x00,0x40,0x02}, // Write to ADF4355 register 2 [For halved fPFD]
{0x00,0x00,0x00,0x01}, // Write to ADF4355 register 1 [For halved fPFD]
};

static uint8_t ADF4355_power_on_register_values_buf2[5][4] = {
{0x00,0x00,0xC0,0x40}, // Write to ADF4355 register 0 [For halved fPFD]
{0x30,0x00,0x89,0x84}, // Write to ADF4355 register 4 [R divider output set to output desired fPFD]
{0x00,0x00,0x40,0x02}, // Write to ADF4355 register 2 [For desired fPFD]
{0x00,0x00,0x00,0x01}, // Write to ADF4355 register 1 [For desired fPFD]
{0x00,0x00,0x03,0x20}, // Write to ADF4355 register 0 [For desired fPFD]
};

/*
 * ADA8282 Quad-Channel Low Noise Amplifier/Variable Gain Amplifier Register Values
 */

static uint8_t ADA8282_U403_power_on_register_values[7][3] = {
{0x00,0x00,0x00}, // Write to ADA8282 register 0x00 [INTF_CONFA]
{0x00,0x10,0x20}, // Write to ADA8282 register 0x10 [LNA_OFFSET0]
{0x00,0x11,0x20}, // Write to ADA8282 register 0x11 [LNA_OFFSET1]
{0x00,0x14,0x00}, // Write to ADA8282 register 0x14 [BIAS_SEL]
{0x00,0x15,0x00}, // Write to ADA8282 register 0x15 [PGA_GAIN]
{0x00,0x17,0x03}, // Write to ADA8282 register 0x17 [EN_CHAN]
{0x00,0x18,0x00}, // Write to ADA8282 register 0x18 [EN_BIAS_GEN]
};

static uint8_t ADA8282_U404_power_on_register_values[7][3] = {
{0x00,0x00,0x00}, // Write to ADA8282 register 0x00 [INTF_CONFA]
{0x00,0x10,0x20}, // Write to ADA8282 register 0x10 [LNA_OFFSET0]
{0x00,0x11,0x20}, // Write to ADA8282 register 0x11 [LNA_OFFSET1]
{0x00,0x14,0x00}, // Write to ADA8282 register 0x14 [BIAS_SEL]
{0x00,0x15,0x00}, // Write to ADA8282 register 0x15 [PGA_GAIN]
{0x00,0x17,0x03}, // Write to ADA8282 register 0x17 [EN_CHAN]
{0x00,0x18,0x00}, // Write to ADA8282 register 0x18 [EN_BIAS_GEN]
};


/*
 *@brief  ADF4159 register setup - set operating frequency, FMCW sweep characteristics
 */

void ADF4159_init(void){

    int i;

    /*
     * Program ADF4159 with power-on register values, i.e. load registers from 7-0, load registers 6/5/4 twice
     */

    //Select multiplexed line for ADF4159
    palClearPad(GPIOG, GPIOG_SPI_NSS_S0);
    palClearPad(GPIOG, GPIOG_SPI_NSS_S1);

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

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

    /*
     * Program ADF4355 with power-on register values, i.e. load registers from 12-1, note that registers 4/2/1 use fPFD/2 value
     */

    //Select multiplexed line for ADF4355
    palClearPad(GPIOG, GPIOG_SPI_NSS_S0);
    palSetPad(GPIOG, GPIOG_SPI_NSS_S1);

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure ADF4355 registers
    for(i = 0; i < 12; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,12,ADF4355_power_on_register_values_buf1[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

    chThdSleepMilliseconds(1); // Have to wait > 16 ADC_CLK cycles, which with ADC_CLK = 100 KHz is 161 uS, however with fPFD being divided by 2 this may be 50 KHz, hence meaning > 320 uS for 16 ADC_CLK cycles - wait for 1 mS to ensure compliance

    /*
     * Program ADF4355 with power-on register values, i.e. load registers 0, 4, 2, 1, 0, note that registers 4/2/1/0 use desired fPFD value upon 2nd load
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure ADF4355 registers
    for(i = 0; i < 5; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,5,ADF4355_power_on_register_values_buf2[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}


void ADA8282_init(void){

    int i;

    /*
     * Program ADA8282 U403/U404 with power-on register values
     */

    //Select multiplexed line for ADA8282 / U403
    palSetPad(GPIOG, GPIOG_SPI_NSS_S0);
    palClearPad(GPIOG, GPIOG_SPI_NSS_S1);

    // Configure ADA8282 / U403 registers
    for(i = 0; i < 7; i++){
      spiAcquireBus(&SPID1);
      spiStart(&SPID1, &hs_spicfg);
      spiSelect(&SPID1);
      spiSend(&SPID1,1,ADA8282_U403_power_on_register_values[i]);
      spiUnselect(&SPID1);
      //chThdSleepMilliseconds(200);
      spiStop(&SPID1);
      spiReleaseBus(&SPID1);
    }

    //Select multiplexed line for ADA8282 / U404
        palSetPad(GPIOG, GPIOG_SPI_NSS_S0);
        palSetPad(GPIOG, GPIOG_SPI_NSS_S1);

        // Configure ADA8282 / U404 registers
        for(i = 0; i < 7; i++){
          spiAcquireBus(&SPID1);
          spiStart(&SPID1, &hs_spicfg);
          spiSelect(&SPID1);
          spiSend(&SPID1,1,ADA8282_U404_power_on_register_values[i]);
          spiUnselect(&SPID1);
          //chThdSleepMilliseconds(200);
          spiStop(&SPID1);
          spiReleaseBus(&SPID1);
        }

}
