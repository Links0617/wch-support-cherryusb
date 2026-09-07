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
#include "board.h"

/* @function declaration */
void cdc_acm_init(uint8_t busid, uintptr_t reg_base);
void cdc_acm_data_send_with_dtr_test(uint8_t busid);

int main(void)
{
    board_init();

    cdc_acm_init(0, REG_BASE0);

    while (1)
    {
        cdc_acm_data_send_with_dtr_test(0);
    }
}
