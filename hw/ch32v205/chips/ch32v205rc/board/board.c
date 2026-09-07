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
#include "ch32v205.h"

/* @function declaration */
void USBD_IRQHandler(uint8_t busid);

void board_init(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(921600);

    printf("======== Startup Information ========\r\n");
    printf("Compiled Time: %s %s\n", __DATE__, __TIME__);
    printf("RISC-V Compiler: %s\r\n", __VERSION__);
    printf("System Clock: %ld\r\n", SystemCoreClock);
    printf("=====================================\r\n\r\n");
}

void usb_dc_low_level_init(uint8_t busid)
{
    if (busid == 0)
    {
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, DISABLE);
        RCC_USBHS_PLLCmd(DISABLE);
        RCC_USBHSPLLCLKConfig(RCC_USBHSPLLSource_HSE);
        RCC_USBHSPLLReferConfig(RCC_USBHSPLLRefer_8M);
        RCC_USBHSPLLClockSourceDivConfig(RCC_USBHSPLL_IN_Div1);
        RCC_USBHS_PLLCmd(ENABLE);
        while (!(RCC->CTLR & RCC_USBHS_PLLRDY));

        /* Enable USBHS Clock */
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, ENABLE);

        /* Enable USBHS interrupt */
        NVIC_EnableIRQ(USBHS_IRQn);
    }
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    if (busid == 0)
    {
        NVIC_DisableIRQ(USBHS_IRQn);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, DISABLE);
        RCC_USBHS_PLLCmd(DISABLE);
    }
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void USBHS_IRQHandler(void)
{
    USBD_IRQHandler(0);
}
