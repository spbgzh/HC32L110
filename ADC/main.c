#include "adc.h"
#include "base_timer.h"
#include "bsp_printf.h"
#include "gpio.h"
#include <stdint.h>
static uint32_t u32AdcResultAcc;
volatile uint8_t u8AdcIrqFlag = 0;

void AdcContIrqCallback(void) {
  // 中断标志位判断和清零，已在库函数中处理Adc_IRQHandler();

  Adc_GetAccResult(&u32AdcResultAcc);
  printf("u32AdcResultAcc = %ld\n", u32AdcResultAcc);
  u8AdcIrqFlag = 1;
  Adc_ClrAccResult();

  // Adc_ClrContIrqState();
}
void AdcRegIrqCallback(void) {
  // 中断标志位判断和清零，已在库函数中处理Adc_IRQHandler();

  // Adc_ClrRegIrqState();
}

void AdcHhtIrqCallback(void) {
  // 中断标志位判断和清零，已在库函数中处理Adc_IRQHandler();

  // Adc_ClrHhtIrqState();
}

void AdcLltIrqCallback(void) {
  // 中断标志位判断和清零，已在库函数中处理Adc_IRQHandler();

  // Adc_ClrLltIrqState();
}

int main(void) {
  stc_adc_cfg_t stcAdcCfg;
  stc_adc_cont_cfg_t stcAdcContCfg;
  stc_adc_irq_t stcAdcIrq;
  stc_adc_irq_calbakfn_pt_t stcAdcIrqCalbaks;

  DDL_ZERO_STRUCT(stcAdcCfg);
  DDL_ZERO_STRUCT(stcAdcContCfg);
  DDL_ZERO_STRUCT(stcAdcIrq);
  DDL_ZERO_STRUCT(stcAdcIrqCalbaks);

  /**
   * Set PCLK = HCLK = Clock source to 24MHz
   */
  Clk_Init(ClkFreq24Mhz, ClkDiv1, ClkDiv1);

  // Enable peripheral clock
  CLK_EnablePeripheralClk(ClkPeripheralBaseTim);
  CLK_EnablePeripheralClk(ClkPeripheralGpio);
  CLK_EnablePeripheralClk(ClkPeripheralAdcBgr);

  // Enable printf
  Bsp_PrinfInit(115200);

  Gpio_SetAnalog(2, 4, TRUE);

  // ADC配置
  ADC_Enable();
  M0P_BGR->CR_f.BGR_EN = 0x1u; // BGR必须使能
  M0P_BGR->CR_f.TS_EN = 0x0u;  // 内置温度传感器，视使用需求
  delay100us(1);

  stcAdcCfg.enAdcOpMode = AdcContMode;          // 连续采样模式
  stcAdcCfg.enAdcClkSel = AdcClkSysTDiv1;       // PCLK
  stcAdcCfg.enAdcSampTimeSel = AdcSampTime8Clk; // 8个采样时钟
  // stcAdcCfg.enAdcRefVolSel = RefVolSelInBgr2p5;
  // //参考电压:内部2.5V(avdd>3V,SPS<=200kHz)
  stcAdcCfg.enAdcRefVolSel = RefVolSelAVDD; // 参考电压:AVDD
  // stcAdcCfg.bAdcInBufEn = TRUE;
  stcAdcCfg.bAdcInBufEn = FALSE; // 电压跟随器如果使能，SPS采样速率 <=200K
  stcAdcCfg.enAdcTrig0Sel = AdcTrigDisable; // ADC转换自动触发设置
  stcAdcCfg.enAdcTrig1Sel = AdcTrigDisable;
  Adc_Init(&stcAdcCfg);

  stcAdcIrq.bAdcIrq = TRUE; // 转换完成中断函数入口配置使能
  stcAdcIrq.bAdcRegCmp = FALSE;
  stcAdcIrq.bAdcHhtCmp = FALSE;
  stcAdcIrq.bAdcLltCmp = FALSE;
  stcAdcIrqCalbaks.pfnAdcContIrq = AdcContIrqCallback; // 转换完成中断入口函数
  stcAdcIrqCalbaks.pfnAdcRegIrq = AdcRegIrqCallback;
  stcAdcIrqCalbaks.pfnAdcHhtIrq = AdcHhtIrqCallback;
  stcAdcIrqCalbaks.pfnAdcLltIrq = AdcLltIrqCallback;
  Adc_ConfigIrq(&stcAdcIrq, &stcAdcIrqCalbaks); // 转换中断入口函数配置
  Adc_EnableIrq();                              // 中断使能
  Adc_CmpCfg(&stcAdcIrq); // 结果比较中断使能/禁止配置

  stcAdcContCfg.enAdcContModeCh = AdcExInputCH0; // 通道0 P24
  stcAdcContCfg.u8AdcSampCnt = 0x09u; // P24 连续累加次数(次数 = 0x09+1)
  stcAdcContCfg.bAdcResultAccEn = TRUE; // 累加使能
  Adc_ConfigContMode(&stcAdcCfg, &stcAdcContCfg);

  while (1) {
    ADC_Start(); // ADC开始转换
    while (0 == u8AdcIrqFlag); // 等待中断标志位
    u8AdcIrqFlag = 0;
    delay1ms(1);
  }
}
