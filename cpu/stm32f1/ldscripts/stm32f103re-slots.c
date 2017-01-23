/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup      cpu_stm32f1
 * @{
 *
 * @file
 * @brief           Memory definitions for different FW slots on
 *                  the STM32F103RE
 *
 * @author          Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

MEMORY
{
    rom (rx)        : ORIGIN = (FW_IMAGE_OFFSET + FW_METADATA_SPACE), LENGTH = FW_IMAGE_LENGTH
    ram (xrw)       : ORIGIN = 0x20000000, LENGTH = 64K
    cpuid (r)       : ORIGIN = 0x1ffff7e8, LENGTH = 12
}

_cpuid_address = ORIGIN(cpuid);

INCLUDE cortexm_base.ld