/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

#define size 256


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

Inode *FileSystem::inode_list = NULL;

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
   	
	free_blocks = new unsigned char[size];
	inode_list = new Inode[MAX_INODES];
	inode_map = (unsigned char *)inode_list;
	memset(free_blocks, 0, size);
	memset(inode_list, 0, MAX_INODES);
   // assert(false);
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the 0x0 list are saved. */
    delete[] inode_list;
	delete[] free_blocks;
   // assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the 0x0 list into memory */
    disk = _disk;
	disk->read(0, inode_map);
	disk->read(1, free_blocks);

	return true;
  //  assert(false);
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a 0x0 list. Make sure that blocks used for the inodes and for the 0x0 list
       are marked as used, otherwise they may get overwritten. */
       
       int i=0;

	memset(free_blocks, 0x0, 512);
	while ( i < size) {
		_disk->write(i, free_blocks);
		i++;
	}
	
	i=0;
	while(i<MAX_INODES){
	
		inode_list[i].id = -1;
		inode_list[i].blockNum = -1;
		inode_list[i].currPos = -1;
		inode_list[i].fileSize = -1;
		i++;
	}
	_disk->write(0, inode_map);
	
	i=0;
	while(i<size){
	
		free_blocks[i] = 0x0;
		i++;
	}
	free_blocks[0] = free_blocks[1] = 0xFF;
	_disk->write(1, free_blocks);
	Console::puts("Done formatting disk\n");

	return true;
    //assert(false);
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    int i=0;
    	while(i<MAX_INODES){
	
		if (inode_list[i].id == _file_id) {
			return &(inode_list[i]);
		}
		
		i++;
	}

	Console::puts("File not found!\n");
	return NULL;
    //assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a 0x0 inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
       
       disk->read(1, free_blocks);

	int i=0;
	int blockNum;
	while(i<size){
	
		if (free_blocks[i] == 0x0) {
			free_blocks[i] = 0xFF;
			break;
		}
		i++;
	}
	blockNum = i;
	disk->write(1, free_blocks);

	disk->read(0, inode_map);
	i=0;
	while(i<MAX_INODES){
	
		if (inode_list[i].id == -1) {
			inode_list[i].id = _file_id;
			inode_list[i].blockNum = blockNum;
			inode_list[i].currPos = 0;
			inode_list[i].fileSize = 0;
			break;
		}
		i++;
	}
	disk->write(0, inode_map);

	return true;
    //assert(false);
}

bool FileSystem::DeleteFile(int _file_id) {
 	
 	
       Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then 0x0 all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
      
       disk->read(0, inode_map);
       
	int i=0;
	int blockNum = -1;
	while(i < MAX_INODES){
	
		if (inode_list[i].id == _file_id) {
			blockNum = inode_list[i].blockNum;
			inode_list[i].id = -1;
			inode_list[i].blockNum = -1;
			inode_list[i].currPos = -1;
			inode_list[i].fileSize = -1;
			break;
		}
		i++;
	}

	if (blockNum == -1) {
		Console::puts("file doesn't exist\n");
		assert(false);
	}

	disk->write(0, inode_map);

	disk->read(1, free_blocks);
	free_blocks[blockNum] = 0x0;
	disk->write(1, free_blocks);
	LookupFile(_file_id);
	return true;
}
