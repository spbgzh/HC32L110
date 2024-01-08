#include "base_timer.h"
#include "uart.h"

volatile uint8_t u8RxData[16] = {0x00}, u8RxFlg = 0, u8RxPos = 0;

void RxIntCallback(void) {
  u8RxData[u8RxPos++] = UART1_RxReceive();
  u8RxPos = u8RxPos % 16;
  u8RxFlg = 1;
}

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
  Gpio_InitIOExt(0, 2,
                 GpioDirIn, // Input
                 TRUE,      // Pull up
                 FALSE,     // No pull down
                 FALSE,     // No open drain
                 TRUE       // Normal driver capability
  );
  while (1) {
    if (GPIO_GetPinIn(0, 2) == TRUE) {
      Uart1_TxString("1\r\n");
    } else {
      Uart1_TxString("0\r\n");
    }
    delay1ms(200);
  }
}
