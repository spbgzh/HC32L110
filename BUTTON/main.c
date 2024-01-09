#include "base_timer.h"
#include "gpio.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

volatile uint8_t u8RxData[16] = {0x00}, u8RxFlg = 0, u8RxPos = 0;
typedef unsigned long millis_t;


#define BUTTON_PRESSED (GPIO_GetPinIn(0, 2) != 1)

void RxIntCallback(void) {
  u8RxData[u8RxPos++] = UART1_RxReceive();
  u8RxPos = u8RxPos % 16;
  u8RxFlg = 1;
}

volatile uint32_t u32Val = 0;
void SysTick_Handler(void) { u32Val++; }
uint32_t millis() { return u32Val; }

#define DEBOUNCETIME 5
#define CLICKTIME 1500
#define LONGTIME 5000

enum KEY_STATUS {
  NO_CLICK = 0, /* 没有动作 */
  SINGLE_CLICK, /* 单击 */
  DOUBLE_CLICK, /* 双击 */
  LONGLE_CLICK  /* 长按 */
};
static uint8_t s_state_1 = 0;
static millis_t s_startTime_1 = 0;
static millis_t s_stopTime_1 = 0;
uint8_t button_tick(void) {
  uint8_t key_status = 0;
  millis_t now = millis();
  uint8_t key = GPIO_GetPinIn(0, 2);
  if (s_state_1 == 0) {
    if (key != 1) {
      s_state_1 = 1;
      s_startTime_1 = now;
    } else
      key_status = NO_CLICK;
  } else if (s_state_1 == 1) {
    if ((!BUTTON_PRESSED) &&
        ((unsigned long)(now - s_startTime_1) < DEBOUNCETIME)) {
      s_state_1 = 0;
    } else if (!BUTTON_PRESSED) {
      s_state_1 = 2;
      s_stopTime_1 = now;
    } else if ((BUTTON_PRESSED) &&
               ((unsigned long)(now - s_startTime_1) > LONGTIME)) {
      s_state_1 = 6;
      s_stopTime_1 = now;
    }
  } else if (s_state_1 == 2) {
    if ((unsigned long)(now - s_startTime_1) > CLICKTIME) {
      key_status = SINGLE_CLICK;
      s_state_1 = 0;
    } else if ((BUTTON_PRESSED) &&
               ((unsigned long)(now - s_stopTime_1) > DEBOUNCETIME)) {
      s_state_1 = 3;
      s_startTime_1 = now;
    }
  } else if (s_state_1 == 3) {
    if ((!BUTTON_PRESSED) &&
        ((unsigned long)(now - s_startTime_1) > DEBOUNCETIME)) {
      key_status = DOUBLE_CLICK;
      s_state_1 = 0;
      s_stopTime_1 = now;
    }
  } else if (s_state_1 == 6) {
    if (!BUTTON_PRESSED) {
      s_state_1 = 0;
      s_stopTime_1 = now;
      key_status = LONGLE_CLICK;
    }
  }

  return key_status;
}
/*
void PORT0_IRQHandler(void) {
Gpio_ClearIrq(KEY_PORT, KEY_PIN);


 if (Gpio_GetIrqStat(KEY_PORT, KEY_PIN)) // if (TRUE ==
M0P_GPIO->P3STAT_f.P33)
{
  Uart1_TxString("Key pressed\r\n");

  delay1ms(10);
} else {
  Uart1_TxString("Key released\r\n");
  delay1ms(10);
}

}*/
int main(void) {

  /**
   * Set PCLK = HCLK = Clock source to 24MHz
   */
  Clk_Init(ClkFreq24Mhz, ClkDiv1, ClkDiv1);
  // Enable peripheral clock
  CLK_EnablePeripheralClk(ClkPeripheralBaseTim);
  CLK_EnablePeripheralClk(
      ClkPeripheralGpio); // GPIO clock is required, equal to
                          // M0P_CLOCK->PERI_CLKEN_f.GPIO = 1;
  CLK_EnablePeripheralClk(ClkPeripheralUart1);
  /*
  Set P01,P02 as UART1 TX,RX, or use P35,P36
      Gpio_SetFunc_UART1_TX_P35();
      Gpio_SetFunc_UART1_RX_P36();
  */
  Gpio_SetFunc_UART1_TX_P35();
  Gpio_SetFunc_UART1_RX_P36();
  // Config UART1
  Uart1_TxRx_Init(115200, RxIntCallback);

  Uart1_TxString("Please input string and return\r\n");

  stc_clk_systickcfg_t stcCfg;
  DDL_ZERO_STRUCT(stcCfg);
  stcCfg.u32LoadVal = 0xF9Fu; // 1ms

  Clk_SysTickConfig(&stcCfg);
  SysTick_Config(stcCfg.u32LoadVal);

  SysTick->VAL = 0;
  Gpio_InitIOExt(0, 2,
                 GpioDirIn, // Input
                 TRUE,      // Pull up
                 FALSE,     // No pull down
                 FALSE,     // No open drain
                 TRUE       // Normal driver capability
  );
  while (1) {
    enum KEY_STATUS button = button_tick();

    if (button == SINGLE_CLICK) {
      Uart1_TxString("SINGLE_CLICK\r\n");
    } else if (button == DOUBLE_CLICK) {
      Uart1_TxString("DOUBLE_CLICK\r\n");
    } else if (button == LONGLE_CLICK) {
      Uart1_TxString("LONGLE_CLICK\r\n");
    } else {
      // Uart1_TxString("NO_CLICK\r\n");
    }
  }
}
