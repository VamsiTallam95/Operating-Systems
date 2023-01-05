/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "simple_timer.H"

/*--------------------------------------------------------------------------*/
/* DATA  URES */
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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
head={nullptr};
tail={nullptr};
  //assert(false);
  Console::puts("Constructeed Scheduler.\n");
}

void Scheduler::yield() {

	// Proper checking of interrupts for BONUS 1
	// Disabke the interrupts if they are enabled to yield the CPU used by the current thread
	if(Machine::interrupts_enabled())
		Machine::disable_interrupts();
	
	if(head!=NULL){
	  FIFO* current= head;
	Thread* currentThread= head->thread;
	head= head->next;
	if(head==NULL){
		tail==NULL;
	}
	
	// We are deleting the entry from the FIFO queue 
	// and we are dispatching it, so that when it becomes active it gets on to the FIFO at the end
	delete(current);
	Thread::dispatch_to(currentThread);
	}
	
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	Machine::enable_interrupts();
		
	
  //assert(false);
}

void Scheduler::resume(Thread * _thread) {

	// BONUS 1: PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	     Machine::disable_interrupts();
	// add the thread to end of the queue and continue thread execution as per the queue
	
	// Create a new entry and push it into queue
	// 
	  FIFO* newEntry= new   FIFO;
	
	
	newEntry->thread= _thread;
	newEntry->next= NULL;
	
	// If head is NULL add it to the head of the queue
	
	// mark head and tail to the new entry
	if(head==NULL){
		head= newEntry;
		tail= newEntry;
		
	}
	// else add it to the end of the list
	else{
		// adding item at the end of the list
		
		tail->next= newEntry;
		tail= newEntry;
	}
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	Machine::enable_interrupts();
	
  //assert(false);
}

void Scheduler::add(Thread * _thread) {
	// adding is nothing but resuming the thread hence use the functionality same as resume
	
	// and BONUS 1 is implicit in the resume functionality
	resume(_thread);
 // assert(false);
}

void Scheduler::terminate(Thread * _thread) {
	// BONUS 1: PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	Machine::disable_interrupts();
	
	// terminating is nothing but  yielding
	yield();
	
	
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	Machine::enable_interrupts();
  //assert(false);
}



RRScheduler::RRScheduler(unsigned eoq){
	
	// RR scheduler 
	// intialise Head and tail
	head= NULL;
	tail= NULL;
	// define a simple timer as defined in kernel.c
	SimpleTimer* timer = new SimpleTimer(100/eoq);
	//register the time with the interupt handler
	InterruptHandler::register_handler(0, timer);
	Console::puts("Constructed Round Robin Scheduler\n");

}

void RRScheduler::yield() {
	// Proper checking of interrupts for BONUS 1
	// Disabke the interrupts if they are enabled to yield the CPU used by the current thread
	if(Machine::interrupts_enabled())
		Machine::disable_interrupts();
	
	if(head!=NULL){
	  FIFO* current= head;
	Thread* currentThread= head->thread;
	head= head->next;
	if(head==NULL){
		tail==NULL;
	}
	
	// We are deleting the entry from the FIFO queue 
	// and we are dispatching it, so that when it becomes active it gets on to the FIFO at the end
	delete(current);
	Thread::dispatch_to(currentThread);
	}
	
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	   Machine::enable_interrupts();
		
	
  //assert(false);
}

void RRScheduler::resume(Thread * _thread){



  // BONUS 1: PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	    Machine::disable_interrupts();
	// add the thread to end of the queue and continue thread execution as per the queue
	
	// Create a new entry and push it into queue
	// 
	  FIFO* newEntry= new   FIFO;
	
	
	newEntry->thread= _thread;
	newEntry->next= NULL;
	
	// If head is NULL add it to the head of the queue
	
	// mark head and tail to the new entry
	if(head==NULL){
		head= newEntry;
		tail= newEntry;
		
	}
	// else add it to the end of the list
	else{
		// adding item at the end of the list
		
		tail->next= newEntry;
		tail= newEntry;
	}
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	Machine::enable_interrupts();
	
  //assert(false);
  
  
}

void RRScheduler::add(Thread * _thread) {
	// adding is nothing but resuming the thread hence use the functionality same as resume
	
	// and BONUS 1 is implicit in the resume functionality
	resume(_thread);
 // assert(false);
}


void RRScheduler::terminate(Thread * _thread) {
	
	// BONUS 1: PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	Machine::disable_interrupts();
	
	// terminating is nothing but  yielding
	yield();
	
	
	
	// Properly enable interrupts once this function is done so that we can serve interrupts
	
	// FOR BONUS 1
	
	if(!Machine::interrupts_enabled())
	Machine::enable_interrupts();
  //assert(false);
}


