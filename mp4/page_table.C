#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"
#include "simple_frame_pool.H"

#define D_MASK 0xFFC00000

unsigned long PageTable::shared_size = 0;
PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;

SimpleFramePool * PageTable::kernel_mem_pool = NULL;
SimpleFramePool * PageTable::process_mem_pool = NULL;

void PageTable::init_paging(SimpleFramePool * _kernel_mem_pool,
                            SimpleFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{

    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
   // assert(false);
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

    unsigned long pd_frame_no;
    
    // Fetch a frame for page directory
    pd_frame_no = process_mem_pool->get_frame();
    // Compute address from the frame number
    page_directory = (unsigned long *)(pd_frame_no << 12);
    
    init_page_directory(page_directory);
    current_page_table = this;
    
    vm_pool_count = 0;
    //assert(false);
    Console::puts("Constructed Page Table object\n");
}

void PageTable::init_page_directory(unsigned long *page_directory)
{
    unsigned long address = 0;
    unsigned int i;
    unsigned long PT_FRAME_NO;
    unsigned long *page_table;

    PT_FRAME_NO = process_mem_pool->get_frame();
    page_table = (unsigned long *)(PT_FRAME_NO << 12);

    // Map the first 4 MB of memory
    for (i = 0; i < 1024; i++) {
        //page_table[i] = address | 3;
        page_table[i] = address | 3;
        address += 4096;
    }

    // Fill first entry of page directory
    page_directory[0] = (unsigned long)page_table | 3;

    // Mark remaining PD entries as not present
    for (i = 1; i < 1023; i++) {
        page_directory[i] = 0 | 2;
    }

    // Set last PDE to point to itself
    page_directory[1023] = (unsigned long)page_directory | 3;
}

void PageTable::load()
{
 write_cr3((unsigned long)page_directory);
    //assert(false);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
 write_cr0(read_cr0() | 0x80000000);
    //assert(false);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{

unsigned long err_code;
    unsigned long fault_addr;
    unsigned long flags;

    // Get fault address from CR2 and error code from _r
    fault_addr = read_cr2();
    err_code = _r->err_code;
    // Get only lower 3 bits of error code
    flags = err_code & 0x7;

    if (!(flags & (1 << 0))) {
        // Page not present
        handle_page_not_present(fault_addr);
    } else {
        if (!(flags & (1 << 1))) {
            Console::puts("Page fault on read access\n");
        } else {
            Console::puts("Page fault on write access\n");
        }

        if (!(flags & (1 << 2))) {
            Console::puts("Page fault caused when in supervisor mode\n");
        } else {
            Console::puts("Page fault caused when in user mode\n");
        }

      //  assert(false);
    }

    Console::puts("handled page fault\n");
    
}

void PageTable::handle_page_not_present(unsigned long fault_addr)
{
    unsigned long PT_ADDR;
    unsigned long frame_no;
    unsigned long FRAME_ADDR;
    unsigned long *pde;
    unsigned long *pte;
    bool is_addr_legitimate = false;
    VMPool **_vm_pool_list = current_page_table->vm_pool_list;
    unsigned long _vm_pool_count = current_page_table->vm_pool_count;

    // Check if address is legitimate in any vm pool.
    for (int i = 0; i < _vm_pool_count; i++) {
        if (_vm_pool_list[i]->is_legitimate(fault_addr)) {
            is_addr_legitimate = true;
            break;
        }
    }   

    // Throw segmentation fault if address is illegitimate/illegal
    if (!is_addr_legitimate) {
        Console::puts("Segmentation fault: Illegitimate address\n");
       // assert(false);
    }   

    pde = PDE_address(fault_addr);
    if (!(*pde & 0x1)) {
        // Page table does not exist. Create one and
		// update PDE as well.
        create_page_table(fault_addr, pde);
    }

    pte = PTE_address(fault_addr);
    if (!(*pte & 0x1)) {
        // Frame doesn't exist. Create one and update its address
        // in page table.
        frame_no = process_mem_pool->get_frame();
        FRAME_ADDR = frame_no << 12;
        *pte = FRAME_ADDR | 3;
    }
}


void PageTable::create_page_table(unsigned long fault_addr, unsigned long *pde)
{
    unsigned long PT_FRAME_NO;
    unsigned long PT_ADDR;
    unsigned long *pte;

    // Get a frame from frame pool for page table
    PT_FRAME_NO = process_mem_pool->get_frame();
    PT_ADDR = PT_FRAME_NO << 12;

    // Update PDE with physical address of page table
    *pde = PT_ADDR | 3;
    pte = PTE_address(fault_addr);

    // Fill PT entries
    for (int i = 0; i < 1024; i++) {
        *pte++ = 2;
    }
}

unsigned long * PageTable::PDE_address(unsigned long addr)
{
    unsigned long VIRTUAL_ADDR = 0xFFFFF000;
    VIRTUAL_ADDR |= ((addr >> 22) << 2);
    return (unsigned long *)VIRTUAL_ADDR;
}

unsigned long * PageTable::PTE_address(unsigned long addr)
{
    unsigned long VIRTUAL_ADDR = D_MASK;
    VIRTUAL_ADDR |= ((addr >> 22) << 12);
    VIRTUAL_ADDR |= (((addr & 0x003FF000) >> 12) << 2);
    return (unsigned long *)VIRTUAL_ADDR;
}


void PageTable::register_pool(VMPool * _vm_pool)
{
vm_pool_list[vm_pool_count] = _vm_pool;
    vm_pool_count++;
	Console::puts("registered VM pool\n");
    //assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
unsigned long *pte = PTE_address(_page_no);

    if (*pte & 0x1) {
        process_mem_pool->release_frame((*pte & 0xFFFFF000) >> 12);
        *pte = 2;
    }

    // Flush TLB
    write_cr3((unsigned long)page_directory);
	Console::puts("freed page\n");
}
