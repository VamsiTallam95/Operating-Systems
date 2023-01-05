/*
 File: ContFramePool.C
 
 Author: Vamsi Tallam
 Date  : Sep 18 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define K  * (0x1 << 10)


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/


ContFramePool* ContFramePool::pool_head = NULL;

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
	// Assign head and next of frame pool linked list
	pool_next = pool_head;
	pool_head = this;

	base_frame_no = _base_frame_no;
	n_frames      = _n_frames;
	n_free_frames = _n_frames;
	info_frame_no = _info_frame_no;
	unsigned long n_info_frames = 1;

	// bitmap is stored in frame 0 in case _info_frame_no is 0
	// Else, the bitmap is storeed in provided frame number
	if (info_frame_no == 0) {
		bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
	} else {
		bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
	}

	// Number of frames must "fill" bitmap!
	assert ((n_frames % 4) == 0);

	// Initially all bytes in bitmap are set as free
	for (int i = 0; i < n_frames; i++) {
		set_state(i, FrameState::Free);
	}

	// Mark the info frames as used
	for (unsigned long i = info_frame_no; i < info_frame_no + n_info_frames; i++) {
		set_state(i, FrameState::Used);
		n_free_frames--;
	}

	Console::puts("Frame pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
	unsigned long frame_count = 0;
	bool found_mem = false;

	// Look in the memory for a contiguous sequence of free frames needed for allocation 
	unsigned long i =0;
	while(i<n_frames){
		if (get_state(i) == FrameState::Free) {
			for (unsigned long j = i; get_state(i) == FrameState::Free; j++) {
				frame_count++;
				if (frame_count == _n_frames) {
					found_mem = true; // flag to ensure that space is found.
					break;
				} // Found a contiguous memory chunk, can now break
			}
		}

		if (found_mem) {
			set_state(i, FrameState::HoS); // Mark first frame as head-of-sequence
			
			// Set rest as allocated
			unsigned long k = i+1;
			while( k < i+frame_count) {
				set_state(k, FrameState::Used);
				k++;
			}

			n_free_frames -= frame_count;
			break;
		}

		frame_count = 0;
		i++;
		
	}

	if (found_mem) {
		return i;
	}

	return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
	set_state(_base_frame_no, FrameState::HoS);

	for (unsigned long i = _base_frame_no + 1; i < _base_frame_no + _n_frames; i++) {
		set_state(i, FrameState::Used);
	}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	ContFramePool* temp;
	for (temp = pool_head; temp->pool_next != NULL; temp = temp->pool_next) {
		// Do bound checking on frame
		if ((_first_frame_no >= temp->base_frame_no) &&
			(_first_frame_no < (temp->base_frame_no + temp->n_frames))) {
			if (temp->get_state(_first_frame_no) != FrameState::HoS) {
				Console::puts("[ERROR][release_frames]: First frame is not HEAD-OF-SEQUENCE\n");
				assert(false);
			}

			temp->set_state(_first_frame_no, FrameState::Free);
			temp->n_free_frames++;

			// Free until we encounter "FREE" or "HEAD_OF_SEQ"
			unsigned long i = _first_frame_no + 1;
			while (temp->get_state(i) == FrameState::Used) {
				temp->set_state(i, FrameState::Free);
				temp->n_free_frames++;
			}

			break;
		}
	}
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
	// 2 bits for each frame in frame pool
	// One frame (4 KB) can hold (8 * 4096) / 2 = 16k frames
        int framesNeeded = (_n_frames/(16 K));
	int extraOffset = ((_n_frames % (16 K)) > 0 ? 1 : 0);
	return framesNeeded+extraOffset;
	
}

ContFramePool::FrameState ContFramePool::get_state(unsigned long frame_num)
{

	unsigned char u = bitmap[frame_num / 4];
    unsigned long x = ((u >> (2 * (frame_num % 4))) & 0x3);
    if (x == 0x1)
        return FrameState::HoS;
    if(x== 0x3)
        return FrameState::Free;
    if (x==0x0)
        return FrameState::Used;
	//return (u >> (2 * (frame_num % 4))) & 0x3;
}

void ContFramePool::set_state(unsigned long frame_num, FrameState status)
{
	unsigned char u = bitmap[frame_num / 4];
	
	// Create mask for right 2 bits
	unsigned char mask = 0x3 << (2 * (frame_num % 4));
	unsigned long _status;
	if(status == FrameState::HoS)
	 _status= 0x1;
	else if(status == FrameState::Free)
	_status = 0x3;
	else
	_status = 0x0;
	unsigned char new_status = _status << (2 * (frame_num % 4));
	
	// Mask and update 2 bits to given status
	bitmap[frame_num / 4] = (bitmap[frame_num / 4] & ~mask)
							| (new_status & mask);
}
