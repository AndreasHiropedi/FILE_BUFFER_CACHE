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


#define TOTAL_BLOCK_SIZE 512
#define MAXIMUM_CACHE_SIZE 64


void CacheAdv::initialise() 
{
    first_elem = NULL;
    last_elem = NULL;
    cache_size = 0;
}

bool CacheAdv::retrieve(void* buffer, size_t offset) 
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
        BlockAdv* curr_block = first_elem;
        while(curr_block && curr_block->block_num != offset) 
        {
            curr_block = curr_block->next;
        }

        // if the block is found, copy its contents into buffer
        if (curr_block) 
        {
            memcpy(buffer, curr_block->content, TOTAL_BLOCK_SIZE);
            return true;
        }

        // If the block is not found, return false
        return false;
    }

    // Return false otherwise
    return false;
}

void CacheAdv::insert(void* contents, size_t offset) 
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
        // Create a new block and store it last
        BlockAdv* new_block = new BlockAdv();
        new_block->block_num = offset;
        new_block->content = new uint8_t[TOTAL_BLOCK_SIZE];
        new_block->next = NULL;
        memcpy(new_block->content, contents, TOTAL_BLOCK_SIZE);

        // If the cache only has 2 elements, add the block at the end
        if (first_elem && cache_size == 2) 
        {
            first_elem->next = last_elem;
            BlockAdv* temp = last_elem;
            last_elem = new_block;
            new_block->prev = temp;
        } 

        // If the cache is not empty and has more than 2 elements, place the new block at the end
        else if (first_elem) 
        {
            BlockAdv* temp = last_elem;
            last_elem = new_block;
            new_block->prev = temp;
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
        // ensure the first element in the cache is defined
        assert(first_elem);

        // Overwrite the first block in memory with the given contents
        BlockAdv* first_block = first_elem;
        first_block->block_num = offset;
        memcpy(first_block->content, contents, TOTAL_BLOCK_SIZE);
        
        if (MAXIMUM_CACHE_SIZE > 1)
        {
            // Shift all blocks up by 1 position
            BlockAdv* curr_block = first_elem->next;
            while(curr_block) 
            {
                BlockAdv* temp = curr_block;
                curr_block->prev = temp;
                curr_block = curr_block->next;
            }
            // and move the newly added block to the end
            last_elem = first_elem;
            last_elem->next = NULL;
        }
    }
}
