#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"
#include "simple_disk.H"
#include "mirroring_disk.H"

//externs

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

RAID_1_DISK::RAID_1_DISK(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	  
	  Console::puts("Constructed RAID_1_DISK\n");
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/



void RAID_1_DISK::wait_until_ready(){
	while(!is_ready()){
		//resume 
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
		//yield
		SYSTEM_SCHEDULER->yield();
	}
}

void RAID_1_DISK::readFromRAID_1(unsigned long _block_no, unsigned char * _buf) {
	
	//issue operation to disk low level
	
	// Console::puts("Reading Done from RAID_1_DISK\n");
  SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);

}
void RAID_1_DISK::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  //SimpleDisk::read(_block_no, _buf);
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x170);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
    
}

void RAID_1_DISK::writeToRAID_1(unsigned long _block_no, unsigned char * _buf) {

//issue write operation to disk ... low level
issue_operation(DISK_OPERATION::WRITE, _block_no);

	  Console::puts("Writing Done to RAID_1 DISK\n");

}
void RAID_1_DISK::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
//  SimpleDisk::write(_block_no, _buf);

/* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x170, tmpw);
  }

}
