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
    "Mystery, Inc.",
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

void* heap_listp = NULL;

#define NUM_SEG_LIST 1

size_t SEG_SIZES[NUM_SEG_LIST] = {1};
void* sep_list_head[NUM_SEG_LIST];

typedef struct double_list
{
    struct double_list* prev;
    struct double_list* next;
}dlist;


void print_list(int index)
{
    dlist *current = sep_list_head[index];
    while (current != NULL)
    {
        printf ("H:%d\n", GET(HDRP((void*)current)));
        printf ("P:%a\n", current->prev);
        printf ("N:%a\n", current->next);
        printf ("F:%d\n", GET(FTRP((void*)current)));
    }
}

}

void insert_node (void *new_bp)
{
    size_t size = GET_SIZE(HDRP(new_bp)) - DSIZE;

    int index, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (size >= SEG_SIZES[i])
            index = i;
        else
            break;
    }

    printf("Inserting node\n");

    dlist *new_node = (dlist*)new_bp;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (sep_list_head[index] == NULL) {
        sep_list_head[index] = new_bp;
        return;
    }

    dlist *head_node = (dlist*)sep_list_head[index];
      
    if (new_bp < sep_list_head[index]) {
        //insert before head
        printf("insert first\n");

        head_node->prev = new_node;
        new_node->next = head_node;
        sep_list_head[index] = new_bp;
        return;
    }

    dlist* current = head_node;

    if (current->next == NULL) {
        // one node
        printf("insert second\n");

        head_node->next = new_node;
        new_node->prev = head_node;
        return;
    }

    while (current->next != NULL && new_node > current->next)
        current = current->next;

    printf("insert mid or last\n");

    new_node->next = current->next;
    new_node->prev = current;

    if (current->next != NULL)
        current->next->prev = new_node;

    current->next = new_node;
}

void remove_node (int index, void *del_bp)
{
    dlist *current = (dlist*) del_bp;

    if (current->prev != NULL)
        current->prev->next = current->next;
    else
        sep_list_head[index] = current->next;

    if (current->next != NULL)
        current->next->prev = current->prev;

    current->prev = NULL;
    current->next = NULL;
}

void* search_node (size_t req_size)
{
    int sl_index, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (req_size >= SEG_SIZES[i])
            sl_index = i;
        else
            break;
    }

    while (sl_index < NUM_SEG_LIST) {
        dlist *current = (dlist*)sep_list_head[sl_index];
        while (current != NULL) {
            if (req_size <= GET_SIZE(HDRP((void*)current))) {
                remove_node(sl_index, (void*)current);
                return (void*)current;
            }
            current = current->next;
        }
        sl_index++;
    }
    return NULL;
}

void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    }

    size_t curr_size = size - DSIZE;
    size_t prev_size = GET_SIZE(PREV_BLKP(bp)) - DSIZE;
    size_t next_size = GET_SIZE(NEXT_BLKP(bp)) - DSIZE;

    int current_index, next_index, prev_index, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (curr_size >= SEG_SIZES[i])
            current_index = i;
        if (prev_size >= SEG_SIZES[i])
            prev_index = i;
        if (next_size >= SEG_SIZES[i])
            next_index = i;
    }

    remove_node(current_index, bp);

    if (prev_alloc && !next_alloc) { /* Case 2 */
        remove_node(next_index, NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        insert_node(bp);
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        remove_node(prev_index, PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        insert_node(PREV_BLKP(bp));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        remove_node(prev_index, PREV_BLKP(bp));
        remove_node(next_index, NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

        insert_node(PREV_BLKP(bp));
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
// void *coalesce(void *bp)
// {
//     size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
//     size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//     size_t size = GET_SIZE(HDRP(bp));

//     if (prev_alloc && next_alloc) {       /* Case 1 */
//         return bp;
//     }

//     else if (prev_alloc && !next_alloc) { /* Case 2 */
//         size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
//         PUT(HDRP(bp), PACK(size, 0));
//         PUT(FTRP(bp), PACK(size, 0));
//         return (bp);
//     }

//     else if (!prev_alloc && next_alloc) { /* Case 3 */
//         size += GET_SIZE(HDRP(PREV_BLKP(bp)));
//         PUT(FTRP(bp), PACK(size, 0));
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//         return (PREV_BLKP(bp));
//     }

//     else {            /* Case 4 */
//         size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
//             GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
//         PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
//         return (PREV_BLKP(bp));
//     }
// }

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    if (asize < bsize) {
        PUT(HDRP(NEXT_BLKP(bp)), PACK(bsize - asize, 1));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(bsize - asize, 1));
        insert_node(NEXT_BLKP(bp));
    }
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    // "I am become death, destroyer of the blocks."

    if(bp == NULL){
      return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
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
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

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
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
  return 1;
}
