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
#include "ch32v4x7.h"

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
    switch (busid)
    {
    case 0:
        if ((RCC->CTLR & RCC_USBHSPLLRDY) == 0)
        {
            RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS1, DISABLE);
            RCC->CTLR &= ~RCC_USBHSPLLON;
            if (RCC->CTLR & RCC_HSEON)
            {
                RCC_USBHSPLLCLKConfig(RCC_USBHSPLLCLKSource_HSE);
                RCC_USBHSPLLReferConfig(RCC_USBHSPLLCKREFCLK_25M);
            }
            else
            {
                RCC_USBHSPLLCLKConfig(RCC_USBHSPLLCLKSource_HSI);
                RCC_USBHSPLLReferConfig(RCC_USBHSPLLCKREFCLK_20M);
            }
            RCC->CTLR |= RCC_USBHSPLLON;
            while (!(RCC->CTLR & RCC_USBHSPLLRDY));
        }

        RCC_UTMI1cmd(ENABLE);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS1, ENABLE);
        NVIC_EnableIRQ(USBHS1_IRQn);
        break;

    case 1:
        if ((RCC->CTLR & RCC_USBHSPLLRDY) == 0)
        {
            RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS2, DISABLE);
            RCC->CTLR &= ~RCC_USBHSPLLON;
            if (RCC->CTLR & RCC_HSEON)
            {
                RCC_USBHSPLLCLKConfig(RCC_USBHSPLLCLKSource_HSE);
                RCC_USBHSPLLReferConfig(RCC_USBHSPLLCKREFCLK_25M);
            }
            else
            {
                RCC_USBHSPLLCLKConfig(RCC_USBHSPLLCLKSource_HSI);
                RCC_USBHSPLLReferConfig(RCC_USBHSPLLCKREFCLK_20M);
            }
            RCC->CTLR |= RCC_USBHSPLLON;
            while (!(RCC->CTLR & RCC_USBHSPLLRDY));
        }

        RCC_UTMI2cmd(ENABLE);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS2, ENABLE);
        NVIC_EnableIRQ(USBHS2_IRQn);
        break;
    }
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    switch (busid)
    {
    case 0:
        NVIC_DisableIRQ(USBHS1_IRQn);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS1, DISABLE);
        RCC_UTMI1cmd(DISABLE);
        break;

    case 1:
        NVIC_DisableIRQ(USBHS2_IRQn);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS2, DISABLE);
        RCC_UTMI2cmd(DISABLE);
        break;
    }
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void USBHS1_IRQHandler(void)
{
    USBD_IRQHandler(0);
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void USBHS2_IRQHandler(void)
{
    USBD_IRQHandler(1);
}
