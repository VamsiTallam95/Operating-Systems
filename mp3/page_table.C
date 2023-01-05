#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

#define PT_MASK 0xFFFFFC00
#define D_MASK 0xFFC00000

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
SimpleFramePool * PageTable::kernel_mem_pool = NULL;
SimpleFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(SimpleFramePool * _kernel_mem_pool,
                            SimpleFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    unsigned long pd_frame_no;
    
    // Fetch a frame for page directory
    pd_frame_no = kernel_mem_pool->get_frame();
    // Compute address from the frame number
    page_directory = (unsigned long *)(pd_frame_no << 12);
    
    init_page_directory(page_directory);
    current_page_table = this;
    Console::puts("Constructed Page Table object\n");
}

void PageTable::init_page_directory(unsigned long *page_directory)
{
    unsigned long address = 0;
    unsigned int i;
    unsigned long pt_frame_no;
    unsigned long *page_table;

    pt_frame_no = kernel_mem_pool->get_frame();
    page_table = (unsigned long *)(pt_frame_no << 12);

    // The first 4 MB of the 32MB memory
    for (i = 0; i < 1024; i++) {
        page_table[i] = address | 3;
        address += 4096;
    }

    // Update 1st entry of page directory
    page_directory[0] = (unsigned long)page_table | 3;

    // Update the remaining pages as not present
    i=1;
    while(i < 1024){
    	page_directory[i] = page_directory[i] | 0 | 2;
    	i++;
    }
}

void PageTable::load()
{
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long err_code;
    unsigned long fault_addr;
    unsigned long flags;

    // Fetch fault address from CR2 and error code from _r
    fault_addr = read_cr2();
    err_code = _r->err_code;
    // Fetch the lower 3 bits of error code
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

        //assert(false);
    }

    Console::puts("Handled page fault\n");
}

void PageTable::handle_page_not_present(unsigned long fault_addr)
{
    unsigned long *pd_addr;
    unsigned long pt_frame_no;
    unsigned long pd_offset;
    unsigned long pt_addr;
    unsigned long pt_offset;
    unsigned long frame_no;
    unsigned long frame_addr;

    pd_addr = current_page_table->page_directory;
    pd_offset = (fault_addr & D_MASK) >> 22;
    if (!(pd_addr[pd_offset] & 0x1)) {
	    // If the page table does not exist, create page table.
        pt_addr = create_page_table(); 
        pd_addr[pd_offset] = pt_addr | 3;
    } else {
	    // if page table exists then fetch its address.
        pt_addr = pd_addr[pd_offset] & PT_MASK;
    }

    pt_offset = (fault_addr & (0x003FF000)) >> 12;
    if (!(((unsigned long *)pt_addr)[pt_offset] & 0x1)) {
	    // If frame doesn't exist, create one and update its address page table.
        frame_no = process_mem_pool->get_frame();
        frame_addr = frame_no << 12;
        ((unsigned long *)pt_addr)[pt_offset] = frame_addr | 3;
    }
}

unsigned long PageTable::create_page_table()
{
    unsigned long pt_frame_no;
    unsigned long page_table_addr;

    // Get a frame from frame pool for page table
    pt_frame_no = kernel_mem_pool->get_frame();
    page_table_addr = pt_frame_no << 12;

    // Update page table entries
    unsigned long i=0;
    while(i<1024){
    	((unsigned long *) page_table_addr)[i] = 2;
    	i++;
    }

    return page_table_addr;
}

