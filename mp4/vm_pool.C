/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
#define MAX_DEFINED_REGIONS 256
/*--------------------------------------------------------------------------*/

#define KB * (0x1 << 10)
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "simple_frame_pool.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               SimpleFramePool *_frame_pool,
               PageTable     *_page_table) {    
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table->register_pool(this);

    // Use first page of vm pool to store info about regions
    VM_List = (VMArray *)base_address;
    VM_List[0].page_addr = base_address;
    VM_List[0].length = 1;
    VM_List[0].allotted = false;
    regionsAvailable = 1;
	Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned long num_pages;

    // How many pages to allocate?
    if (_size % (4 KB) == 0) {
        num_pages = (_size / (4 KB));
    } else {
        num_pages = (_size / (4 KB)) + 1; 
    }

    // check for the first available free region
    // We can reuse this region.
    int i=0;
    while(i<regionsAvailable){
    
        if (VM_List[i].allotted && VM_List[i].length >= num_pages) {
            VM_List[i].allotted = false;
            return VM_List[i].page_addr;
        }
        i++;
    }

    // If no such free region is found, assert and loop indefinitely
    if (regionsAvailable == MAX_DEFINED_REGIONS) {
        Console::puts("Number of regions maxed out\n");
        assert(false);
    }

	// Allocate new memory at the boundary of the an instance in VMList array
    VM_List[regionsAvailable].page_addr = VM_List[regionsAvailable-1].page_addr + (VM_List[regionsAvailable-1].length << 12);
    VM_List[regionsAvailable].length = num_pages;
    VM_List[regionsAvailable].allotted = false;
    regionsAvailable++;
    Console::puts("Allocated region of memory.\n");

    return VM_List[regionsAvailable-1].page_addr;
}

void VMPool::release(unsigned long _start_address) {
    unsigned long _page_addr;
    unsigned long _length;
    unsigned int _region_index;

    // Look for a matching region
    unsigned long i=0;
    while(i<regionsAvailable){
    
        if (_start_address == VM_List[i].page_addr) {
            _page_addr = VM_List[i].page_addr;
            _length = VM_List[i].length;
            _region_index = i;
            break;
        }
        i++;
    }

    // Free the regions.
    
    unsigned long j = _page_addr;
    unsigned long limit = _page_addr + (_length << 12);
    unsigned int offset = 1<<12;
    while(j<limit){
        page_table->free_page(j);
        j+=offset;
    }

    VM_List[_region_index].allotted = true;
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    // Bounds checking of _address
    if ((_address >= base_address) &&
        (_address < base_address + size)) {
        return true;
    }

    return false;
    Console::puts("Checked whether address is part of an allocated region.\n");
}

