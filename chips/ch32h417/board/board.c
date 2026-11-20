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
#include <rtthread.h>

#include "usbh_core.h"

#include "ch32h417.h"

/* @define */
#ifndef USB_IP
#define USB_IP USB_IP_USBHS
#endif

#define USB_IP_USBFS       0
#define USB_IP_USBHS       1
#define USB_IP_USBSS       2

#define GET_INT_SP()       asm("csrrw sp,mscratch,sp")
#define FREE_INT_SP()      asm("csrrw sp,mscratch,sp")

#define CONSOLE_MB_SIZE    64

#define USBSS_PHY_CFG_CR   (*((__IO uint32_t *)0x400341f8))
#define USBSS_PHY_CFG_DAT  (*((__IO uint32_t *)0x400341fc))

#define LMP_HP             0
#define LMP_SUBTYPE_MASK   (0xf << 5)
#define LMP_SET_LINK_FUNC  (0x1 << 5)
#define LMP_U2_INACT_TOUT  (0x2 << 5)
#define LMP_VENDOR_TEST    (0x3 << 5)
#define LMP_PORT_CAP       (0x4 << 5)
#define LMP_PORT_CFG       (0x5 << 5)
#define LMP_PORT_CFG_RES   (0x6 << 5)
#define LMP_LINK_SPEED     (1 << 9)
#define NUM_HP_BUF         (4 << 0)
#define DOWN_STREAM        (1 << 16)
#define UP_STREAM          (2 << 16)
#define USBSS_U1_EXIT_TIME (1 << 9)

/* @enum */
typedef enum
{
    WORK_MODE_IDLE,
    WORK_MODE_USBH,
    WORK_MODE_USBD,
} work_mode_t;

/* @global */
static uint8_t chip;
static struct rt_mailbox console_mb;
static work_mode_t work_mode[CONFIG_USB_MAX_BUS];
static rt_ubase_t console_mb_pool[CONSOLE_MB_SIZE];

/* @function declaration */
static void usart_config(void);
static uint32_t systick_config(rt_uint32_t ticks);
static void usb_low_level_init(uint8_t busid, work_mode_t mode);
static void usb_low_level_deinit(uint8_t busid);
void USBD_IRQHandler(uint8_t busid);
void USBH_IRQHandler(uint8_t busid);

void rt_hw_board_init()
{
    /* System Clock Configuration */
    SystemInit();
    SystemAndCoreClockUpdate();

    /* USART Configuration */
    usart_config();

    /* System Tick Configuration */
    systick_config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    extern char _end;
    extern char _heap_end;
    rt_system_heap_init(&_end, &_heap_end);
#endif

    rt_mb_init(&console_mb, "console", console_mb_pool, CONSOLE_MB_SIZE, RT_IPC_FLAG_PRIO);

    for (int i = 0; i < sizeof(work_mode) / sizeof(work_mode[0]); i++)
    {
        work_mode[i] = WORK_MODE_IDLE;
    }

    chip = (DBGMCU_GetCHIPID() >> 4) & 0x0F;
}

static void usart_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_USART1 | RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF7);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 921600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    NVIC_SetPriority(USART1_IRQn, 0xF0);
    NVIC_EnableIRQ(USART1_IRQn);
}

static uint32_t systick_config(rt_uint32_t ticks)
{
    NVIC_SetPriority(SysTick0_IRQn, 0xF0);
    NVIC_SetPriority(Software_IRQn, 0xF0);
    NVIC_EnableIRQ(SysTick0_IRQn);
    NVIC_EnableIRQ(Software_IRQn);
    SysTick0->CTLR = 0;
    SysTick0->ISR = 0;
    SysTick0->CNT = 0;
    SysTick0->CMP = ticks - 1;
    SysTick0->CTLR = 0x0F;
    return 0;
}

void rt_hw_console_output(const char *str)
{
    rt_size_t size = rt_strlen(str);

    for (rt_size_t i = 0; i < size; i++)
    {
        if (str[i] == '\n')
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
            USART_SendData(USART1, '\r');
        }

        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, str[i]);
    }
}

char rt_hw_console_getchar(void)
{
    rt_ubase_t ch;

    if (rt_mb_recv(&console_mb, &ch, RT_WAITING_FOREVER) != RT_EOK)
    {
        return -1;
    }

    return (char)ch;
}

static uint32_t USBSS_PHY_Cfg(uint8_t port_num, uint8_t addr, uint16_t data)
{
    if (port_num == 0)
    {
        USBSS_PHY_CFG_CR = (1 << 23) | (addr << 16) | data;
        USBSS_PHY_CFG_DAT = 0x01;
        return (USBSS_PHY_CFG_DAT);
    }
    else
    {
        return (0);
    }
}

static void usb_low_level_init(uint8_t busid, work_mode_t mode)
{
    switch (busid)
    {
    case 0:
#if (USB_IP == USB_IP_USBFS)
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
        {
            RCC_USBHS_PLLCmd(DISABLE);
            RCC_USBHSPLLCLKConfig((RCC->CTLR & RCC_HSERDY) ? RCC_USBHSPLLSource_HSE : RCC_USBHSPLLSource_HSI);
            RCC_USBHSPLLReferConfig(RCC_USBHSPLLRefer_25M);
            RCC_USBHSPLLClockSourceDivConfig(RCC_USBHSPLL_IN_Div1);
            RCC_USBHS_PLLCmd(ENABLE);
            while (!(RCC->CTLR & RCC_USBHS_PLLRDY));
        }
        RCC_USBFSCLKConfig(RCC_USBFSCLKSource_USBHSPLL);
        RCC_USBFS48ClockSourceDivConfig(RCC_USBFS_Div10);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_OTG_FS, ENABLE);

        NVIC_EnableIRQ(USBFS_IRQn);
#endif

#if (USB_IP == USB_IP_USBHS)
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
        {
            RCC_USBHS_PLLCmd(DISABLE);
            RCC_USBHSPLLCLKConfig(RCC_USBHSPLLSource_HSE);
            RCC_USBHSPLLReferConfig(RCC_USBHSPLLRefer_25M);
            RCC_USBHSPLLClockSourceDivConfig(RCC_USBHSPLL_IN_Div1);
            RCC_USBHS_PLLCmd(ENABLE);
            while (!(RCC->CTLR & RCC_USBHS_PLLRDY));
        }

        RCC_UTMIcmd(ENABLE);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, ENABLE);

        NVIC_EnableIRQ(USBHS_IRQn);
#endif

#if (USB_IP == USB_IP_USBSS)
        RCC->CTLR |= (uint32_t)RCC_USBSS_PLLON;
        while ((RCC->CTLR & (uint32_t)RCC_USBSS_PLLRDY) != (uint32_t)RCC_USBSS_PLLRDY);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBSS, ENABLE);
        RCC_PIPECmd(ENABLE);
        RCC_UTMIcmd(ENABLE);
        RCC_USBSS_PLLCmd(ENABLE);

        /* LINK layer configuration */
        USBSSD->LINK_CFG = LINK_RX_EQ_EN | LINK_TX_DEEMPH_MASK | LINK_PHY_RESET;
        USBSSD->LINK_CTRL = LINK_P2_MODE | LINK_GO_DISABLED;
        USBSSD->LINK_CFG = LINK_RX_EQ_EN | LINK_TX_DEEMPH_MASK | LINK_LTSSM_MODE | LINK_TOUT_MODE;
#ifdef DEF_UP_U1_EN /* disable U1 */
        USBSSD->LINK_CFG |= LINK_U1_PING_EN;
#endif
        USBSSD->LINK_LPM_CR |= LINK_LPM_EN;
        USBSSD->LINK_CFG |= LINK_RX_TERM_EN;

        USBSSD->LINK_U1_EXIT_CFG |= USBSS_U1_EXIT_TIME;

        USBSSD->LINK_INT_CTRL = LINK_IE_TX_LMP | LINK_IE_RX_LMP | LINK_IE_RX_LMP_TOUT | LINK_IE_STATE_CHG |
                                LINK_IE_WARM_RST | LINK_IE_TERM_PRES;

        if (chip >= 3)
        {
            USBSSD->LINK_INT_CTRL |= LINK_IE_RX_SET_FC;
        }

        USBSSD->LINK_CTRL = LINK_P2_MODE;
        USBSSD->LINK_U1_WKUP_TMR = 120;
        USBSSD->LINK_U1_WKUP_FILTER = 50;
        USBSSD->LINK_U2_WKUP_FILTER = 0;
        USBSSD->LINK_U3_WKUP_FILTER = 0;
        USBSSD->USB_CONTROL |= USBSS_FORCE_RST;
        USBSSD->USB_STATUS = USBSS_UIF_TRANSFER;
        USBSSD->USB_CONTROL = USBSS_UIE_TRANSFER | USBSS_UDIE_SETUP | USBSS_UDIE_STATUS | USBSS_DMA_EN |
                              USBSS_SETUP_FLOW;
#if (DEF_FUN_IF_TEST_EN == 0x01)
        USBSSD->LINK_CFG |= LINK_U1_ALLOW;
        USBSSD->LINK_CFG |= LINK_U2_ALLOW;
        USBSS_Dev_Info.u1_enable = ENABLE;
        USBSS_Dev_Info.u2_enable = ENABLE;
        USBSSD->LINK_CFG |= LINK_COMPLIANCE_EN;
#endif

        /* USBSS PHY configuration */
        USBSS_PHY_Cfg(0, 0x03, 0x7c12);
        USBSS_PHY_Cfg(0, 0x0D, 0x79AA);
        USBSS_PHY_Cfg(0, 0x15, 0x4430);
        USBSS_PHY_Cfg(0, 0x13, 0x0010);
        USBSS_PHY_Cfg(0, 0x11, 0x0501);
        (*((__IO uint32_t *)0x5003C018)) = 0xB0054000;

        NVIC_EnableIRQ(USBSS_IRQn);
        NVIC_EnableIRQ(USBSS_LINK_IRQn);
#endif

        work_mode[0] = mode;
        break;
    }
}

static void usb_low_level_deinit(uint8_t busid)
{
    switch (busid)
    {
    case 0:
#if (USB_IP == USB_IP_USBFS)
        NVIC_DisableIRQ(USBFS_IRQn);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_OTG_FS, DISABLE);
        RCC_USBHS_PLLCmd(DISABLE);
#endif

#if (USB_IP == USB_IP_USBHS)
        NVIC_DisableIRQ(USBHS_IRQn);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBHS, DISABLE);
        RCC_UTMIcmd(DISABLE);
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBHS)
        {
            RCC_USBHS_PLLCmd(DISABLE);
        }
#endif

#if (USB_IP == USB_IP_USBSS)
        NVIC_DisableIRQ(USBSS_LINK_IRQn);
        NVIC_DisableIRQ(USBSS_IRQn);
        USBSSD->LINK_CFG |= LINK_PHY_RESET | U3_LINK_RESET;
        Delay_Us(100);
        USBSSD->LINK_CFG &= ~(LINK_PHY_RESET | U3_LINK_RESET);
        RCC_HBPeriphClockCmd(RCC_HBPeriph_USBSS, DISABLE);
        RCC_UTMIcmd(DISABLE);
        RCC_PIPECmd(DISABLE);
        if ((RCC->PLLCFGR & RCC_SYSPLL_SEL) != RCC_SYSPLL_USBSS)
        {
            RCC_USBSS_PLLCmd(DISABLE);
        }
#endif

        work_mode[0] = WORK_MODE_IDLE;
        break;
    }
}

void usb_dc_low_level_init(uint8_t busid)
{
    usb_low_level_init(busid, WORK_MODE_USBD);
}

void usb_dc_low_level_deinit(uint8_t busid)
{
    usb_low_level_deinit(busid);
}

void usb_hc_low_level_init(struct usbh_bus *bus)
{
    usb_low_level_init(bus->busid, WORK_MODE_USBH);
}

void usb_hc_low_level_deinit(struct usbh_bus *bus)
{
    usb_low_level_deinit(bus->busid);
}

__attribute__((interrupt())) void SysTick0_Handler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    SysTick0->ISR = 0;
    rt_tick_increase();

    rt_interrupt_leave();

    FREE_INT_SP();
}

__attribute__((interrupt())) void USART1_IRQHandler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        rt_mb_send(&console_mb, (rt_ubase_t)(USART_ReceiveData(USART1) & 0xFF));
    }

    rt_interrupt_leave();

    FREE_INT_SP();
}

#if (USB_IP == USB_IP_USBFS)
__attribute__((interrupt())) void USBFS_IRQHandler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    if (work_mode[0] == WORK_MODE_USBD)
    {
        USBD_IRQHandler(0);
    }
    else if (work_mode[0] == WORK_MODE_USBH)
    {
        USBH_IRQHandler(0);
    }

    rt_interrupt_leave();

    FREE_INT_SP();
}
#endif

#if (USB_IP == USB_IP_USBHS)
__attribute__((interrupt())) void USBHS_IRQHandler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    if (work_mode[0] == WORK_MODE_USBD)
    {
        USBD_IRQHandler(0);
    }
    else if (work_mode[0] == WORK_MODE_USBH)
    {
        USBH_IRQHandler(0);
    }

    rt_interrupt_leave();

    FREE_INT_SP();
}
#endif

#if (USB_IP == USB_IP_USBSS)
__attribute__((interrupt())) void USBSS_IRQHandler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    if (work_mode[0] == WORK_MODE_USBD)
    {
        USBD_IRQHandler(0);
    }
    else if (work_mode[0] == WORK_MODE_USBH)
    {
        USBH_IRQHandler(0);
    }

    rt_interrupt_leave();

    FREE_INT_SP();
}

__attribute__((interrupt())) void USBSS_LINK_IRQHandler(void)
{
    GET_INT_SP();

    rt_interrupt_enter();

    static uint8_t link_low_power = 0;
    uint32_t link_state;
    uint32_t link_int;
    uint32_t link_lpm_r_data0 = 0;

    link_state = USBSSH->LINK_STATUS & LINK_STATE_MASK;
    link_int = USBSSH->LINK_INT_FLAG;

    if (chip >= 3)
    {
        if (link_int & LINK_IF_RX_SET_FC)
        {
            USBSSH->LINK_INT_FLAG = LINK_IF_RX_SET_FC;
            if (USBSSH->LINK_LMP_PORT_CAP & FORCE_PM)
            {
                USBSSH->LINK_CFG |= LINK_U1_ALLOW;
                USBSSH->LINK_CFG |= LINK_U2_ALLOW;
            }
            else
            {
                USBSSH->LINK_CFG &= ~LINK_U1_ALLOW;
                USBSSH->LINK_CFG &= ~LINK_U2_ALLOW;
            }
        }
    }

    if (link_int & LINK_IF_STATE_CHG)
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_STATE_CHG;

        if (link_state == LINK_STATE_RXDET)
        {
            // USBSS_Dev_Info.u1_enable = DISABLE;
            // USBSS_Dev_Info.u2_enable = DISABLE;
        }
        else if (link_state == LINK_STATE_RECOVERY)
        {
            if (link_low_power)
            {
                link_low_power = 0;
                Delay_Us(100);
                USBSS_PHY_Cfg(0, 0x12, 0x67c8);
            }
        }
        else if (link_state == LINK_STATE_DISABLE)
        {
            USBSSH->LINK_CTRL &= ~LINK_GO_DISABLED;

            // if (USB_Enum_Status == UNINIT)
            // {
            //     USB_Enum_Status = U3_INI_FRIST;
            //     USB_Timer_Start(ENABLE);
            // }
            // else if (USB_Enum_Status == U3_INIT_SECOND)
            // {
            //     USB_Enum_Status = U2_INIT;
            //     USBHS_Device_Init(DISABLE);
            //     USB_Timer_Start(ENABLE);
            // }
            // else if (USB_Enum_Status == U2U3_SUCC)
            // {
            //     USB_Enum_Status = UNINIT;
            //     USBHS_Device_Init(ENABLE);
            // }
        }
        else if (link_state == LINK_STATE_U3)
        {
            link_low_power = 0x01;
            USBSS_PHY_Cfg(0, 0x12, 0x67c8 & (~(1 << 9)));
        }
        else if (link_state == LINK_STATE_U1)
        {
            link_low_power = 0x01;
            USBSS_PHY_Cfg(0, 0x12, 0x67c8 & (~(1 << 9)));
        }
        else if (link_state == LINK_STATE_U2)
        {
            link_low_power = 0x01;
            USBSS_PHY_Cfg(0, 0x12, 0x67c8 & (~(1 << 9)));
        }
        else if (link_state == LINK_STATE_HOTRST)
        {
            // USBSS_Dev_Info.u1_enable = DISABLE;
            // USBSS_Dev_Info.u2_enable = DISABLE;
            // USBSS_Reset_Init(ENABLE);
            // USBSS_DevEnumStatus = 0;
            void usbss_link_reset(uint8_t busid);
            usbss_link_reset(0);
            USBSSH->LINK_CTRL &= ~LINK_HOT_RESET; // HOT RESET end(device mode)
        }
        else if (link_state == LINK_STATE_INACTIVE)
        {
            // USB_Enum_Status = U3_INI_FRIST;
            // USBHS_Device_Init(ENABLE);
        }
        else if (link_state == LINK_STATE_POLLING)
        {
        }
        else if (link_state == LINK_STATE_U0)
        {
        }
    }
    else if (link_int & LINK_IF_TERM_PRES)
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_TERM_PRES;
        // USBSS_DevEnumStatus = 0;
        if (USBSSH->LINK_STATUS & LINK_RX_TERM_PRES)
        {
        }
    }
    else if (link_int & LINK_IF_RX_LMP_TOUT) // port config err
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_RX_LMP_TOUT;
        USBSSH->LINK_CTRL |= LINK_GO_DISABLED; // upstream
        USBSSH->LINK_CTRL |= LINK_GO_RX_DET; // upstream
    }
    else if (link_int & LINK_IF_TX_LMP)
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_TX_LMP;
        USBSSH->LINK_LMP_TX_DATA0 = LMP_LINK_SPEED | LMP_PORT_CAP | LMP_HP;

        if (USBSSH->LINK_CFG & LINK_DOWN_MODE)
        {
            USBSSH->LINK_LMP_TX_DATA1 = DOWN_STREAM | NUM_HP_BUF;
        }
        else
        {
            USBSSH->LINK_LMP_TX_DATA1 = UP_STREAM | NUM_HP_BUF;
        }
        USBSSH->LINK_LMP_TX_DATA2 = 0x0;
    }
    else if (link_int & LINK_IF_RX_LMP)
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_RX_LMP; // clear int flag
        link_lpm_r_data0 = USBSSH->LINK_LMP_RX_DATA0;

        if (USBSSH->LINK_CFG & LINK_DOWN_MODE) // HOST MODE
        {
            if ((link_lpm_r_data0 & LMP_SUBTYPE_MASK) == LMP_PORT_CAP) // RX PORT_CAP
            {
                /* LMP, TX PORT_CFG & RX PORT_CFG_RES */
                USBSSH->LINK_LMP_TX_DATA0 = LMP_LINK_SPEED | LMP_PORT_CFG | LMP_HP;
                USBSSH->LINK_LMP_TX_DATA1 = 0x0;
                USBSSH->LINK_LMP_TX_DATA2 = 0x0;
            }
            else if ((link_lpm_r_data0 & LMP_SUBTYPE_MASK) == LMP_PORT_CFG_RES) // RX PORT_CFG_RES
            {
                USBSSH->LINK_LMP_PORT_CAP |= LINK_LMP_TX_CAP_VLD; // clear timer(20us timeout)
            }
        }
        else // UPSTREAM
        {
            if (((link_lpm_r_data0 & LMP_SUBTYPE_MASK) == LMP_PORT_CFG) &&
                (USBSSH->LINK_LMP_PORT_CAP & LINK_LMP_RX_CAP_VLD)) // device RX PORT_CFG, return PORT_CFG_RES
            {
                USBSSH->LINK_LMP_TX_DATA0 = LMP_LINK_SPEED | LMP_PORT_CFG_RES | LMP_HP;
                USBSSH->LINK_LMP_TX_DATA1 = 0x0;
                USBSSH->LINK_LMP_TX_DATA2 = 0x0;
                USBSSH->LINK_LMP_PORT_CAP |= LINK_LMP_TX_CAP_VLD;
                // USB_Enum_Status = U2U3_SUCC;
                // USB_Timer_Start(DISABLE);
                // USBHS_Device_Init(DISABLE);
            }
            else if ((link_lpm_r_data0 & LMP_SUBTYPE_MASK) == LMP_U2_INACT_TOUT)
            {
                USBSSH->LINK_U2_INACT_TIMER = (link_lpm_r_data0 >> 9) & 0xff;
            }
            else if ((link_lpm_r_data0 & LMP_SUBTYPE_MASK) == LMP_SET_LINK_FUNC)
            {
                if (chip < 3)
                {
                    if (USBSSH->LINK_LMP_RX_DATA0 & (0x02 << 9))
                    {
                        USBSSD->LINK_CFG |= LINK_U1_ALLOW;
                        USBSSD->LINK_CFG |= LINK_U2_ALLOW;
                        // USBSS_Dev_Info.u1_enable = ENABLE;
                        // USBSS_Dev_Info.u2_enable = ENABLE;
                    }
                    else
                    {
                        USBSSD->LINK_CFG &= ~LINK_U1_ALLOW;
                        USBSSD->LINK_CFG &= ~LINK_U2_ALLOW;
                        // USBSS_Dev_Info.u1_enable = DISABLE;
                        // USBSS_Dev_Info.u2_enable = DISABLE;
                    }
                }
            }
        }
    }
    else if (link_int & LINK_IF_WARM_RST)
    {
        USBSSH->LINK_INT_FLAG = LINK_IF_WARM_RST;
        if (USBSSH->LINK_STATUS & LINK_RX_WARM_RST)
        {
            // USBSS_DevEnumStatus = 0;
            // USBSS_Reset_Init(ENABLE); // WARM RESET Init
            void usbss_link_reset(uint8_t busid);
            usbss_link_reset(0);
            USBSSH->LINK_CTRL |= LINK_GO_DISABLED;
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            USBSSH->LINK_CTRL &= ~LINK_GO_DISABLED;
        }
    }

    rt_interrupt_leave();

    FREE_INT_SP();
}
#endif
