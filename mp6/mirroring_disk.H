#ifndef _MIRRORING_DISK_H_
#define _MIRRORING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
//externs

extern Scheduler* SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "blocking_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* M i r r o r i n g  D i s k  */
/*--------------------------------------------------------------------------*/

class RAID_1_DISK : public SimpleDisk {
DISK_ID disk_id;
unsigned int size;

//void issue_operation(DISK_OPERATION _op, unsigned long _block_no);
public:
	
     // virtual bool is_ready();
     /* Return true if disk is ready to transfer data from/to disk, false otherwise. */

    // virtual bool is_ready();
     virtual void wait_until_ready();
     /* Is called after each read/write operation to check whether the disk is
        ready to start transfering the data from/to the disk. */
     /* In SimpleDisk, this function simply loops until is_ready() returns TRUE.
        In more sophisticated disk implementations, the thread may give up the CPU
        and return to check later. */

	
   RAID_1_DISK(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a MirroringDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */

   /* DISK OPERATIONS */
   //issue read operation low level
    void readFromRAID_1(unsigned long _block_no, unsigned char * _buf);
    
    //issue write operation low level
 void writeToRAID_1(unsigned long _block_no, unsigned char * _buf);

    virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif
