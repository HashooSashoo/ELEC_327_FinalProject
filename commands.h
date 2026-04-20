#ifndef commands_include
#define commands_include

#define NOP            (0x00) // no operation, do nothing
#define SWRESET        (0x01) // software reset, wait 5ms before next command (120ms after commanding to sleep)
#define RDDIDIF        (0x04) // read display identification information, sends back IDs of drivers as explained in datasheet
#define RDDST          (0x09) // read display status, sends back display status info (details in datasheet)
#define RDDPM          (0x0A) // read display power mode, info about power/sleep etc. more details in datasheet
#define RDDMADCTL      (0x0B) // read MADCTL, MADCTL is the configuration of the order of refreshing, reading, and writing the pixels on the screen
#define RDDCOLMOD      (0x0C) // read display pixel format, this gives how RGB vals are represented (16/18 bit) in 8-bit parallel and serial (more info on datasheet)
#define RDDIM          (0x0D) // read display image format, basically just give info about a gamma curve or not
#define RDDSM          (0x0E) // read display signal mode, tell us about whether all the cool parameters are on (VSYNC, HSYNC, anti-tearing etc.)
#define RDDSDR         (0x0F) // read display self diagnostic result, tells us if the display is working right through internal check
#define SLPIN          (0x10) // enter sleep mode, minimal power consumption, screen still displays, just can't change
#define SLPOUT         (0x11) // exits sleep mode
#define PTLON          (0x12) // turns on partial mode, turns some modules on and some off based on specifications given by partial area command
#define NORON          (0x13) // turns partial mode off
#define DINVOFF        (0x20) // display inversion OFF, turns off display inversion
#define DINVON         (0x21) // display inversion ON, reverses the color of each pixel
#define GAMSET         (0x26) // gamma set, selects the desired gamma curve for the display
#define DISPOFF        (0x28) // display OFF, blanks the screen while keeping frame memory intact
#define DISPON         (0x29) // display ON, recovers from display OFF mode
#define CASET          (0x2A) // column address set, defines the horizontal window for pixel writing, send 16-bit start column coord and 16-bit end column coord
#define PASET          (0x2B) // page address set, defines the vertical window for pixel writing, send 16-bit start column coord and 16-bit end column coord
#define RAMWR          (0x2C) // memory write, tells the driver that following data is pixel color data for the frame buffer
#define RAMRD          (0x2E) // memory read, allows reading pixel data back from the frame buffer
#define PTLAR          (0x30) // partial area, defines the start and end rows for partial mode
#define VSCRDEF        (0x33) // vertical scrolling definition, sets the top, bottom, and scrolling zones of the screen
#define TEOFF          (0x34) // tearing effect line OFF, disables the synchronization signal line
#define TEON           (0x35) // tearing effect line ON, enables sync signal to help prevent "tearing" during fast updates
#define MADCTL         (0x36) // memory access control, sets rotation, mirroring, and BGR/RGB color order
#define VSCRSADD       (0x37) // vertical scrolling start address, defines which row of memory is at the top of the display
#define IDMOFF         (0x38) // idle mode OFF, restores full color depth
#define IDMON          (0x39) // idle mode ON, reduces color depth (usually 8-color) to save power
#define COLMOD         (0x3A) // pixel format set, defines the color depth (16-bit or 18-bit) and interface format
#define RAMWR_CONT     (0x3C) // write memory continue, allows resuming pixel writes from previous location
#define RAMRD_CONT     (0x3E) // read memory continue, allows resuming pixel reads from previous location
#define WRDISBV        (0x51) // write display brightness, sets the brightness value of the display (if supported)
#define WRCTRLD        (0x53) // write control display, used to control backlight and dimming functions
#define WRCABC         (0x55) // write content adaptive brightness control, settings for automatic brightness adjustments
#define WRCABCMB       (0x5E) // write CABC minimum brightness, prevents the screen from getting too dark
#define FRMCTR1        (0xB1) // frame rate control (normal mode), sets the refresh rate for standard operation
#define FRMCTR2        (0xB2) // frame rate control (idle mode), sets the refresh rate when in low-power idle
#define FRMCTR3        (0xB3) // frame rate control (partial mode), sets the refresh rate for partial display areas
#define INVCTR         (0xB4) // display inversion control, determines how the display is electrically driven (dot/column/frame)
#define DFUNCTR        (0xB6) // display function control, configures scan direction and drive timing
#define PWCTR1         (0xC0) // power control 1, sets the internal voltage (GVDD)
#define PWCTR2         (0xC1) // power control 2, sets the internal voltage (VGH/VGL)
#define VMCTR1         (0xC5) // VCOM control 1, fine-tunes the common voltage to prevent flickering
#define VMCTR2         (0xC7) // VCOM control 2, secondary VCOM adjustment
#define RDID1          (0xDA) // read ID1, returns the LCD module's manufacturer ID
#define RDID2          (0xDB) // read ID2, returns the LCD module's version/type ID
#define RDID3          (0xDC) // read ID3, returns the LCD module's driver ID
#define GMCTRP1        (0xE0) // positive gamma correction, calibration values for the positive voltage curve
#define GMCTRN1        (0xE1) // negative gamma correction, calibration values for the negative voltage curve
#define IFCTR          (0xF6) // interface control, sets up how the driver communicates with the MCU
#define PRCTR          (0xF7) // pump ratio control, sets the internal charge pump voltage levels

#include <stdint.h>
#include <stdbool.h>

// ---- Pin Configuration (fill in your actual pins) ----
// Which GPIO port (GPIOA or GPIOB)
#define LCD_CS_PORT     GPIOA
#define LCD_DC_PORT     GPIOA
#define LCD_RST_PORT    GPIOA

// Pin indices within that port
#define LCD_CS_PIN      (1 << 27)   // example: PA27
#define LCD_DC_PIN      (1 << 12)   // example: PA12
#define LCD_RST_PIN     (1 << 13)   // example: PA13

// IOMUX PINCM indices for those pins (for configuring as GPIO output)
#define LCD_CS_PINCM    IOMUX_PINCM60   // example: matches PA12
#define LCD_DC_PINCM    IOMUX_PINCM34   // example: matches PA13
#define LCD_RST_PINCM   IOMUX_PINCM35   // example: matches PA14


void LCD_DC_Command(void);
void LCD_DC_Data(void);
void LCD_CS_Low(void);
void LCD_CS_High(void);
void LCD_RST_Low(void);
void LCD_RST_High(void);

void SPI_SendByte(uint8_t byte);

void LCD_InitSPI(void);
void LCD_SendCommand(uint8_t cmd, const uint8_t *data, uint16_t data_len);
void LCD_SendData(const uint8_t *data, uint16_t len);
void LCD_HardReset(void);

#endif // COMMAND_H