/*------------------------------------------------------------------------------
GWU - CSCI 3411 - Fall 2017
Best fit memory test
author : James Taylor
------------------------------------------------------------------------------*/

#include "types.h"
#include "user.h"

#define VERBOSE 1   // determines whether tests are produce status info

//-----------------------------------------------------------------------------
// These defines should remain constant.  Modifying them will require 
// modification of test logic
#define SBRK_SZ 	4096			// page size (bytes)
#define MEM_HEADER_SZ 	8			// size of the freelist header
#define SEGMENT_SZ 	256			// small size for segments
#define SEGMENTS 	SBRK_SZ / SEGMENT_SZ	// number of segments to alloc
#define BLOCKS 		4			// number of blocks to alloc

//-----------------------------------------------------------------------------
// computes the local allocation segment size which is the segment size less
// the hidden size of the freelist header
int segment_size( void ) {
	return SEGMENT_SZ - MEM_HEADER_SZ;
}

//-----------------------------------------------------------------------------
// computes the local allocation block size which is a multiple of the segment 
// size less the hidden size of the freelist header
int block_size( int x ) {
	return SEGMENT_SZ * x - MEM_HEADER_SZ;
}

//-----------------------------------------------------------------------------
// computes the 'true' size of an allocation meaning takes the stored allocation
// size and adds back the size of the freelist header
int alloc_size( int sz ) {
	return sz + MEM_HEADER_SZ;
}

//-----------------------------------------------------------------------------
// structure to hold information on each allocation for test record keeping 
struct alloc {
	char* ptr;
	int sz;
	int free;
};

//-----------------------------------------------------------------------------
int bestfit_search_test( void ) {
	int pass = 0;

	// heavily segment the memory
	// free various blocks of different sizes from memory... big to small
	// allocate several new segments small to big

	struct alloc segments[SEGMENTS];
	struct alloc blocks[BLOCKS];
	int i,j,k;

	// allocate a bunch of segments of relatively small size
	for(i = 0; i < SEGMENTS; i++) {
		segments[i].sz = segment_size();
		segments[i].ptr = (char*)malloc(segments[i].sz*sizeof(char));
		segments[i].free = 0;
	}

	if( VERBOSE ) {
		// print a table of the initial segment allocations
		printf( 1, "%d segment addresses\n", SEGMENTS);
		for(i = 0; i < SEGMENTS; i++) {
			printf( 1, "%p, alloc:%d, free:%d\n", segments[i].ptr, alloc_size(segments[i].sz), segments[i].free);
		}
	}

	// free a number of the segments beginning with a relatively large block
	// and proceed to free smaller blocks.  Leave an allocated segment 
	// between each block so that the free space remains fragmented
	k = 0;
	for(j = BLOCKS; j > 0; j--) {
		for(i = 0; i < j; i++) {
			free(segments[k+i].ptr);
			segments[k+i].free = 1;
		}
		k += j + 1;
	}

	if( VERBOSE ) {
		// print the segment allocation table again so the state of 
		// the segments can be visualized 
		printf( 1, "%d segment addresses\n", SEGMENTS);
		for(i = 0; i < SEGMENTS; i++) {
			printf( 1, "%p, alloc:%d, free:%d\n", segments[i].ptr, alloc_size(segments[i].sz), segments[i].free);
		}
	}
	
	// allocate blocks beginning with a relatively large block and proceed
	// to allocate smaller blocks.  This order is chosen because the order
	// in which the freelist is maintained
	for(i = BLOCKS; i > 0 ; i--) {
		blocks[i-1].sz = block_size(i);
		blocks[i-1].ptr = (char*)malloc(blocks[i-1].sz * sizeof(char));
		blocks[i-1].free = 0;	
	}

	if( VERBOSE ) {
		// print the blocks so that they can be visualized
		printf( 1, "%d block addresses\n", BLOCKS);
		for(i = 0; i < BLOCKS; i++) {
			printf( 1, "%p, alloc:%d\n", blocks[i].ptr, alloc_size(blocks[i].sz));
		}
	}

	// validate the block allocations.  If the allocation policy is best fit
	// the block allocations should fit within the original segments.  If
	// another policy is used, then the block allocations will have resulted
	// in a new page being requested and the block addresses will not map
	// to the original segment allocations
	if( blocks[0].ptr == segments[12].ptr &&
            blocks[1].ptr == segments[10].ptr &&
            blocks[2].ptr == segments[7].ptr &&
            blocks[3].ptr == segments[3].ptr ) {
		pass = 1;
	}

	// clean up the allocations
	for(i = 0; i < SEGMENTS; i++) {
		if( !segments[i].free ) free(segments[i].ptr);
	}
	for(i = 0; i < BLOCKS; i++) {
		if( !blocks[i].free ) free(blocks[i].ptr);
	}

	if( !pass ) return 1;
	return 0;
}

//-----------------------------------------------------------------------------
int bigalloc_test( void ) {
	int pass = 0;

	// allocate multiple pages
	char *p1, *p2;
	int sz = 2 * SBRK_SZ - MEM_HEADER_SZ;
	int delta;

	// This is a two page block.  One block is already in the freelist
	// from the previous test, so this allocates a second page and merges
	// the two
	p1 = (char*)malloc( sz * sizeof(char));
	if( p1 == 0 ) {
		return 2;
	}
	// This is a two page block.  Neither have been requested from the
	// kernel yet, so both pages must be allocated in one call to malloc 
	p2 = (char*)malloc( sz * sizeof(char));
	if( p2 == 0 ) {
		free(p1);
		return 3;
	}

	// compute the differential between pointers
	delta = p1 - p2;

	if( VERBOSE ) {
		// print the references and delta for user visualization
		printf(1, "%p\n", p1);
		printf(1, "%p\n", p2);
		printf(1, "delta:%d\n", delta);
	}

	// The differential between p1 and p2 must be two pages; otherwise,
	// the malloc call for p2 failed to produce one large block
	if( delta == 2 * SBRK_SZ ) {
		pass = 1;
	}

	free(p1);
	free(p2);

	if( !pass ) return 1;
	return 0;
}

//-----------------------------------------------------------------------------
int main( int argv, char* argc[] ) {
	int res;

	// Test 1
	printf( 1, "Verifying allocations find best fit blocks\n" );
	if( (res = bestfit_search_test()) == 0 ) {	
		printf( 1, "Bestfit search test: pass\n" );
	} else {
		printf( 1, "Bestfit search test: fail (code: %d)\n", res );
	}

	printf(1, "\n");

	// Test 2
	printf( 1, "Verifying allocations can span multiple pages\n" );
	if( (res = bigalloc_test()) == 0 ) {
		printf( 1, "Multiple page allocation test: pass\n" );
	} else {
		printf( 1, "Multiple page allocation test: fail (code: %d)\n", res );
	}

	exit();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------