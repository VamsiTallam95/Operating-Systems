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

Scheduler::Scheduler(): head{nullptr}, tail{nullptr} {

  //assert(false);
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {

	// Interrupt handling for Bonus
	if(Machine::interrupts_enabled())
		Machine::disable_interrupts();
	
	// Get the thread next inn queue and make it head
	if(!head)
		return;
	FIFO* current= head;
	Thread* currentThread= head->thread;
	head= head->next;
	if(!head){
		tail==NULL;
	}
	
	// dispatch the thread now pointed by head
	delete(current);
	Thread::dispatch_to(currentThread);
		
  //assert(false);
}

void Scheduler::resume(Thread * _thread) {

	// BONUS: PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	     Machine::disable_interrupts();
	// add the thread to queue and continue  execution as per the queue
	 FIFO* newEntry= new struct FIFO;
	newEntry->thread= _thread;
	newEntry->next= NULL;
	if(head==NULL){
		head= newEntry;
		tail= newEntry;
		
	}
	else{
		// adding at the end of the list
		
		tail->next= newEntry;
		tail= newEntry;
	}
	
	
}

void Scheduler::add(Thread * _thread) {
	// adding is same as resume hence functionality same as resume
	
	// and BONUS  is implicit in the resume functionality
	resume(_thread);
 // assert(false);
}

void Scheduler::terminate(Thread * _thread) {
	//  PROPER HANDLING OF INTERRUPTS
	// Check if the interrupts are enabled to disable interrupts in order to allow this thread to resume
	if(Machine::interrupts_enabled())
	Machine::disable_interrupts();
	
	if(!head)
	return;
	struct FIFO* current= head;
	struct FIFO*  previous= NULL;
	while(current->thread!=_thread){
	previous = current;
	current= current->next;
	if(!current)
		return;
	}

	previous->next= current->next;
	delete(current);

	return;	
}

