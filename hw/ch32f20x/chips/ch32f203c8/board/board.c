/**
 * @file board.c
 * @author Links (lhd@wch.cn)
 * @brief Board functional configuration.
 * @version 0.1
 * @date 2026-09-05
 *
 * @copyright Copyright (c) 2026
 *
 */

/* @include */
#include "debug.h"

/* @function declaration */
void USBD_IRQHandler(uint8_t busid);

void board_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();
    USART_Printf_Init(921600);

    printf("======== Startup Information ========\r\n");
    printf("Compiled Time: %s %s\n", __DATE__, __TIME__);
    printf("ARM Compiler: %s\r\n", __VERSION__);
    printf("System Clock: %ld\r\n", SystemCoreClock);
    printf("=====================================\r\n\r\n");
}

void usb_dc_low_level_init(uint8_t busid)
{
    if (busid == 0)
    {
        RCC_ClocksTypeDef RCC_ClocksStatus = {0};
        RCC_GetClocksFreq(&RCC_ClocksStatus);

        if (RCC_ClocksStatus.SYSCLK_Frequency == 144000000)
        {
            RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3);
        }
        else if (RCC_ClocksStatus.SYSCLK_Frequency == 96000000)
        {
            RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div2);
        }
        else if (RCC_ClocksStatus.SYSCLK_Frequency == 48000000)
        {
            RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
        }

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBFS, ENABLE);
        NVIC_EnableIRQ(USBFS_IRQn);
    }
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    if (busid == 0)
    {
        NVIC_DisableIRQ(USBFS_IRQn);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBFS, DISABLE);
    }
}

void USBHD_IRQHandler(void)
{
    USBD_IRQHandler(0);
}
