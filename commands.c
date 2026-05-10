// commands.c
#include "commands.h"
#include "delay.h"
#include <stdint.h>
#include <ti/devices/msp/msp.h>

// --------------------------------
// LOW LEVEL PIN HELPERS
// --------------------------------

void SetDC_Command(void) { // Sets DC to 0 to have screen take in commands
    GPIOA->DOUTCLR31_0 = LCD_DC_PIN;
}
void SetDC_Data(void) { // Sets DC to 1 to have screen take in data
    GPIOA->DOUTSET31_0 = LCD_DC_PIN;
}

void SetCS_Low(void) { // Sets CS to low to start communication
    GPIOA->DOUTCLR31_0 = LCD_CS_PIN;
}
void SetCS_High(void) { // Sets CS high to end communication
    GPIOA->DOUTSET31_0 = LCD_CS_PIN;
}

void SetRST_Low(void) { // ACTIVE LOW, set low to actually reset
    GPIOA->DOUTCLR31_0 = LCD_RST_PIN;
}
void SetRST_High(void) { // stays high by default
    GPIOA->DOUTSET31_0 = LCD_RST_PIN;
}

// THE MAIN BYTE SENDER!!! will use a lot
void SPI_SendByte(uint8_t byte) {
    SPI1->TXDATA = byte;
    while (SPI1->STAT & SPI_STAT_BUSY_MASK);    // wait for byte to be done transmitting
}

// --------------------------------
// SPI + GPIO SETUP FOR SPI
// --------------------------------

void InitSPIModule(void) {
        // SPI peripheral reset and power
    SPI1->GPRCM.RSTCTL = (SPI_RSTCTL_KEY_UNLOCK_W |          // key unlock
                          SPI_RSTCTL_RESETSTKYCLR_CLR |     // reset past clear status holder
                          SPI_RSTCTL_RESETASSERT_ASSERT);   // assert reset
    SPI1->GPRCM.PWREN =  (SPI_PWREN_KEY_UNLOCK_W |            // power enable key unlock
                          SPI_PWREN_ENABLE_ENABLE);          // power enable

    delay_cycles(POWER_STARTUP_DELAY);

    SPI1->CLKSEL = SPI_CLKSEL_SYSCLK_SEL_ENABLE; // use system clock (32 MHz) as reference clock (need this for speed)
    SPI1->CLKDIV = SPI_CLKDIV_RATIO_DIV_BY_1;    // basically don't divide the clock source, we need all the speed we can get!
    SPI1->CTL0 = SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_FIRST |  // SPI mode 0
                 SPI_CTL0_FRF_MOTOROLA_3WIRE |            // no hardware CS
                 SPI_CTL0_DSS_DSS_8;                      // 8-bit frames
    SPI1->CTL1 = SPI_CTL1_CP_ENABLE |      // microcontroller is the controller
                 SPI_CTL1_PREN_DISABLE |   // no parity bit
                 SPI_CTL1_PTEN_DISABLE |   // no parity transmission
                 SPI_CTL1_MSB_ENABLE;      // transmit most significant bit first

    // 16 MHz: 32MHz / ((1+0) * 2)
    SPI1->CLKCTL = 0;

    SPI1->CTL1 |= SPI_CTL1_ENABLE_ENABLE; // enable SPI

    // SPI pin muxing
    IOMUX->SECCFG.PINCM[IOMUX_PINCM26] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM26_PF_SPI1_SCLK; // SPI clock
    IOMUX->SECCFG.PINCM[IOMUX_PINCM24] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM24_PF_SPI1_POCI; // SPI MISO
    IOMUX->SECCFG.PINCM[IOMUX_PINCM25] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM25_PF_SPI1_PICO; // SPI MOSI
}

void InitGPIOPins(void) {
            // GPIO power-on
    GPIOA->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |                 // key unlock
                                    GPIO_RSTCTL_RESETSTKYCLR_CLR |    // clear reset status bit
                                    GPIO_RSTCTL_RESETASSERT_ASSERT);  // assert reset

    GPIOA->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |                  // enable power key
                                    GPIO_PWREN_ENABLE_ENABLE);        // enable power
    delay_cycles(MS_TO_CYCLES(10));

    // DC, CS, RST as GPIO outputs
    IOMUX->SECCFG.PINCM[LCD_CS_PINCM]  = IOMUX_PINCM_PC_CONNECTED | GPIO_CONFIG;
    IOMUX->SECCFG.PINCM[LCD_DC_PINCM]  = IOMUX_PINCM_PC_CONNECTED | GPIO_CONFIG;
    IOMUX->SECCFG.PINCM[LCD_RST_PINCM] = IOMUX_PINCM_PC_CONNECTED | GPIO_CONFIG;

    // output enable the GPIO pins
    GPIOA->DOESET31_0 = LCD_DC_PIN;
    GPIOA->DOESET31_0 = LCD_CS_PIN;
    GPIOA->DOESET31_0 = LCD_RST_PIN;
}

void LCD_InitSPI(void) {

    // initialize the SPI and GPIO modules
    InitSPIModule();
    InitGPIOPins();

    // set initial values before screen configuration
    SetCS_High();  // no transmission
    SetDC_Data();  // data mode
    SetRST_High(); // no reset

}

// ----------------------------------
// FUNCTIONS TO SEND COMMANDS TO SCREEN
// ----------------------------------

void LCD_SendCommand(uint8_t cmd, const uint8_t* data, uint16_t data_len) {
    // start command transmission with CS -> low, DC -> low
    SetCS_Low();
    SetDC_Command();

    // send the command byte
    SPI_SendByte(cmd);

    // send any data as bytes
    if ((data != 0x00) && (data_len > 0)) { // this is a flag where if pointer is 0x00 or data_len <= 0, no data transmission happens 
        SetDC_Data();
        for (uint16_t i = 0; i < data_len; i++) {
            SPI_SendByte(data[i]);
        }
    }

    // end transmission
    SetCS_High();
}

// this is just the above function but without sending the command (i dont think I even use this lol)
void LCD_SendData(const uint8_t* data, uint16_t len) {
    SetCS_Low();
    SetDC_Data();
    for (uint16_t i = 0; i < len; i++) {
        SPI_SendByte(data[i]);
    }
    SetCS_High();
}

// ---------------------------------------------
// HARDWARE RESET: uses the reset pin to hardware reset the screen
// ---------------------------------------------
void LCD_HardReset(void) {
    SetRST_High();
    delay_cycles(4000);
    SetRST_Low();
    delay_cycles(MS_TO_CYCLES(10));   // 10ms low pulse
    SetRST_High();
    delay_cycles(MS_TO_CYCLES(120));  // 120ms recovery
}