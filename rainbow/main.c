#include "gpio.h"
void __nop(void) { __asm__("nop"); }

void RGB_LED_Write1(void);
void RGB_LED_Write0(void);
void RGB_LED_Write_24_Byte(uint32_t rgb);
void SET_RGB(uint32_t rgb);
void RGB_LED_Reset(void);
uint32_t my_rainbow(uint8_t brightness, uint32_t value);
uint32_t ws2812_led[18];
#define RGB_LED_HIGH                                                           \
  *((volatile uint32_t *)((uint32_t)&M0P_GPIO->P0OUT)) |= (0x2);
#define RGB_LED_LOW                                                            \
  *((volatile uint32_t *)((uint32_t)&M0P_GPIO->P0OUT)) &= (0xFFFFFFFD);
int main(void) {

  /**
   * Set PCLK = HCLK = Clock source to 24MHz
   */
  Clk_Init(ClkFreq24Mhz, ClkDiv1, ClkDiv1);
  // Enable peripheral clock
  CLK_EnablePeripheralClk(
      ClkPeripheralGpio); // GPIO clock is required, equal to
                          // M0P_CLOCK->PERI_CLKEN_f.GPIO = 1;
  Gpio_InitIOExt(0, 1, GpioDirOut, FALSE, FALSE, FALSE, FALSE);
  uint32_t x = 0;
  while (1) {
    x++;

    for (int i = 0; i < 18; i++) {
      ws2812_led[i] = my_rainbow(255, x);
    }

    RGB_LED_Reset();
    delay1ms(200);

    for (int i = 0; i < 18; i++) {
      SET_RGB(ws2812_led[i]);
    }
  }
}

void RGB_LED_Write0(void) {
  RGB_LED_HIGH;
  __nop();
  __nop();
  RGB_LED_LOW;
  __nop();
}

void RGB_LED_Write1(void) {
  RGB_LED_HIGH;
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  __nop();
  RGB_LED_LOW;
}

void RGB_LED_Reset(void) {
  RGB_LED_LOW;
  delay100us(3);
}

void RGB_LED_Write_24_Byte(uint32_t rgb) {
  uint8_t i;
  uint32_t tmp = rgb;

  for (i = 0; i < 24; i++) {
    if (tmp & 0x80000) {
      RGB_LED_Write1();
    } else {
      RGB_LED_Write0();
    }

    tmp <<= 1;
  }
}

void SET_RGB(uint32_t rgb) {
  uint8_t r = (rgb & 0xff0000) >> 16;
  uint8_t g = (rgb & 0xff00) >> 8;
  uint8_t b = rgb & 0xff;

  uint32_t res = g << 8;
  res = res << 8;
  res |= r << 8 | b;

  RGB_LED_Write_24_Byte(res);
}

uint32_t my_rainbow(uint8_t brightness, uint32_t value) {
  uint8_t red = 0; // Red is the top 5 bits of a 16 bit colour value
  uint8_t green =
      0;            // Green is the middle 6 bits, but only top 5 bits used here
  uint8_t blue = 0; // Blue is the bottom 5 bits
  uint8_t sector = 0;
  uint8_t amplit = 0;
  uint8_t tmp = value % (brightness * 6);
  uint32_t res = 0;
  sector = tmp >> 5;
  amplit = tmp & brightness;
  switch (sector) {
  case 0:
    red = brightness;
    green = amplit; // Green ramps up
    blue = 0;
    break;

  case 1:
    red = brightness - amplit; // Red ramps down
    green = brightness;
    blue = 0;
    break;

  case 2:
    red = 0;
    green = brightness;
    blue = amplit; // Blue ramps up
    break;

  case 3:
    red = 0;
    green = brightness - amplit; // Green ramps down
    blue = brightness;
    break;

  case 4:
    red = amplit; // Red ramps up
    green = 0;
    blue = brightness;
    break;

  case 5:
    red = brightness;
    green = 0;
    blue = brightness - amplit; // Blue ramps down
    break;
  }
  res = red << 8;
  res = res << 8;
  res |= green << 8 | blue;
  return res;
}
