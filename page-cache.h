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
                size_t id;
                uint8_t contents;
                Block* next_block;
                Block* prev_block;
            };

            class LRUCache {
                public:
                    void init();
                    bool read(void* buffer, size_t offset);
                    void put(void* contents, size_t offset);
                private:
                    Block* first = nullptr;
                    Block* last = nullptr;
                    uint16_t size= 0;
            };
        }
    }
}
