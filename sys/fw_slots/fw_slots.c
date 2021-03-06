/*
 * Copyright (c) 2016, Mark Solters <msolters@gmail.com>.
 *               2016, Francisco Acosta <francisco.acosta@inria.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * @file
 * @author      Mark Solters <msolters@gmail.com>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 */

#include <stdio.h>
#include <string.h>

#include "fw_slots.h"
#include "cpu_conf.h"
#include "irq.h"
#if !defined(FLASH_SECTORS)        /* defined by Makefile.include of the CPU */
#include "periph/flashpage.h"
#else
#include "periph/flashsector.h"
#endif
#include "hashes/sha256.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define HASH_BUF             (1024)

static uint8_t firmware_buffer[HASH_BUF];

/**
 * @brief       Read internal flash to a buffer at specific address.
 *
 * @param[in]   address - Address to be read.
 * @param[in]   count - count in bytes.
 *
 * @param[out]  data_buffer - The buffer filled with the read information.
 *
 */
static void int_flash_read(uint8_t *data_buffer, uint32_t address, uint32_t count)
{
    uint8_t *read_addres = (uint8_t*)address;
    while (count--) {
        *data_buffer++ = *read_addres++;
    }
}

int fw_slots_validate_int_slot(uint8_t fw_slot)
{
    /*
     * TODO
     */

    return 0;
}

uint32_t fw_slots_get_slot_address(uint8_t fw_slot)
{
    return get_slot_address(fw_slot);
}

uint32_t fw_slots_get_slot_page(uint8_t fw_slot)
{
    return get_slot_page(fw_slot);
}

void fw_slots_print_metadata(FW_metadata_t *metadata)
{
    printf("Firmware Size: %ld\n", metadata->size);
    printf("Firmware Version: %#x\n", metadata->version);
    printf("Firmware UUID: %#lx\n", metadata->uuid);
    printf("Firmware HASH: ");
    for (unsigned long i = 0; i < sizeof(metadata->hash); i++) {
        printf("%02x ", metadata->hash[i]);
    }
    printf("\n");
    printf("Firmware signed HASH: ");
    for (unsigned long i = 0; i < sizeof(metadata->shash); i++) {
        printf("%02x ", metadata->shash[i]);
    }
    printf("\n");
}

int fw_slots_get_int_metadata(uint8_t fw_slot_page, FW_metadata_t *fw_metadata)
{
    uint32_t fw_address;

#if !defined(FLASH_SECTORS)
    fw_address = fw_slot_page * FLASHPAGE_SIZE + CPU_FLASH_BASE;
#else
    fw_address = (uint32_t)flashsector_addr(fw_slot_page);
#endif

    DEBUG("[fw_slots] Getting internal metadata on page %d at address %#lx\n",
            fw_slot_page, fw_address);
    int_flash_read((uint8_t*)fw_metadata, fw_address, sizeof(FW_metadata_t));

    return 0;
}

int fw_slots_get_slot_metadata(uint8_t fw_slot, FW_metadata_t *fw_slot_metadata)
{
    /*
     * TODO
     */

    return 0;
}

int fw_slots_get_int_slot_metadata(uint8_t fw_slot, FW_metadata_t *fw_slot_metadata)
{
    uint32_t page;

    DEBUG("[fw_slots] Getting internal FW slot %d metadata\n", fw_slot);
    if (fw_slot > MAX_FW_SLOTS || fw_slot == 0) {
        printf("[fw_slots] FW slot not valid, should be <= %d and > 0\n",
                MAX_FW_SLOTS);
        return -1;
    }

    page = fw_slots_get_slot_page(fw_slot);

    return fw_slots_get_int_metadata(page, fw_slot_metadata);
}

int fw_slots_overwrite_int_slot_metadata(uint8_t fw_slot, FW_metadata_t *fw_slot_metadata)
{
    /*
     * TODO
     */
    return 0;
}



int fw_slots_overwrite_slot_metadata(uint8_t fw_slot, FW_metadata_t *fw_slot_metadata)
{
    /*
     * TODO
     */
    return 0;
}

int fw_slots_backup_golden_image(void)
{
    /*
     * TODO
     */
    return 0;
}

int fw_slots_verify_int_slot(uint8_t fw_slot)
{
    FW_metadata_t fw_metadata;
    uint32_t fw_image_address;
    uint32_t address;
    uint16_t rest;
    sha256_context_t sha256_ctx;
    uint8_t hash[SHA256_DIGEST_LENGTH];
    int parts = 0, i = 0;

    /* Determine the external flash address corresponding to the FW slot */
    if (fw_slot > MAX_FW_SLOTS || fw_slot == 0) {
        printf("[fw_slots] FW slot not valid, should be <= %d and > 0\n",
                MAX_FW_SLOTS);
        return -1;
    }

    /* Read the metadata of the corresponding FW slot */
    fw_image_address = fw_slots_get_slot_address(fw_slot);

    if (fw_slots_get_int_slot_metadata(fw_slot, &fw_metadata) == 0) {
        fw_slots_print_metadata(&fw_metadata);
    } else {
        printf("[fw_slots] ERROR cannot get slot metadata.\n");
    }

    printf("Verifying slot %d at 0x%lx \n", fw_slot, fw_image_address);

    address = fw_image_address;
    address += FW_METADATA_SPACE;
    sha256_init(&sha256_ctx);

    parts = fw_metadata.size / sizeof(firmware_buffer);
    rest = fw_metadata.size % sizeof(firmware_buffer);

    while (parts) {
        int_flash_read(firmware_buffer, address, sizeof(firmware_buffer));
        sha256_update(&sha256_ctx, firmware_buffer, sizeof(firmware_buffer));
        address += sizeof(firmware_buffer);
        parts--;
    }

    int_flash_read(firmware_buffer, address, rest);
    sha256_update(&sha256_ctx, firmware_buffer, rest);
    sha256_final(&sha256_ctx, hash);

    for (i = 0; i < sizeof(hash); i++) {
        if (hash[i] != fw_metadata.hash[i]) {
            printf("[fw_slots] hash verification failed!\n");
            return -1;
        }
    }

    return 0;
}

int fw_slots_validate_metadata(FW_metadata_t *metadata)
{
    /* Is the FW slot erased?
     * First, we check to see if every byte in the metadata is 0xFF.
     * If this is the case, this metadata is "erased" and therefore we assume
     * the FW slot to be empty.
     */
    int erased = 1;
    uint8_t *metadata_ptr = (uint8_t*)metadata;
    int b = FW_METADATA_LENGTH;

    while (b--) {
        if (*metadata_ptr++ != 0xff) {
            /* We encountered a non-erased byte.
             * There's some non-trivial data here.
             */
            erased = 0;
            break;
        }
    }

    /* If the FW slot is erased, it's not valid!  No more work to do here. */
    if (erased) {
        return -1;
    }

    /* If we get this far, all metadata bytes were cleared (0xff) */
    return 0;
}

int fw_slots_find_matching_int_slot(uint16_t version)
{
    int matching_slot = -1; /* Assume there is no matching FW slot. */

    /* Iterate through each of the FW slots. */
    for (int slot = 1; slot <= MAX_FW_SLOTS; slot++) {

        /* Get the metadata of the current FW slot. */
        FW_metadata_t fw_slot_metadata;
        if(fw_slots_get_int_slot_metadata(slot, &fw_slot_metadata) == 0) {
            fw_slots_print_metadata(&fw_slot_metadata);
        } else {
            printf("[fw_slots] ERROR cannot get slot metadata.\n");
        }

        /* Is this slot empty? If yes, skip. */
        if (fw_slots_validate_metadata(&fw_slot_metadata) == false) {
            continue;
        }

        /* Does this slot's FW version match our search parameter? */
        if (fw_slot_metadata.version == version) {
            matching_slot = slot;
            break;
        }
    }

    if (matching_slot == -1) {
        printf("[fw_slots] No FW slot matches Firmware v%i\n", version);
    } else {
        printf("[fw_slots] FW slot #%i matches Firmware v%i\n", matching_slot,
                version);
    }

    return matching_slot;
}

int fw_slots_find_empty_int_slot(void)
{
    /* Iterate through each of the MAX_FW_SLOTS internal slots. */
    for (int slot = 1; slot <= MAX_FW_SLOTS; slot++) {

        /* Get the metadata of the current FW slot. */
        FW_metadata_t fw_slot_metadata;

        if(fw_slots_get_int_slot_metadata(slot, &fw_slot_metadata) == 0) {
            fw_slots_print_metadata(&fw_slot_metadata);
        } else {
            printf("[fw_slots] ERROR cannot get slot metadata.\n");
        }

        /* Is this slot invalid? If yes, let's treat it as empty. */
        if (fw_slots_validate_metadata(&fw_slot_metadata) == false) {
            return slot;
        }
    }

    printf("[fw_slots] Could not find any empty FW slots!"
            "\nSearching for oldest FW slot...\n");
    /*
     * If execution goes this far, no empty slot was found. Now, we look for
     * the oldest FW slot instead.
     */
    return fw_slots_find_oldest_int_image();
}

int fw_slots_find_oldest_int_image(void)
{
    /* The oldest firmware should be the v0 */
    int oldest_fw_slot = 1;
    uint16_t oldest_firmware_version = 0;

    /* Iterate through each of the MAX_FW_SLOTS internal slots. */
    for (int slot = 1; slot <= MAX_FW_SLOTS; slot++) {
        /* Get the metadata of the current FW slot. */
        FW_metadata_t fw_slot_metadata;

        if(fw_slots_get_int_slot_metadata(slot, &fw_slot_metadata) == 0) {
            fw_slots_print_metadata(&fw_slot_metadata);
        } else {
            printf("[fw_slots] ERROR cannot get slot metadata.\n");
        }

        /* Is this slot populated? If not, skip. */
        if (fw_slots_validate_metadata(&fw_slot_metadata) == false) {
            continue;
        }

        /* Is this the oldest image we've found thus far? */
        if (oldest_firmware_version) {
            if (fw_slot_metadata.version < oldest_firmware_version) {
                oldest_fw_slot = slot;
                oldest_firmware_version = fw_slot_metadata.version;
            }
        } else {
            oldest_fw_slot = slot;
            oldest_firmware_version = fw_slot_metadata.version;
        }
    }

    printf("[fw_slots] Oldest FW slot: #%d; Firmware v%d\n", oldest_fw_slot,
            oldest_firmware_version);

    return oldest_fw_slot;
}

int fw_slots_find_newest_int_image(void)
{
    /* At first, we only assume knowledge of version v0 */
    int newest_fw_slot = 0;
    uint16_t newest_firmware_version = 0;

    /* Iterate through each of the MAX_FW_SLOTS. */
    for (int slot = 1; slot <= MAX_FW_SLOTS ; slot++) {
        /* Get the metadata of the current FW slot. */
        FW_metadata_t fw_slot_metadata;

        if(fw_slots_get_int_slot_metadata(slot, &fw_slot_metadata) == 0) {
            fw_slots_print_metadata(&fw_slot_metadata);
        } else {
            printf("[fw_slots] ERROR cannot get slot metadata.\n");
        }

        /* Is this slot populated? If not, skip. */
        if (fw_slots_validate_metadata(&fw_slot_metadata) == -1) {
            continue;
        }

        /* Is this the newest non-Golden Image image we've found thus far? */
        if ( fw_slot_metadata.version > newest_firmware_version ) {
            newest_fw_slot = slot;
            newest_firmware_version = fw_slot_metadata.version;
        }
    }

    printf("Newest FW slot: #%d; Firmware v%d\n", newest_fw_slot,
            newest_firmware_version);

    return newest_fw_slot;
}

int fw_slots_erase_int_image(uint8_t fw_slot)
{
    /* Get page address of the fw_slot in internal flash */
    uint32_t fw_image_base_address;
    /* Get the page where the fw_slot is located */
    uint8_t slot_page;

    if (fw_slot > MAX_FW_SLOTS || fw_slot == 0) {
        printf("[fw_slots] FW slot not valid, should be <= %d and > 0\n",
                MAX_FW_SLOTS);
        return -1;
    }

    fw_image_base_address = fw_slots_get_slot_address(fw_slot);

#if !defined(FLASH_SECTORS)
    printf("[fw_slots] Erasing FW slot %u [%#lx, %#lx]...\n", fw_slot,
            fw_image_base_address,
            fw_image_base_address + (FW_SLOT_PAGES * FLASHPAGE_SIZE) - 1);
#else
    printf("[fw_slots] Erasing FW slot %u [%#lx, %#lx]...\n", fw_slot,
            fw_image_base_address,
            fw_image_base_address + get_slot_size(fw_slot) - 1);
#endif

    slot_page = fw_slots_get_slot_page(fw_slot);

    /* Erase each page in the FW internal slot! */
#if !defined(FLASH_SECTORS)
    for (int page = slot_page; page < slot_page + FW_SLOT_PAGES; page++) {
        DEBUG("[fw_slots] Erasing page %d\n", page);
        flashpage_write(page, NULL);
    }
#else
    int slot_last_page = flashsector_sector((void *)(fw_image_base_address + get_slot_size(fw_slot) - 1));
    for (int sector = slot_page; sector < slot_last_page; sector++) {
        DEBUG("[fw_slots] Erasing sector %d\n", page);
        flashsector_write(sector, NULL, 0);
    }
#endif


    printf("[fw_slots] Erase successful\n");

    return 0;
}

int fw_slots_update_firmware(uint8_t fw_slot)
{
    /*
     * TODO
     */
    return 0;
}

int fw_slots_store_fw_data( uint32_t ext_address, uint8_t *data, size_t data_length )
{
    /*
     * TODO
     */
    return 0;
}

/*
 * _estack pointer needed to reset PSP position
 */
extern uint32_t _estack;

void fw_slots_jump_to_image(uint32_t destination_address)
{
    if (destination_address) {
        /*
         * Only add the metadata length offset if destination_address is NOT 0!
         * (Jumping to 0x0 is used to reboot the device)
         */
        destination_address += FW_METADATA_SPACE;
    }

    /* Disable IRQ */
    (void)irq_disable();

    /* Move PSP to the end of the stack */
    __set_PSP((uint32_t)&_estack);

    /* Move to the second pointer on VTOR (reset_handler_default) */
    destination_address += VTOR_RESET_HANDLER;

    /* Load the destination address */
    __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address));
    /* Make sure the Thumb State bit is set. */
    __asm("ORR R0, #1");
    /* Branch execution */
    __asm("BX R0");
}
