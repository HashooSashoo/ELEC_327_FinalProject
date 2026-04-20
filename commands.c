// command.c
#include "commands.h"
#include "delay.h"
#include <ti/devices/msp/msp.h>

// PUBLIC
// Flags for interface to main code
bool spi_wakeup;

// PRIVATE
// Data buffer and length for SPI TX - we'll load this with a function
uint16_t *spi_message;
int      spi_message_len;
int      spi_message_idx;
bool     spi_transmission_in_progress;

// ----- Low-level pin helpers -----

void LCD_DC_Command(void) {
    LCD_DC_PORT->DOUTCLR31_0 = LCD_DC_PIN;
}

void LCD_DC_Data(void) {
    LCD_DC_PORT->DOUTSET31_0 = LCD_DC_PIN;
}

void LCD_CS_Low(void) {
    LCD_CS_PORT->DOUTCLR31_0 = LCD_CS_PIN;
}

void LCD_CS_High(void) {
    LCD_CS_PORT->DOUTSET31_0 = LCD_CS_PIN;
}

void LCD_RST_Low(void) {
    LCD_RST_PORT->DOUTCLR31_0 = LCD_RST_PIN;
}

void LCD_RST_High(void) {
    LCD_RST_PORT->DOUTSET31_0 = LCD_RST_PIN;
}

// ----- Blocking SPI byte send -----

void SPI_SendByte(uint8_t byte) {
    SPI1->TXDATA = byte;
    while (SPI1->STAT & SPI_STAT_BUSY_MASK);
}

// ----- Public: SPI + GPIO setup -----

void LCD_InitSPI(void) {
    // GPIO power-on
    if (LCD_DC_PORT->GPRCM.STAT & GPIO_STAT_RESETSTKY_MASK) {
        LCD_DC_PORT->GPRCM.RSTCTL = (GPIO_RSTCTL_KEY_UNLOCK_W |
                                      GPIO_RSTCTL_RESETSTKYCLR_CLR |
                                      GPIO_RSTCTL_RESETASSERT_ASSERT);
        LCD_DC_PORT->GPRCM.PWREN  = (GPIO_PWREN_KEY_UNLOCK_W |
                                      GPIO_PWREN_ENABLE_ENABLE);
        delay_cycles(POWER_STARTUP_DELAY);
    }

    // DC, CS, RST as GPIO outputs
    IOMUX->SECCFG.PINCM[LCD_CS_PINCM]  = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM60_PF_GPIOA_DIO27;
    IOMUX->SECCFG.PINCM[LCD_DC_PINCM]  = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM34_PF_GPIOA_DIO12;
    IOMUX->SECCFG.PINCM[LCD_RST_PINCM] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM35_PF_GPIOA_DIO13;

    LCD_DC_PORT->DOE31_0  |= LCD_DC_PIN;
    LCD_CS_PORT->DOE31_0  |= LCD_CS_PIN;
    LCD_RST_PORT->DOE31_0 |= LCD_RST_PIN;

    LCD_CS_High();
    LCD_DC_Data();
    LCD_RST_High();

    // SPI pin muxing
    IOMUX->SECCFG.PINCM[IOMUX_PINCM26] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM26_PF_SPI1_SCLK; // SPI Clock
    IOMUX->SECCFG.PINCM[IOMUX_PINCM24] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM24_PF_SPI1_POCI; // SPI MISO
    IOMUX->SECCFG.PINCM[IOMUX_PINCM25] = IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM25_PF_SPI1_PICO; // SPI MOSI

    // SPI peripheral reset and power
    SPI1->GPRCM.RSTCTL = (SPI_RSTCTL_KEY_UNLOCK_W |
                           SPI_RSTCTL_RESETSTKYCLR_CLR |
                           SPI_RSTCTL_RESETASSERT_ASSERT);
    SPI1->GPRCM.PWREN = (SPI_PWREN_KEY_UNLOCK_W |
                          SPI_PWREN_ENABLE_ENABLE);
    delay_cycles(POWER_STARTUP_DELAY);

    SPI1->CLKSEL = SPI_CLKSEL_SYSCLK_SEL_ENABLE;
    SPI1->CLKDIV = SPI_CLKDIV_RATIO_DIV_BY_1;

    SPI1->CTL0 = SPI_CTL0_SPO_LOW | SPI_CTL0_SPH_FIRST |  // SPI Mode 0
                 SPI_CTL0_FRF_MOTOROLA_3WIRE |              // No hardware CS
                 SPI_CTL0_DSS_DSS_8;                        // 8-bit frames

    SPI1->CTL1 = SPI_CTL1_CP_ENABLE |
                 SPI_CTL1_PREN_DISABLE |
                 SPI_CTL1_PTEN_DISABLE |
                 SPI_CTL1_MSB_ENABLE;

    // 2 MHz: 32MHz / ((1+7) * 2)
    SPI1->CLKCTL = 0;

    SPI1->CTL1 |= SPI_CTL1_ENABLE_ENABLE;
}

// ----- Public: transport layer -----

void LCD_SendCommand(uint8_t cmd, const uint8_t* data, uint16_t data_len) {
    LCD_CS_Low();
    LCD_DC_Command();
    SPI_SendByte(cmd);

    if (data != 0x00 && data_len > 0) {
        LCD_DC_Data();
        for (uint16_t i = 0; i < data_len; i++) {
            SPI_SendByte(data[i]);
        }
    }

    LCD_CS_High();
}

void LCD_SendData(const uint8_t* data, uint16_t len) {
    LCD_CS_Low();
    LCD_DC_Data();
    for (uint16_t i = 0; i < len; i++) {
        SPI_SendByte(data[i]);
    }
    LCD_CS_High();
}

void LCD_HardReset(void) {
    LCD_RST_High();
    delay_cycles(4000);
    LCD_RST_Low();
    delay_cycles(MS_TO_CYCLES(10));   // ~10ms low pulse
    LCD_RST_High();
    delay_cycles(MS_TO_CYCLES(120));  // ~120ms recovery
}