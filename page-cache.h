/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/util/lock.h>
#include <infos/util/string.h>
#include <arch/x86/pio.h>
#include <infos/kernel/kernel.h>
#include <infos/drivers/ata/ata-device.h>
#include <infos/drivers/ata/ata-controller.h>
#include <infos/drivers/block/block-device.h>

namespace infos
{
    namespace drivers
    {
        namespace ata
        {
            struct Block {
                size_t block_num;
                uint8_t* content;
                Block* next;
                Block* prev;
            };

            class Cache {
                public:
                    void initialise();

                    bool retrieve(void* buffer, size_t offset);

                    void insert(void* contents, size_t offset);

                private:
                    Block* first_elem = NULL;
                    Block* last_elem = NULL;
                    uint16_t cache_size = 0;
            };
        }
    }
}
