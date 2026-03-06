/**
 * @file oled_sh1106.c
 * @brief SH1106 128x64 SPI OLED driver - uses HAL SPI from STM32CubeMX
 *        Pins from main.h: OLED_NSS, OLED_RST, OLED_DC
 */
#include "oled_sh1106.h"
#include "main.h"

/* Use hspi1 from main.c (initialized by MX_SPI1_Init) */
extern SPI_HandleTypeDef hspi1;

#define OLED_CMD   0
#define OLED_DATA  1

/* Framebuffer: 128 x (64/8) = 1024 bytes */
static uint8_t s_fb[OLED_WIDTH * OLED_PAGES];

static void DC(uint8_t cmd) {
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, cmd ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void CS(uint8_t sel) {
  HAL_GPIO_WritePin(OLED_NSS_GPIO_Port, OLED_NSS_Pin, sel ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void WriteCmd(uint8_t cmd) {
  DC(OLED_CMD);
  CS(1);
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
  CS(0);
}

/* Send multiple data bytes in one CS transaction (required for page write) */
static void WriteDataBurst(const uint8_t *data, uint16_t len) {
  DC(OLED_DATA);
  CS(1);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)data, len, 100);
  CS(0);
}

void OLED_Init(void) {
  /* RST pulse - GPIO initialized by MX_GPIO_Init */
  HAL_GPIO_WritePin(OLED_NSS_GPIO_Port, OLED_NSS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(10);

  WriteCmd(0xAE); /* display off */
  WriteCmd(0xA8); WriteCmd(0x3F); /* multiplex 64-1 */
  WriteCmd(0xD3); WriteCmd(0x00); /* display offset */
  WriteCmd(0x40); /* display start line 0 */
  WriteCmd(0xA0); /* segment remap: 0xA0=SEG0->col0, 0xA1=flipped (was 0xA1) */
  WriteCmd(0xC8); /* COM scan reversed (COM63->COM0): 0xC0=normal, 0xC8=flipped (was 0xC0) */
  WriteCmd(0xDA); WriteCmd(0x12); /* COM config */
  WriteCmd(0x81); WriteCmd(0x7F); /* contrast */
  WriteCmd(0xD5); WriteCmd(0x80); /* clock divide */
  WriteCmd(0xD9); WriteCmd(0x22); /* precharge */
  WriteCmd(0xDB); WriteCmd(0x20); /* Vcomh */
  WriteCmd(0x8D); WriteCmd(0x14); /* charge pump enable (required for some 1.3" modules) */
  WriteCmd(0x20); WriteCmd(0x02); /* page addressing mode */
  WriteCmd(0xA6); /* normal display */
  WriteCmd(0xAF); /* display on */

  OLED_Clear();
  OLED_Refresh();
}

void OLED_Clear(void) {
  for (uint16_t i = 0; i < sizeof(s_fb); i++)
    s_fb[i] = 0x00;
}

void OLED_ClearRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  if (x0 > x1) { uint8_t t = x0; x0 = x1; x1 = t; }
  if (y0 > y1) { uint8_t t = y0; y0 = y1; y1 = t; }
  for (uint8_t y = y0; y <= y1 && y < OLED_HEIGHT; y++)
    for (uint8_t x = x0; x <= x1 && x < OLED_WIDTH; x++)
      OLED_SetPixel(x, y, false);
}

void OLED_Refresh(void) {
  for (uint8_t page = 0; page < OLED_PAGES; page++) {
    WriteCmd(0xB0 + page);
    WriteCmd(0x02);
    WriteCmd(0x10);
    WriteDataBurst(&s_fb[page * OLED_WIDTH], OLED_WIDTH);
  }
}

void OLED_SetPixel(uint8_t x, uint8_t y, bool on) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
  x = OLED_WIDTH - 1 - x;  /* X position flip (display hardware offset) */
  uint8_t page = y / 8;
  uint8_t bit = y % 8;
  uint16_t idx = page * OLED_WIDTH + x;
  if (on)
    s_fb[idx] |=  (1u << bit);
  else
    s_fb[idx] &= ~(1u << bit);
}

void OLED_DrawLineH(uint8_t x0, uint8_t x1, uint8_t y) {
  if (y >= OLED_HEIGHT) return;
  if (x0 > x1) { uint8_t t = x0; x0 = x1; x1 = t; }
  for (uint8_t x = x0; x <= x1 && x < OLED_WIDTH; x++)
    OLED_SetPixel(x, y, true);
}

void OLED_DrawLineV(uint8_t x, uint8_t y0, uint8_t y1) {
  if (x >= OLED_WIDTH) return;
  if (y0 > y1) { uint8_t t = y0; y0 = y1; y1 = t; }
  for (uint8_t y = y0; y <= y1 && y < OLED_HEIGHT; y++)
    OLED_SetPixel(x, y, true);
}

void OLED_DrawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  OLED_DrawLineH(x0, x1, y0);
  OLED_DrawLineH(x0, x1, y1);
  OLED_DrawLineV(x0, y0, y1);
  OLED_DrawLineV(x1, y0, y1);
}

void OLED_DrawFillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  if (x0 > x1) { uint8_t t = x0; x0 = x1; x1 = t; }
  if (y0 > y1) { uint8_t t = y0; y0 = y1; y1 = t; }
  for (uint8_t y = y0; y <= y1; y++)
    OLED_DrawLineH(x0, x1, y);
}

void OLED_DrawCircle(uint8_t cx, uint8_t cy, uint8_t r, bool filled) {
  if (r == 0) { OLED_SetPixel(cx, cy, true); return; }
  int16_t x0 = (int16_t)cx - (int16_t)r;
  int16_t y0 = (int16_t)cy - (int16_t)r;
  int16_t x1 = (int16_t)cx + (int16_t)r;
  int16_t y1 = (int16_t)cy + (int16_t)r;
  for (int16_t y = y0; y <= y1; y++) {
    for (int16_t x = x0; x <= x1; x++) {
      int16_t dx = x - (int16_t)cx, dy = y - (int16_t)cy;
      uint32_t d2 = (uint32_t)(dx * dx + dy * dy);
      uint32_t r2 = (uint32_t)r * (uint32_t)r;
      if (filled) {
        if (d2 <= r2 && x >= 0 && x < OLED_WIDTH && y >= 0 && y < OLED_HEIGHT)
          OLED_SetPixel((uint8_t)x, (uint8_t)y, true);
      } else {
        /* 1-pixel ring: (r-0.5)^2 <= d2 <= (r+0.5)^2 approx */
        uint32_t rmin = r > 0 ? (uint32_t)r * (uint32_t)(r - 1) + 1u : 0;
        uint32_t rmax = (uint32_t)r * (uint32_t)(r + 1);
        if (d2 >= rmin && d2 <= rmax && x >= 0 && x < OLED_WIDTH && y >= 0 && y < OLED_HEIGHT)
          OLED_SetPixel((uint8_t)x, (uint8_t)y, true);
      }
    }
  }
}

/* Minimal 6x8 font (ASCII 0x20-0x7F subset - only used chars in test) */
static const uint8_t font6x8[][6] = {
  { 0x00,0x00,0x00,0x00,0x00,0x00 }, /* space */
  { 0x00,0x00,0x5F,0x00,0x00,0x00 }, /* ! */
  { 0x00,0x07,0x00,0x07,0x00,0x00 }, /* " */
  { 0x14,0x7F,0x14,0x7F,0x14,0x00 }, /* # */
  { 0x24,0x2A,0x7F,0x2A,0x12,0x00 }, /* $ */
  { 0x23,0x13,0x08,0x64,0x62,0x00 }, /* % */
  { 0x36,0x49,0x56,0x20,0x50,0x00 }, /* & */
  { 0x00,0x08,0x07,0x03,0x00,0x00 }, /* ' */
  { 0x00,0x1C,0x22,0x41,0x00,0x00 }, /* ( */
  { 0x00,0x41,0x22,0x1C,0x00,0x00 }, /* ) */
  { 0x2A,0x1C,0x7F,0x1C,0x2A,0x00 }, /* * */
  { 0x08,0x08,0x3E,0x08,0x08,0x00 }, /* + */
  { 0x00,0x80,0x70,0x30,0x00,0x00 }, /* , */
  { 0x08,0x08,0x08,0x08,0x08,0x00 }, /* - */
  { 0x00,0x00,0x60,0x60,0x00,0x00 }, /* . */
  { 0x20,0x10,0x08,0x04,0x02,0x00 }, /* / */
  { 0x3E,0x51,0x49,0x45,0x3E,0x00 }, /* 0 */
  { 0x00,0x42,0x7F,0x40,0x00,0x00 }, /* 1 */
  { 0x72,0x49,0x49,0x49,0x46,0x00 }, /* 2 */
  { 0x21,0x41,0x49,0x4D,0x33,0x00 }, /* 3 */
  { 0x18,0x14,0x12,0x7F,0x10,0x00 }, /* 4 */
  { 0x27,0x45,0x45,0x45,0x39,0x00 }, /* 5 */
  { 0x3C,0x4A,0x49,0x49,0x31,0x00 }, /* 6 */
  { 0x41,0x21,0x11,0x09,0x07,0x00 }, /* 7 */
  { 0x36,0x49,0x49,0x49,0x36,0x00 }, /* 8 */
  { 0x46,0x49,0x49,0x29,0x1E,0x00 }, /* 9 */
  { 0x00,0x36,0x36,0x00,0x00,0x00 }, /* : */
  { 0x00,0x56,0x36,0x00,0x00,0x00 }, /* ; */
  { 0x08,0x14,0x22,0x41,0x00,0x00 }, /* < */
  { 0x14,0x14,0x14,0x14,0x14,0x00 }, /* = */
  { 0x00,0x41,0x22,0x14,0x08,0x00 }, /* > */
  { 0x02,0x01,0x59,0x09,0x06,0x00 }, /* ? */
  { 0x3E,0x41,0x5D,0x59,0x4E,0x00 }, /* @ */
  { 0x7C,0x12,0x11,0x12,0x7C,0x00 }, /* A */
  { 0x7F,0x49,0x49,0x49,0x36,0x00 }, /* B */
  { 0x3E,0x41,0x41,0x41,0x22,0x00 }, /* C */
  { 0x7F,0x41,0x41,0x41,0x3E,0x00 }, /* D */
  { 0x7F,0x49,0x49,0x49,0x41,0x00 }, /* E */
  { 0x7F,0x09,0x09,0x09,0x01,0x00 }, /* F */
  { 0x3E,0x41,0x41,0x51,0x73,0x00 }, /* G */
  { 0x7F,0x08,0x08,0x08,0x7F,0x00 }, /* H */
  { 0x00,0x41,0x7F,0x41,0x00,0x00 }, /* I */
  { 0x20,0x40,0x41,0x3F,0x01,0x00 }, /* J */
  { 0x7F,0x08,0x14,0x22,0x41,0x00 }, /* K */
  { 0x7F,0x40,0x40,0x40,0x40,0x00 }, /* L */
  { 0x7F,0x02,0x0C,0x02,0x7F,0x00 }, /* M */
  { 0x7F,0x04,0x08,0x10,0x7F,0x00 }, /* N */
  { 0x3E,0x41,0x41,0x41,0x3E,0x00 }, /* O */
  { 0x7F,0x09,0x09,0x09,0x06,0x00 }, /* P */
  { 0x3E,0x41,0x51,0x21,0x5E,0x00 }, /* Q */
  { 0x7F,0x09,0x19,0x29,0x46,0x00 }, /* R */
  { 0x26,0x49,0x49,0x49,0x32,0x00 }, /* S */
  { 0x03,0x01,0x7F,0x01,0x03,0x00 }, /* T */
  { 0x3F,0x40,0x40,0x40,0x3F,0x00 }, /* U */
  { 0x1F,0x20,0x40,0x20,0x1F,0x00 }, /* V */
  { 0x3F,0x40,0x38,0x40,0x3F,0x00 }, /* W */
  { 0x63,0x14,0x08,0x14,0x63,0x00 }, /* X */
  { 0x03,0x04,0x78,0x04,0x03,0x00 }, /* Y */
  { 0x61,0x59,0x49,0x4D,0x43,0x00 }, /* Z */
};

#define FONT_W 6
#define FONT_H 8

void OLED_DrawChar(uint8_t x, uint8_t y, char c) {
  if (c < 0x20 || c > 0x5A) c = ' ';
  const uint8_t *glyph = font6x8[c - 0x20];
  for (uint8_t col = 0; col < FONT_W; col++) {
    uint8_t bits = glyph[col];
    for (uint8_t row = 0; row < FONT_H; row++) {
      if (y + row < OLED_HEIGHT && x + col < OLED_WIDTH)
        OLED_SetPixel(x + col, y + row, (bits >> row) & 1);
    }
  }
}

void OLED_DrawString(uint8_t x, uint8_t y, const char *s) {
  while (*s && x < OLED_WIDTH) {
    OLED_DrawChar(x, y, *s++);
    /* fill 1px gap between chars with background */
    for (uint8_t row = 0; row < FONT_H && y + row < OLED_HEIGHT; row++)
      OLED_SetPixel(x + FONT_W, y + row, false);
    x += FONT_W + 1;
  }
}
