/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "mirroring_disk.H"
#include "simple_disk.H"
#include "scheduler.H"
#include "mirroring_disk.H"
extern Scheduler* SYSTEM_SCHEDULER;

/// RAID_1_DISK pointer
RAID_1_DISK* raid_1;


// ts_lock pointer

TestAndSetLock* ts_lock;


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size), head{nullptr}, tail{nullptr} {
  Console::puts("Constructed Blocking Disk\n");
  
  raid_1 = new RAID_1_DISK(DISK_ID::DEPENDENT, _size);
  ts_lock = new TestAndSetLock();

	  
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


void BlockingDisk::wait_until_ready(){
	
	addToQueue(Thread::CurrentThread());

	while(!is_ready()){
	// queue the waiting threads
	SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
	// yield the cpu
	Console::puts("Yielding CPU for I/O\n");
	SYSTEM_SCHEDULER->yield();
	}

	// dequeue 
	removeFromQueue(Thread::CurrentThread());
	
}
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  //SimpleDisk::read(_block_no, _buf);
  
  
  //ts_lock the disk
  ts_lock->lock();
  // issue operation to mirrored disk
  raid_1->readFromRAID_1(_block_no, _buf);
  
  // issue opeeration to blovking disk
  //SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);
  
  wait_until_ready();
  
  if(is_ready()){
	   /* read data from port */
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
  }else{
	  raid_1->read(_block_no, _buf);
	 	  
}
	  Console::puts("Reading Done from Blocking DISK\n");

//unlock the ts_lock once done
ts_lock->unlock();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
 // SimpleDisk::write(_block_no, _buf);
 
  //ts_lock the disk
  ts_lock->lock();
  
  //issue operation to raid_1 disk
  raid_1->writeToRAID_1(_block_no, _buf);
  
  //issue operation to blovking disj
  issue_operation(DISK_OPERATION::WRITE, _block_no);
  
  wait_until_ready();
  
  
  if(is_ready()){
	   int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }
  }else{
	 raid_1->write(_block_no, _buf);
}

	  Console::puts("Writing Done to Blocking DISK\n");
//unlock the ts_lock once done
ts_lock->unlock();
}



TestAndSetLock::TestAndSetLock(){
	//initialie key to 0
	this->key=0;
	Console::puts("Implementing Disk Locks Using Test And Set \n");
  
}

int TestAndSetLock:: TestAndSet(int* key){
	
	// test and set the key
	int temp = *key;
	*key= 1;
	return temp;
}

void TestAndSetLock::lock(){
	//wait until the key can be acquired
	while(TestAndSet(&key));
	Console::puts("Disk is Locked\n");
}

void TestAndSetLock::unlock(){
	//unlock key by setting to 0;
	key=0;
	
	Console::puts("Disk is unlocked\n");
}


void BlockingDisk::addToQueue(Thread* thread){
		
	//basic queueing operation same as scheduling queue
diskQueue* curr= new diskQueue;
curr->thread= thread;
curr->next= NULL;

if(!head){
	head= tail=curr;
}
else{
	tail->next= curr;
	tail= curr;
}
}


void BlockingDisk::removeFromQueue(Thread* thread){
	
	//same dequeuing as scheduling yielding
	if(head==NULL)
		return;
	diskQueue* prev= NULL;
	diskQueue* curr= head;
	while(curr->thread!= thread){
		prev=curr;
		curr= curr->next;
		if(!curr)
			return;
	}
	prev->next= curr->next;
	
	
	return;
	
}
