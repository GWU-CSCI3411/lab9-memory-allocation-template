#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

// Refactor of XV6 distribution umalloc.c to make variables and algorithm more 
// clear

struct header {
	uint          size;
};

struct freelist_node {
	struct freelist_node *next;
	struct header hdr;
};

static struct freelist_node  base;
static struct freelist_node *freelist;

/**
 * @brief Frees memory allocated at the given address
 * @param *ptr address of allocated memory to free
*/

void
free(void *ptr)
{
	struct freelist_node *mem, *node;

	mem = (struct freelist_node *)ptr - 1;
	node = freelist;
	// find where the memory to free (mem) lies 
	while( !(mem > node && mem < node->next) ) {
		if (node >= node->next && (mem > node || mem < node->next)) break;
		node = node->next;
	}
	if (mem + mem->hdr.size == node->next) {
		//  mem borders the next freelist node, merge them
		mem->hdr.size += node->next->hdr.size;
		mem->next = node->next->next;
	} else
		// otherwise link mem to next freelist node
		mem->next = node->next;

	if (node + node->hdr.size == mem) {
		// if mem borders previous freelist node, merge them
		node->hdr.size += mem->hdr.size;
		node->next = mem->next;
	} else
		// otherwise list the previous freelist node to mem
		node->next = mem;

	// update the freelist to point at the freelist
	freelist = node;
}

/**
 * @brief allocate new page upon request from malloc
 * @param nu indicates amount of memory requested
 * @return new freelist node (will point at most recent node)
*/
static struct freelist_node *
morecore(uint nu)
{
	char *  ptr;
	struct freelist_node *node;

	// if the allocation is smaller than the minimum, set it to the minimum
	if (nu < 4096) nu = 4096;
	// request a page
	ptr = sbrk(nu * sizeof(struct freelist_node));
	if (ptr == (char *)-1) return 0;
	// update page size
	node         = (struct freelist_node *)ptr;
	node->hdr.size = nu;
	// call free to get node into the freelist
	free((void *)(node + 1));
	return freelist;
}

/**
 * @brief allocate memory of a given size
 * @param nbytes signifies the amount of space needing to be allocated
 * @return pointer to allocated memory
*/
void *
malloc(uint nbytes)
{
	struct freelist_node *node, *prev_node;
	uint    nunits;

	// compute the number of units need for allocation
	nunits = (nbytes + sizeof(struct freelist_node) - 1) / sizeof(struct freelist_node) + 1;
	// if malloc hasn't been called before, init base pointer
	if (freelist == 0) {
		// init base pointer to point to itself and all others to reference base
		base.next = freelist = prev_node = &base;
		base.hdr.size = 0;
	}
	// prep to iterate over freelist
	node = freelist->next;
	prev_node = freelist;

	while(1) {
		if (node->hdr.size >= nunits) {
			if (node->hdr.size == nunits) {
				// if current node's size matches, amend the freelist
				prev_node->next = node->next;
			} else {
				// otherwise shrink excess space from the node
				node->hdr.size -= nunits;
				// advance the pointer to the allocated node
				node += node->hdr.size; 
				// update the allocated node size to exact size
				node->hdr.size = nunits;
			}
			freelist = prev_node;

			return (void *)(node + 1);
		}

		// if we run out of space, request more
		if (node == freelist)
			if ((node = morecore(nunits)) == 0) return 0;

		prev_node = node;
		node = node->next;
	}
}