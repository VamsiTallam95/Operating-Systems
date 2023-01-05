/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
     Console::puts("Opening file.\n");
	fs = _fs;
	id = _id;

    _inode_list = new Inode[FileSystem::MAX_INODES];
	
	fs->disk->read(0, (unsigned char *)_inode_list);
	int i=0;
	while(i<FileSystem::MAX_INODES){
	
		if (_inode_list[i].id == _id) {
		    blockNum = _inode_list[i].blockNum;
			fileSize = _inode_list[i].fileSize;
			currPos = _inode_list[i].currPos; 
			inode_no = i;
			break;
		}
		i++;
	}

    fs->disk->read(blockNum, block_cache);
    
    //assert(false);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
        fs->disk->read(0, (unsigned char *)_inode_list);
	_inode_list[inode_no].currPos = currPos;
	_inode_list[inode_no].fileSize = fileSize;
	fs->disk->write(0, (unsigned char *)_inode_list);
	delete[] _inode_list;
	
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    unsigned long _currPos;

    Console::puts("reading from file\n");
	if ((currPos + _n) >= fileSize) {
		memcpy(_buf, &(block_cache[currPos]), fileSize - currPos);
		_currPos = currPos;
		currPos = 0;
		return fileSize - _currPos;
	}

	memcpy(_buf, &(block_cache[currPos]), _n);
	currPos += _n;
	return _n;
}

int File::Write(unsigned int _n, const char *_buf) {
    int nbytes = 0;

    Console::puts("writing to file\n");
	if ((currPos + _n) >= SimpleDisk::BLOCK_SIZE) {
		memcpy(&(block_cache[currPos]), _buf, SimpleDisk::BLOCK_SIZE - currPos);
		nbytes = SimpleDisk::BLOCK_SIZE - currPos;
		fileSize = SimpleDisk::BLOCK_SIZE;
		currPos = 0;
	} else {
		memcpy(&(block_cache[currPos]), _buf, _n);
		nbytes = _n;
		fileSize += _n;
		currPos += _n;
	}

	fs->disk->write(blockNum, block_cache);

	return nbytes;
}

void File::Reset() {
    Console::puts("resetting file\n");
	currPos = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    if (currPos == fileSize - 1) {
		return true;
	}
	return false;
    //assert(false);
}
