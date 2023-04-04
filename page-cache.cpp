/* SPDX-License-Identifier: MIT */
#include <infos/util/lock.h>
#include <infos/util/string.h>
#include <arch/x86/pio.h>
#include <infos/kernel/kernel.h>
#include <infos/drivers/ata/ata-device.h>
#include <infos/drivers/ata/ata-controller.h>
#include <infos/drivers/block/block-device.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::ata;
using namespace infos::drivers::block;
using namespace infos::util;
using namespace infos::arch::x86;


#define TOTAL_BLOCKS 512
#define MAXIMUM_CACHE_SIZE 64


void Cache::initialise() 
{
    first_elem = NULL;
    last_elem = NULL;
    cache_size = 0;
}

bool Cache::retrieve(void* buffer, size_t offset) 
{
    // ensure the function parameters are correct
    assert(buffer);
    assert(offset >= 0);
    // ensure the cache size is within the correct range
    assert(cache_size <= MAXIMUM_CACHE_SIZE);
    assert(cache_size >= 0);

    // if the cache is populated
    if (cache_size > 0) 
    {
        // ensure the first and last element in the cache are defined
        assert(first_elem);
        assert(last_elem);

        // Search the cache for the block
        Block* curr_block = first_elem;
        while(curr_block && curr_block->block_num != offset) 
        {
            curr_block = curr_block->next;
        }

        // if the block is found, move it to front of LRU list
        if (curr_block) 
        {
            // If it's the last block
            if ((curr_block != first_elem) && (curr_block == last_elem))
            {
                last_elem = last_elem->prev;
            }
            // If it's not the first
            else if (curr_block != first_elem)
            {
                curr_block->next->prev = curr_block->prev;
                curr_block->prev->next = curr_block->next;
                curr_block->prev = NULL;
                curr_block->next = first_elem;
                first_elem->prev = curr_block;
                first_elem = curr_block;
            }

            // Copy the block contents into the buffer
            memcpy(buffer, curr_block->content, TOTAL_BLOCKS);
            return true;
        }

        // If the block is not found, return false
        return false;
    }

    // Return false otherwise
    return false;
}

void Cache::insert(void* contents, size_t offset) 
{
    // ensure the function parameters are correct
    assert(contents);
    assert(offset >= 0);
    // ensure the cache size is within the correct range
    assert(cache_size <= MAXIMUM_CACHE_SIZE);
    assert(cache_size >= 0);

    // if the cache is not full
    if (cache_size < MAXIMUM_CACHE_SIZE) 
    {
        // Create a new block and store it first
        Block* new_block = new Block();
        new_block->block_num = offset;
        new_block->content = new uint8_t[TOTAL_BLOCKS];
        new_block->prev = NULL;
        memcpy(new_block->content, contents, TOTAL_BLOCKS);

        // If the cache is not empty, place the new block before the first one
        if (first_elem) 
        {
            new_block->prev = NULL;
            new_block->next = first_elem;
            first_elem->prev = new_block;
            first_elem = new_block;
        } 

        // If it is empty, new block is both first and last
        else 
        {
            first_elem = new_block;
            last_elem = new_block;
            new_block->next = NULL;
            new_block->prev = NULL;
        }

        cache_size++;
    } 

    // if the cache is full
    else 
    {
        // ensure the last element in the cache is defined
        assert(last_elem);

        // Overwrite the last block in memory with the given contents
        Block* last_block = last_elem;
        last_block->block_num = offset;
        memcpy(last_block->content, contents, TOTAL_BLOCKS);

        if (MAXIMUM_CACHE_SIZE > 1)
        {
            // Move last block to the front of LRU list
            last_block->prev->next = NULL;
            last_elem = last_block->prev;
            last_block->prev = NULL;
            last_block->next = first_elem;
            first_elem->prev = last_block;
            first_elem = last_block;
        }
    }
}


