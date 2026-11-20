/**
 * @file main.c
 * @author Links (lhd@wch.cn)
 * @brief Main file.
 * @version 0.1
 * @date 2026-09-05
 *
 * @copyright Copyright (c) 2026
 *
 */

/* @include */
#include <stdint.h>

/* @function declaration */
void cdc_acm_init(uint8_t busid, uintptr_t reg_base);

int main(void)
{
    cdc_acm_init(0, USBD_REG_BASE0);
}
