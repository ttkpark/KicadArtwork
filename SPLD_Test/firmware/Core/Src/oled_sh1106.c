/**
 * @file oled_sh1106.c
 * @brief SH1106 128x64 SPI OLED driver - software SPI on PA4,5,7,8,9
 */
#include "oled_sh1106.h"
#include "main.h"

/* Pin definitions: PA4=NSS, PA5=CLK, PA7=MOSI, PA8=RST, PA9=DC */
#define OLED_NSS_PORT   GPIOA
#define OLED_NSS_PIN    GPIO_PIN_4
#define OLED_CLK_PORT   GPIOA
#define OLED_CLK_PIN    GPIO_PIN_5
#define OLED_MOSI_PORT  GPIOA
#define OLED_MOSI_PIN   GPIO_PIN_7
#define OLED_RST_PORT  GPIOA
#define OLED_RST_PIN   GPIO_PIN_8
#define OLED_DC_PORT   GPIOA
#define OLED_DC_PIN    GPIO_PIN_9

#define OLED_CMD   0
#define OLED_DATA  1

/* Framebuffer: 128 x (64/8) = 1024 bytes */
static uint8_t s_fb[OLED_WIDTH * OLED_PAGES];

static void DC(uint8_t cmd) {
  HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, cmd ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void CS(uint8_t sel) {
  HAL_GPIO_WritePin(OLED_NSS_PORT, OLED_NSS_PIN, sel ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void SPI_Byte(uint8_t byte) {
  for (int8_t i = 7; i >= 0; i--) {
    HAL_GPIO_WritePin(OLED_CLK_PORT, OLED_CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_MOSI_PORT, OLED_MOSI_PIN, (byte & (1u << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_CLK_PORT, OLED_CLK_PIN, GPIO_PIN_SET);
  }
}

static void WriteCmd(uint8_t cmd) {
  DC(OLED_CMD);
  CS(1);
  SPI_Byte(cmd);
  CS(0);
}

static void WriteData(uint8_t dat) {
  DC(OLED_DATA);
  CS(1);
  SPI_Byte(dat);
  CS(0);
}

void OLED_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin   = OLED_NSS_PIN | OLED_CLK_PIN | OLED_MOSI_PIN | OLED_RST_PIN | OLED_DC_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(OLED_NSS_PORT, OLED_NSS_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(10);

  WriteCmd(0xAE); /* display off */
  WriteCmd(0x02); /* lower column start (SH1106 132-col: offset 2 for 128) */
  WriteCmd(0x10); /* higher column start */
  WriteCmd(0x40); /* display start line 0 */
  WriteCmd(0xA1); /* segment remap (column 131 -> 0) */
  WriteCmd(0xC8); /* reverse scan (COM63 -> COM0) */
  WriteCmd(0xA6); /* normal display */
  WriteCmd(0xA8); WriteCmd(0x3F); /* multiplex 64-1 */
  WriteCmd(0xD3); WriteCmd(0x00); /* display offset */
  WriteCmd(0xD5); WriteCmd(0x80); /* clock divide */
  WriteCmd(0xD9); WriteCmd(0x22); /* precharge */
  WriteCmd(0xDA); WriteCmd(0x12); /* COM config */
  WriteCmd(0xDB); WriteCmd(0x20); /* Vcomh */
  WriteCmd(0x81); WriteCmd(0x7F); /* contrast */
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
    for (uint8_t col = 0; col < OLED_WIDTH; col++)
      WriteData(s_fb[page * OLED_WIDTH + col]);
  }
}

void OLED_SetPixel(uint8_t x, uint8_t y, bool on) {
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
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
    x += FONT_W + 1;
  }
}
