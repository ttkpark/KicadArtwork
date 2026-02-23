/**
 * @file oled_sh1106.h
 * @brief 1.3" SPI OLED (M130-12864-7-V2.0) SH1106 driver - software SPI
 *        Pins: PA4=NSS, PA5=CLK, PA7=MOSI, PA8=RST, PA9=DC
 */
#ifndef __OLED_SH1106_H
#define __OLED_SH1106_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   (OLED_HEIGHT / 8)

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ClearRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);  /* set region to off */
void OLED_Refresh(void);           /* send framebuffer to display */

void OLED_SetPixel(uint8_t x, uint8_t y, bool on);
void OLED_DrawLineH(uint8_t x0, uint8_t x1, uint8_t y);
void OLED_DrawLineV(uint8_t x, uint8_t y0, uint8_t y1);
void OLED_DrawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void OLED_DrawFillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void OLED_DrawCircle(uint8_t cx, uint8_t cy, uint8_t r, bool filled);  /* filled or outline */
void OLED_DrawChar(uint8_t x, uint8_t y, char c);  /* 6x8 font */
void OLED_DrawString(uint8_t x, uint8_t y, const char *s);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_SH1106_H */
