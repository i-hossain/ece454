/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Midnight Mafia",
    /* First member's full name */
    "Rushab Ramesh Kumar",
    /* First member's email address */
    "rushab.kumar@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Ismail Hossain",
    /* Second member's email address (leave blank if none) */
    "ridoy.hossain@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// Global definitions for Lab
//#define DEBUG_BUILD

#ifdef DEBUG_BUILD
#define PRINTDBG(x) (printf x)
#else
#define PRINTDBG(x)
#endif

#define NUM_SEG_LIST 14

void* heap_listp = NULL;

size_t SEG_SIZES[NUM_SEG_LIST] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144};

void* sep_list_head[NUM_SEG_LIST];

typedef struct double_list
{
    struct double_list* prev;
    struct double_list* next;
}dlist;

int mm_check(void);
void print_block_contents(void *bp, size_t size);
void print_block(void *bp);
void print_heap();
void print_list(int index);
void print_sep_list();
int is_in_heap(void *bp);

/**********************************************************
 * insert_node
 * Inserts specified node into the corresponding index of
 * the segregated free list
 **********************************************************/
int insert_node (void *new_bp)
{
    size_t size = GET_SIZE(HDRP(new_bp));
    int index = 0, i;

    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (size >= SEG_SIZES[i])
            index = i;
    }

    PRINTDBG (("Inserting node: %ld\n", size));

    dlist *new_node = (dlist*)new_bp;
    new_node->prev = NULL;
    new_node->next = NULL;

    if (sep_list_head[index] == NULL) {
        sep_list_head[index] = new_bp;

        return index;
    }

    dlist *head_node = (dlist*)sep_list_head[index];
      
    if (size >= GET_SIZE(HDRP(sep_list_head[index]))) { //insert before head

        PRINTDBG (("insert first\n"));

        head_node->prev = new_node;
        new_node->next = head_node;
        sep_list_head[index] = new_bp;

        return index;
    }

    dlist* current = head_node;

    if (current->next == NULL) {    //one node

        PRINTDBG (("insert second\n"));

        head_node->next = new_node;
        new_node->prev = head_node;

        return index;
    }

    while (current->next != NULL && size < GET_SIZE(HDRP(current->next)))
        current = current->next;

    PRINTDBG (("insert mid or last\n"));

    new_node->next = current->next;
    new_node->prev = current;

    if (current->next != NULL)
        current->next->prev = new_node;

    current->next = new_node;

    return index;
}

/**********************************************************
 * remove_node
 * Removes a specified node from the segregated list of
 * free blocks
 **********************************************************/
void remove_node (int index, void *del_bp)
{
    dlist *current = (dlist*) del_bp;

    if ((current->prev != NULL && !is_in_heap(current->prev)) || 
        (current->next != NULL && !is_in_heap(current->next)))
        return;

    if (current->prev == NULL && current->next == NULL)
        if (sep_list_head[index] != current)
            return;

    PRINTDBG (("Removing node\n"));
    PRINTDBG (("H:%ld::", GET(HDRP((void*)current))));
    PRINTDBG (("B:%p::", current));
    PRINTDBG (("P:%p::", current->prev));
    PRINTDBG (("N:%p\n", current->next));

    if (current->prev != NULL)
        current->prev->next = current->next;
    else
        sep_list_head[index] = current->next;

    if (current->next != NULL)
        current->next->prev = current->prev;

    current->prev = NULL;
    current->next = NULL;
}

// void* search_full (int sl_index, size_t req_size)
// {
//     PRINTDBG (("Searching Full: %ld\n", req_size));

//     void *found_node = NULL;

//     while (sl_index < NUM_SEG_LIST) {
//         dlist *current = (dlist*)sep_list_head[sl_index];
//         while (current != NULL) {
//             if (req_size <= GET_SIZE(HDRP((void*)current))) {
//                 found_node = (void*)current;
//             }
//             else {
//                 break;
//             }
//             current = current->next;
//         }
//         if (found_node != NULL) {
//             remove_node(sl_index, found_node);
//             return found_node;
//         }
//         sl_index++;
//     }
//     return NULL;
// }

/**********************************************************
 * Finds first fit
 * Goto each head pointer in the separated list and find
 * a free block
 * RETURN null if not found
 **********************************************************/
void* search_node (size_t req_size)
{
    PRINTDBG (("Searching: %ld\n", req_size));

    int sl_index = 0, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (req_size >= SEG_SIZES[i])
            sl_index = i;
        else
            break;
    }

    while (sl_index < NUM_SEG_LIST) {
        dlist *current = (dlist*)sep_list_head[sl_index];
            if (current != NULL) {
                if(req_size <= GET_SIZE(HDRP((void*)current))) {
                    remove_node(sl_index, (void*)current);
                    return (void*)current;
                }
                // else {
                //     return search_full(sl_index+1, req_size);
                // }
            }
        sl_index++;
    }
    return NULL;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/

void *coalesce(void *bp, size_t extendsize)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {         /* Case 1 */

        PRINTDBG (("case 1\n"));

        if (extendsize != 0)
            return NULL;
        return bp;
    }

    size_t prev_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));

    int next_index = 0, prev_index = 0, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (prev_size >= SEG_SIZES[i])
            prev_index = i;
        if (next_size >= SEG_SIZES[i])
            next_index = i;
    }

    if (prev_alloc && !next_alloc) {        /* Case 2 */

        PRINTDBG (("case 2: %d\n", next_index));

        if (extendsize > (size + next_size))
            return NULL;

        remove_node(next_index, NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        return (bp);
    }

    else if (!prev_alloc && next_alloc) {   /* Case 3 */

        PRINTDBG (("case 3: %d\n", prev_index));

        if (extendsize > (size + prev_size))
            return NULL;

        remove_node(prev_index, PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        return (PREV_BLKP(bp));
    }

    else {                                  /* Case 4 */


        PRINTDBG (("case 4: %d :: %d\n", prev_index, next_index));

        if (extendsize > (size + prev_size + next_size))
            return NULL;

        remove_node(prev_index, PREV_BLKP(bp));
        remove_node(next_index, NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header

    heap_listp += DSIZE;

    // Initialize all structures
    int i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        sep_list_head[i] = NULL;
    }

    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/


/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    PRINTDBG (("Extend heap: %ld\n", words * WSIZE));

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp, 0);
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PRINTDBG (("placing %ld -> %ld\n", asize, bsize));

    // If slicing will produce a block less than 2 * DSIZE then dont slice.
    if (bsize - asize >= 4 * WSIZE) {
        // min block size
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));

        // Insert the sliced block in the free list
        insert_node(NEXT_BLKP(bp));
    }
    else {
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
    }
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    // "I am become death, destroyer of the blocks."
    PRINTDBG (("mm_free invoked: %p\n", bp));

    if (bp == NULL) {
      return;
    }

    // double free?! - check if bp is already free
    if (!GET_ALLOC(HDRP(bp))) {
        PRINTDBG (("double free called!!\n"));
        return;
    }

    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    // Immediately coalesce the free block and then insert in separated list
    void *cbp = coalesce(bp, 0);
    insert_node(cbp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize = 0; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    PRINTDBG (("mm_malloc invoked: %ld :: %ld\n", size, asize));

    /* Search the free list for a fit */
    if ((bp = search_node(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    PRINTDBG (("mm_realloc called %p : %ld\n", ptr, size));

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
      mm_free(ptr);
      return NULL;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    /* If realloc is called with already free block then malloc new block */
    if (!GET_ALLOC(HDRP(ptr)))
        return (mm_malloc(size));

    void *oldptr = ptr;
    size_t copySize = GET_SIZE(HDRP(oldptr));

    /* Adjust block size to include overhead and alignment reqs. */
    size_t asize;
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    // If asize is less then just slice the block
    if (asize <= copySize) {
        place(oldptr, asize);
        return oldptr;
    }

    // print_heap();
    void *cptr = coalesce(oldptr, asize);
    // print_heap();
    // char c;
    // scanf("%c\n", &c);

    if (cptr == NULL) {
        // If coalescing was not possible then allocate new block
        void *newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;

        // Copy the old data.
        memcpy(newptr, oldptr, copySize);
        mm_free(oldptr);
        return newptr;
    }
    else if (cptr < oldptr) {
        // If coalesced with prev block then memmove the data
        memmove(cptr, oldptr, copySize);

        place(cptr, asize);
    }
    else {
        // If coalesced with next block then slice the block
        //place(cptr, asize);
        
        /* Get the current block size */
        size_t bsize = GET_SIZE(HDRP(cptr));

        PRINTDBG (("placing %ld -> %ld\n", asize, bsize));

        // If slicing will produce a block less than 2 * DSIZE then dont slice.
        if (bsize - asize >= 4 * WSIZE) {
            // min block size
            PUT(HDRP(cptr), PACK(asize, 1));
            PUT(FTRP(cptr), PACK(asize, 1));

            PUT(HDRP(NEXT_BLKP(cptr)), PACK(bsize - asize, 0));
            PUT(FTRP(NEXT_BLKP(cptr)), PACK(bsize - asize, 0));

            // Dont insert block in free list so it can be used later for realloc
        }
        else {
            PUT(HDRP(cptr), PACK(bsize, 1));
            PUT(FTRP(cptr), PACK(bsize, 1));
        }
    }

    return cptr;
}

// Debugging functions

/**********************************************************
 * print_block_contents
 * Prints the entire block including its payload
 **********************************************************/
void print_block_contents(void *bp, size_t size)
{
    dlist *blk = (dlist*)bp;

    if (blk != NULL) {
        if (GET_ALLOC(HDRP((void*)blk)) == 0)
            PRINTDBG (("XXXXX::"));
        
        PRINTDBG (("H:%ld::", GET(HDRP((void*)blk))));
        PRINTDBG (("B:%p::", blk));
        PRINTDBG (("P:%p::", blk->prev));
        PRINTDBG (("N:%p\n", blk->next));

        void *end = bp + ((size == 0)?GET_SIZE(HDRP(bp)):size);
        
        while (bp != end) {
            PRINTDBG (("%ld::", GET(bp)));
            bp++;
        }

        PRINTDBG (("\n"));
    }
    else
        PRINTDBG (("BLOCK IS NULL\n"));
}

/**********************************************************
 * print_block
 * Prints the header, current block pointer,
 * previous block pointer, next block pointer
 **********************************************************/
void print_block(void *bp)
{
    dlist *blk = (dlist*)bp;

    if (blk != NULL) {
        if (GET_ALLOC(HDRP((void*)blk)) == 0)       //free block
            PRINTDBG (("XXXXX::"));

        PRINTDBG (("H:%ld::", GET(HDRP((void*)blk))));
        PRINTDBG (("B:%p::", blk));
        PRINTDBG (("P:%p::", blk->prev));
        PRINTDBG (("N:%p\n", blk->next));
    }
    else
        PRINTDBG (("BLOCK IS NULL\n"));
}

/**********************************************************
 * print_heap
 * Prints all the blocks in the heap till epilogue
 **********************************************************/
void print_heap()
{
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        print_block(bp);
    }
}

/**********************************************************
 * print_list
 * Determines which bin to print from the segregated list
 * based on the index
 **********************************************************/
void print_list(int index)
{
    dlist *current = sep_list_head[index];
    
    while (current != NULL) {
        print_block((void*)current);
        current = current->next;
    }
}

/**********************************************************
 * print_sep_list
 * Prints all the blocks in the segregated lists
 **********************************************************/
void print_sep_list()
{
    int i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (sep_list_head[i] != NULL) {
            print_list(i);
        }
        else {
            PRINTDBG (("sep_list_head[%d] is NULL\n", i));
        }
    }
}

/**********************************************************
 * is_in_heap
 * Checks if the block pointer is in heap. Prints an error
 * message if the block is outside the heap
 **********************************************************/
int is_in_heap(void *bp)
{
    if (bp < mem_heap_lo() || bp > mem_heap_hi())
    {
        PRINTDBG (("The block is not on the heap\n"));
        return 0;
    }
    
    return 1;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void)
{
#ifdef DEBUG_BUILD

    int i;
    dlist *current;
    void *bp; //To point at the start of the heap
    size_t size = 0; //To measure the total size of blocks on the heap

    // Check if every block in free list is actually marked free.
    // If a allocated block is in the free list then it may result in segmentation faults
    // while accessing the prev and next pointers. It will also result in the allocater manipulating 
    // memory allocated to user which will be nasty!

    for(i = 0; i < NUM_SEG_LIST; i++) {
        current = sep_list_head[i];
    
        while (current != NULL) {
            // Check if block is in heap
            if (is_in_heap((void*)current) == 0)
                return 0;

            if (GET_ALLOC(HDRP((void*)current))) {
                PRINTDBG (("block (%p) in free list is allocated!\n", current));
                return 0;
            }
            current = current->next;
        }
    }

    // Check if there exist any free blocks that may have escaped coalescing. 
    // If found, these cases result in internal fragmentation.
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        // Check if the current block is free and the next block is also free
        // Since this is done in a sequential manner, we don't need to check the previous blk
        if (!GET_ALLOC(HDRP(bp)) && (!GET_ALLOC(HDRP(NEXT_BLKP(bp))))) {
            PRINTDBG (("Consecutive blocks (%p, %p) have escaped coalescing!\n", bp, NEXT_BLKP(bp)));
            return 0;
        }
    }

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (bp < mem_heap_lo() || bp > mem_heap_hi()) {
            PRINTDBG (("Block is outside the heap.\n"));
            return 0;
        }
        else {
            size += GET_SIZE(HDRP(bp));     //Add the size of the current block to the total size of the calculated blocks on the heap so far
        }
    }
            
    if (size == mem_heapsize())
        PRINTDBG (("The total size of all blocks on the heap match the heap size.\n"));

    if (mem_heapsize() > mem_pagesize())   //Check if heap size exceeded systems page size  
        PRINTDBG (("Heap size is more than page size. TLB misses might occur\n"));

    char c;
    scanf("%c\n", &c);

#endif
    return 1;
}
