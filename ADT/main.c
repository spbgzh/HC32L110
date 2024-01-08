#include "gpio.h"
#include "adt.h"

int main(void) {
    en_adt_unit_t           enAdt;
    uint16_t                u16Period;
    en_adt_compare_t        enAdtCompareA;
    uint16_t                u16CompareA;
    stc_adt_basecnt_cfg_t   stcAdtBaseCntCfg;
    stc_adt_CHxX_port_cfg_t stcAdtTIM4ACfg;
    
    DDL_ZERO_STRUCT(stcAdtBaseCntCfg);
    DDL_ZERO_STRUCT(stcAdtTIM4ACfg);
    
    Clk_Init(ClkFreq24Mhz, ClkDiv1, ClkDiv1);
    CLK_EnablePeripheralClk(ClkPeripheralGpio); // GPIO clock is required, equal to M0P_CLOCK->PERI_CLKEN_f.GPIO = 1;

        
    Gpio_SetFunc_TIM4_CHA_P23();
    CLK_EnablePeripheralClk(ClkPeripheralAdt);
    
    enAdt = AdTIM4;
    
    stcAdtBaseCntCfg.enCntMode = AdtSawtoothMode;
    stcAdtBaseCntCfg.enCntDir = AdtCntUp;
    stcAdtBaseCntCfg.enCntClkDiv = AdtClkPClk0Div4;
    
    Adt_Init(enAdt, &stcAdtBaseCntCfg);                      //ADT载波、计数模式、时钟配置
    
    u16Period = 0xC000;
    
    Adt_SetPeriod(enAdt, u16Period);                         //周期设置
    
    enAdtCompareA = AdtCompareA;
    u16CompareA = 0x1000;
    
    Adt_SetCompareValue(enAdt, enAdtCompareA, u16CompareA);  //通用比较基准值寄存器A设置
    
    stcAdtTIM4ACfg.enCap = AdtCHxCompareOutput;
    stcAdtTIM4ACfg.bOutEn = TRUE;
    stcAdtTIM4ACfg.enPerc = AdtCHxPeriodLow;
    stcAdtTIM4ACfg.enCmpc = AdtCHxCompareHigh;
    stcAdtTIM4ACfg.enStaStp = AdtCHxStateSelSS;
    stcAdtTIM4ACfg.enStaOut = AdtCHxPortOutLow;
    Adt_CHxXPortConfig(enAdt, AdtCHxA, &stcAdtTIM4ACfg);    //端口CHA配置
    
    Adt_StartCount(enAdt);
    
    while(1);
}

