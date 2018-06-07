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
 * SPI configuration (SPI Speed = fVCO_CLK/4/128 = 328.125kHz, CPHA=1 [2ND CLK transition is the 1st data capture edge], CPOL=1 [CLK to 1 when idle], MSB first).
*/

static const SPIConfig hs_spicfg = {
        NULL,
        GPIOA,
        GPIOA_SPI1_NSS,
        SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA
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
{0x00,0x63,0x04,0x83}, // Write to ADF4159 register 3
{0x00,0x40,0x81,0x92}, // Write to ADF4159 register 2
{0x00,0x00,0x00,0x01}, // Write to ADF4159 register 1
{0xB0,0x36,0x60,0x00}, // Write to ADF4159 register 0
};

static uint8_t ADF4159_register_values_buf[8][4] = {
{0x00,0x00,0x00,0x07}, // Write to ADF4159 register 7
{0x00,0x00,0x3E,0x86}, // Write to ADF4159 register 6
{0x00,0x12,0x8F,0x75}, // Write to ADF4159 register 5

{0x00,0x18,0x00,0x84}, // Write to ADF4159 register 4 // Ramp Status = Normal Operation
//{0x00,0x78,0x00,0x84}, // Write to ADF4159 register 4 // Ramp Status = Ramp Complete to MUXOUT

{0x00,0x63,0x04,0x83}, // Write to ADF4159 register 3 // Continuous triangular ramp
//{0x00,0x63,0x00,0x83}, // Write to ADF4159 register 3 // Continuous sawtooth ramp

{0x00,0x40,0x81,0x92}, // Write to ADF4159 register 2 // Charge pump current = min
//{0x0F,0x40,0x81,0x92}, // Write to ADF4159 register 2 // Charge pump current = max


{0x00,0x00,0x00,0x01}, // Write to ADF4159 register 1


//{0xB0,0x36,0x40,0x00}, // Write to ADF4159 register 0 // 21.65-22.65 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x36,0x40,0x00}, // Write to ADF4159 register 0 // 21.65-22.65 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x36,0x60,0x00}, // Write to ADF4159 register 0 // 21.75-22.75 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x36,0x60,0x00}, // Write to ADF4159 register 0 // 21.75-22.75 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x37,0x00,0x00}, // Write to ADF4159 register 0 // 22.0-23.0 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x37,0x00,0x00}, // Write to ADF4159 register 0 // 22.0-23.0 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x37,0x20,0x00}, // Write to ADF4159 register 0 // 22.05-23.05 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x37,0x20,0x00}, // Write to ADF4159 register 0 // 22.05-23.05 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x37,0x60,0x00}, // Write to ADF4159 register 0 // 22.15-23.15 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x37,0x60,0x00}, // Write to ADF4159 register 0 // 22.15-23.15 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x37,0xA0,0x00}, // Write to ADF4159 register 0 // 22.25-23.25 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x37,0xA0,0x00}, // Write to ADF4159 register 0 // 22.25-23.25 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0xB0,0x38,0x00,0x00}, // Write to ADF4159 register 0 // 22.4-23.4 GHz FMCW Ramp enabled, MUXOUT = Digital Lock Detect
//{0xF8,0x38,0x00,0x00}, // Write to ADF4159 register 0 // 22.4-23.4 GHz FMCW Ramp enabled, MUXOUT = Ramp Complete

//{0x30,0x36,0x40,0x00}, // Write to ADF4159 register 0 // 21.65 GHz frequency, FMCW Ramp disabled, MUXOUT = Digital Lock Detect

{0x30,0x36,0x60,0x00}, // Write to ADF4159 register 0 // 21.75 GHz frequency, FMCW Ramp disabled, MUXOUT = Digital Lock Detect
//{0x08,0x36,0x60,0x00}, // Write to ADF4159 register 0 // 21.75 GHz frequency, FMCW Ramp disabled, MUXOUT = DVDD
//{0x18,0x36,0x60,0x00}, // Write to ADF4159 register 0 // 21.75 GHz frequency, FMCW Ramp disabled, MUXOUT = R DIVIDER OUTPUT


//{0x30,0x38,0xC0,0x00}, // Write to ADF4159 register 0 // 22.65 GHz frequency, FMCW Ramp disabled, MUXOUT = Digital Lock Detect
//{0x30,0x38,0xE0,0x00}, // Write to ADF4159 register 0 // 22.75 GHz frequency, FMCW Ramp disabled, MUXOUT = Digital Lock Detect

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

//{0x15,0x1F,0xE0,0x76}, // Write to ADF4355 register 6 // RF Output power = Maximum (+5 dBm)
{0x15,0x1F,0xE0,0x66}, // Write to ADF4355 register 6 // RF Output power = +2 dBm
//{0x15,0x1F,0xE0,0x46}, // Write to ADF4355 register 6 // RF Output power = Minimum (-4 dBm)

{0x00,0x80,0x00,0x25}, // Write to ADF4355 register 5
{0x30,0x01,0x09,0x84}, // Write to ADF4355 register 4 [R divider output set to output half fPFD]
{0x00,0x00,0x00,0x03}, // Write to ADF4355 register 3

//**SET BELOW REGISTERS FOR EITHER 5.4 GHz or 5.402 GHz OPERATION**//

//{0x00,0x00,0x40,0x02}, // Write to ADF4355 register 2 [For halved fPFD] // IF frequency = 5.4 GHz
//{0x00,0x20,0x00,0x01}, // Write to ADF4355 register 1 [For halved fPFD] // IF frequency = 5.4 GHz
{0x36,0x9D,0x55,0x52}, // Write to ADF4355 register 2 [For halved fPFD] // IF frequency = 5.402 GHz
{0x00,0xA3,0xD7,0x01}, // Write to ADF4355 register 1 [For halved fPFD] // IF frequency = 5.402 GHz

};

static uint8_t ADF4355_power_on_register_values_buf2[5][4] = {
//{0x00,0x20,0x06,0x40}, // Write to ADF4355 register 0 [For halved fPFD] // IF frequency = 5.0 GHz
{0x00,0x20,0x06,0xC0}, // Write to ADF4355 register 0 [For halved fPFD] // IF frequency = 5.4 GHz /5.402 GHz
{0x30,0x00,0x89,0x84}, // Write to ADF4355 register 4 [R divider output set to output desired fPFD]

//**SET BELOW REGISTERS FOR EITHER 5.4 GHz or 5.402 GHz OPERATION**//

//{0x00,0x00,0x40,0x02}, // Write to ADF4355 register 2 [For desired fPFD] IF frequency = 5.4 GHz
//{0x00,0x00,0x00,0x01}, // Write to ADF4355 register 1 [For desired fPFD] IF frequency = 5.4 GHz
{0x51,0xEF,0xFF,0xF2}, // Write to ADF4355 register 2 [For desired fPFD] IF frequency = 5.402 GHz
{0x00,0x51,0xEB,0x81}, // Write to ADF4355 register 1 [For desired fPFD] IF frequency = 5.402 GHz

//**

{0x00,0x20,0x03,0x60}, // Write to ADF4355 register 0 [For desired fPFD] // IF frequency = 5.4 GHz / 5.402 GHz
};

/*
 * ADA8282 Quad-Channel Low Noise Amplifier/Variable Gain Amplifier Register Values
 */

static uint8_t ADA8282_U404_power_on_register_values[7][3] = {
{0x00,0x00,0x00}, // Write to ADA8282 register 0x00 [INTF_CONFA]
{0x00,0x10,0x20}, // Write to ADA8282 register 0x10 [LNA_OFFSET0]
{0x00,0x11,0x20}, // Write to ADA8282 register 0x11 [LNA_OFFSET1]
{0x00,0x14,0x00}, // Write to ADA8282 register 0x14 [BIAS_SEL]
//{0x00,0x15,0x00}, // Write to ADA8282 register 0x15 [PGA_GAIN] (minimum gain)
{0x00,0x15,0xFF}, // Write to ADA8282 register 0x15 [PGA_GAIN] (maximum gain)
{0x00,0x17,0x07}, // Write to ADA8282 register 0x17 [EN_CHAN]
{0x00,0x18,0x00}, // Write to ADA8282 register 0x18 [EN_BIAS_GEN]
};

static uint8_t ADA8282_U405_power_on_register_values[7][3] = {
{0x00,0x00,0x00}, // Write to ADA8282 register 0x00 [INTF_CONFA]
{0x00,0x10,0x20}, // Write to ADA8282 register 0x10 [LNA_OFFSET0]
{0x00,0x11,0x20}, // Write to ADA8282 register 0x11 [LNA_OFFSET1]
{0x00,0x14,0x00}, // Write to ADA8282 register 0x14 [BIAS_SEL]
//{0x00,0x15,0x00}, // Write to ADA8282 register 0x15 [PGA_GAIN] (minimum gain)
{0x00,0x15,0xFF}, // Write to ADA8282 register 0x15 [PGA_GAIN] (maximum gain)
{0x00,0x17,0x07}, // Write to ADA8282 register 0x17 [EN_CHAN]
{0x00,0x18,0x00}, // Write to ADA8282 register 0x18 [EN_BIAS_GEN]
};

static uint8_t ADA8282_U404_power_on_register_values_readback[7][3] = {
{0x80,0x00,0x00}, // Write to ADA8282 register 0x00 [INTF_CONFA]
{0x80,0x10,0x20}, // Write to ADA8282 register 0x10 [LNA_OFFSET0]
{0x80,0x11,0x20}, // Write to ADA8282 register 0x11 [LNA_OFFSET1]
{0x80,0x14,0x00}, // Write to ADA8282 register 0x14 [BIAS_SEL]
//{0x00,0x15,0x00}, // Write to ADA8282 register 0x15 [PGA_GAIN] (minimum gain)
{0x80,0x15,0xFF}, // Write to ADA8282 register 0x15 [PGA_GAIN] (maximum gain)
{0x80,0x17,0x07}, // Write to ADA8282 register 0x17 [EN_CHAN]
{0x80,0x18,0x00}, // Write to ADA8282 register 0x18 [EN_BIAS_GEN]
};

static uint8_t AD9648_init1[6][3] = {
{0x00, 0x05, 0x03}, /* Select both ADC channels */
{0x00, 0xFF, 0x01}, /* Transfer bit */
{0x00, 0x08, 0x03}, /* Digital reset of the ADC chip */
{0x00, 0xFF, 0x01}, /* Transfer bit */
};

static uint8_t AD9648_init2[2][3] = {
{0x00, 0x08, 0x00}, /* Normal operation of the ADC chip */
{0x00, 0xFF, 0x01}, /* Transfer bit */
};

static uint8_t AD9648_init3[4][3] = {
{0x00, 0x00, 0x3C}, /* Soft Reset */
{0x00, 0xFF, 0x01}, /* Transfer bit */
{0x00, 0x04, 0x00}, /* Device index B */
{0x00, 0x05, 0x01}, /* Device index A */
};

static uint8_t AD9648_write[1][3] = {
{0x00, 0x0D, 0x02}, /* Test mode */
//{0x00, 0xFF, 0x01}, /* Transfer bit */
};


static uint8_t AD9648_read[2][3] = {
//{0x00, 0x04, 0x00}, /* Device index B */
//{0x00, 0x05, 0x01}, /* Device index A */
{0x80, 0x01, 0x00}, /* Chip ID */
//{0x00, 0xFF, 0x01}, /* Transfer bit */
//{0x00, 0x04, 0x00}, /* Device index B */
//{0x00, 0x05, 0x01}, /* Device index A */
{0x80, 0x0D, 0x00}, /* Test Mode */
//{0x00, 0xFF, 0x01}, /* Transfer bit */
};


/*
 *@brief  ADF4159 register setup - set operating frequency, FMCW sweep characteristics
 */

void AD9648_init(void){

    int i;

    /*
     * Program AD9648 with desired register values
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure AD9648 registers
    for(i = 0; i < 4; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,3,AD9648_init1[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

    chThdSleepMilliseconds(1000);


    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure AD9648 registers
    for(i = 0; i < 2; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,3,AD9648_init2[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

    chThdSleepMilliseconds(1000);


    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure AD9648 registers
    for(i = 0; i < 4; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,3,AD9648_init3[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

    chThdSleepMilliseconds(1000);

}

void AD9648_write_func(void){

    int i;

    /*
     * Program AD9648 with desired register values
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure AD9648 registers
    for(i = 0; i < 1; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,3,AD9648_write[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}

void AD9648_read_func(void){

    int i;

    /*
     * Program AD9648 with desired register values
     */

    spiAcquireBus(&SPID1);
    spiStart(&SPID1, &hs_spicfg);

    // Configure AD9648 registers
    for(i = 0; i < 2; i++){
        spiSelect(&SPID1);
        spiSend(&SPID1,3,AD9648_read[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}

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
        spiSend(&SPID1,4,ADF4159_power_on_register_values_buf[i]);
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
        spiSend(&SPID1,4,ADF4159_register_values_buf[i]);
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
        spiSend(&SPID1,4,ADF4355_power_on_register_values_buf1[i]);
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
        spiSend(&SPID1,4,ADF4355_power_on_register_values_buf2[i]);
        spiUnselect(&SPID1);
    }

    //chThdSleepMilliseconds(200);
    spiStop(&SPID1);
    spiReleaseBus(&SPID1);

}


void ADA8282_init(void){

    int i;

    /*
     * Program ADA8282 U404/U405 with power-on register values
     */

    //Select multiplexed line for ADA8282 / U404
    palSetPad(GPIOG, GPIOG_SPI_NSS_S0);
    palClearPad(GPIOG, GPIOG_SPI_NSS_S1);

    // Configure ADA8282 / U404 registers
    for(i = 0; i < 7; i++){
      spiAcquireBus(&SPID1);
      spiStart(&SPID1, &hs_spicfg);
      spiSelect(&SPID1);
      //spiSend(&SPID1,3,ADA8282_U404_power_on_register_values[i]);
      spiSend(&SPID1,3,ADA8282_U404_power_on_register_values[i]);
      spiUnselect(&SPID1);
      //chThdSleepMilliseconds(200);
      spiStop(&SPID1);
      spiReleaseBus(&SPID1);
    }

    //Select multiplexed line for ADA8282 / U405
        palSetPad(GPIOG, GPIOG_SPI_NSS_S0);
        palSetPad(GPIOG, GPIOG_SPI_NSS_S1);

        // Configure ADA8282 / U405 registers
        for(i = 0; i < 7; i++){
          spiAcquireBus(&SPID1);
          spiStart(&SPID1, &hs_spicfg);
          spiSelect(&SPID1);
          spiSend(&SPID1,3,ADA8282_U405_power_on_register_values[i]);
          spiUnselect(&SPID1);
          //chThdSleepMilliseconds(200);
          spiStop(&SPID1);
          spiReleaseBus(&SPID1);
        }

}
