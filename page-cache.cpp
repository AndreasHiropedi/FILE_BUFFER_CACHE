#include <infos/drivers/ata/page-cache.h>
#include <infos/drivers/ata/ata-device.h>
#include <infos/drivers/ata/ata-controller.h>
#include <infos/drivers/block/block-device.h>
#include <infos/util/lock.h>
#include <infos/util/string.h>
#include <arch/x86/pio.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::ata;
using namespace infos::drivers::block;
using namespace infos::util;
using namespace infos::arch::x86;


#define BLOCK_SIZE   512
#define MAX_SIZE   64


void LRUCache::init() 
{
    first = nullptr;
    last = nullptr;
    size = 0;
}

bool LRUCache::read(void* buffer, size_t offset) 
{
    assert(offset >= 0);
    assert(buffer);
    assert(size <= MAX_SIZE);
    assert(size >= 0);

    // Return immediately if the block is not in the cache.
    if (size == 0) 
    {
        return false;
    }

    assert(first);
    assert(last);

    // Search for block with matching ID
    Block* current_block = first;
    while(current_block && current_block->id != offset) 
    {
        current_block = current_block->next_block;
    }

    // If block not found, return false
    if (!current_block) 
    {
        return false;
    }

    // Move accessed block to front of LRU list
    if (current_block != first) 
    {
        if (current_block == last) 
        {
            last = last->prev_block;
        }
        else 
        {
            current_block->next_block->prev_block = current_block->prev_block;
        }
        current_block->prev_block->next_block = current_block->next_block;
        current_block->prev_block = nullptr;
        current_block->next_block = first;
        first->prev_block = current_block;
        first = current_block;
    }

    // Copy block contents into buffer
    memcpy(buffer, &current_block->contents, BLOCK_SIZE);
    return true;
}

void LRUCache::put(void* contents, size_t offset) 
{
    assert(contents);
    assert(offset >= 0);
    assert(size <= MAX_SIZE);
    assert(size >= 0);

    // Check if cache is full
    if (size == MAX_SIZE) 
    {
        assert(last);

        // Overwrite the last block memory with the given contents
        Block* last_block = last;
        last_block->id = offset;
        memcpy(&last_block->contents, contents, BLOCK_SIZE);

        if (MAX_SIZE > 1)
        {
            // Move last block to the front of LRU list
            last_block->prev_block->next_block = nullptr;
            last = last_block->prev_block;
            last_block->prev_block = nullptr;
            last_block->next_block = first;
            first->prev_block = last_block;
            first = last_block;
        }
    } 

    else 
    {
        // Create a new block and store it first
        Block* new_block = new Block();
        new_block->id = offset;
        new_block->contents = *(new uint8_t[BLOCK_SIZE]);
        new_block->prev_block = nullptr;
        memcpy(&new_block->contents, contents, BLOCK_SIZE);

        // If the cache is empty, new block is both first and last
        if (!first) 
        {
            first = new_block;
            last = new_block;
            new_block->next_block = nullptr;
            new_block->prev_block = nullptr;
        } 

        // Else place the new block before the first one
        else 
        {
            new_block->prev_block = nullptr;
            new_block->next_block = first;
            first->prev_block = new_block;
            first = new_block;
        }

        size++;
    }
}
