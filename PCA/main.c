#include "pca.h"
#include "lpm.h"
#include "gpio.h"

#define THRESHOLD    0x80  // 这个值可能需要根据具体的计数调整

static volatile uint32_t u32PcaTestFlag = 0;

int main(void)
{
    stc_pca_config_t pcaConfig;
    stc_pca_capmodconfig_t pcaModConfig;
    uint8_t period = THRESHOLD / 2;  // 设置周期以生成300kHz的方波

    // 配置系统时钟为24MHz
    Clk_Init(ClkFreq24Mhz, ClkDiv1, ClkDiv1);

    // 使能PCA和GPIO的时钟
    CLK_EnablePeripheralClk(ClkPeripheralPca);
    CLK_EnablePeripheralClk(ClkPeripheralGpio);

    // 将P25设置为PCA的通道0，方向为输出
    Gpio_SetFunc_PCA_CH0_P25(0);

    // PCA配置
    pcaConfig.enCIDL = IdleGoon;
    pcaConfig.enWDTE = PCAWDTDisable;
    pcaConfig.enCPS  = PCAPCLKDiv1;  // 使用系统时钟，无分频
    Pca_Init(&pcaConfig);

    // PWM模式配置
    pcaModConfig.enECOM = ECOMEnable;
    pcaModConfig.enCAPP = CAPPDisable;
    pcaModConfig.enCAPN = CAPNDisable;
    pcaModConfig.enMAT  = MATDisable;
    pcaModConfig.enTOG  = TOGDisable;
    pcaModConfig.enPWM  = PCAPWMEnable;
    Pca_CapModConfig(Module0, &pcaModConfig);

    // 设置PWM周期
    Pca_CapDataLSet(Module0, period);
    Pca_CapDataHSet(Module0, period);
    Pca_Run();

    while (1)
    {
        // 主循环中的代码
    }
}
