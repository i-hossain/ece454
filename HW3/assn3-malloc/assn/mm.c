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

int mm_check(int d);

#define NUM_SEG_LIST 14

size_t SEG_SIZES[NUM_SEG_LIST] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144};
int utilization [NUM_SEG_LIST] = {0};
void* sep_list_head[NUM_SEG_LIST];
void* sep_list_tail[NUM_SEG_LIST];

typedef struct double_list
{
    struct double_list* prev;
    struct double_list* next;
}dlist;

void *epilogue_bp;

int re_bs = 0;
int re_ws = 0;

// #define DEBUG

void print_whole_block(void *bp, size_t size){
    dlist *blk = (dlist*)bp;
    
    if (blk != NULL) {
        if (GET_ALLOC(HDRP((void*)blk)) == 0)
            printf("XXXXX::");
        printf ("H:%ld::", GET(HDRP((void*)blk)));
        printf ("B:%p::", blk);
        printf ("P:%p::", blk->prev);
        printf ("N:%p\n", blk->next);

        void *end = bp + ((size == 0)?GET_SIZE(HDRP(bp)):size);
        while(bp != end) {
            printf("%ld::", GET(bp));
            bp++;
        }
        printf("\n");
    }
    else
        printf("BLOCK IS NULL\n");
}

void print_block(void *bp)
{
    dlist *blk = (dlist*)bp;
    if (blk != NULL) {
        if (GET_ALLOC(HDRP((void*)blk)) == 0)
            printf("XXXXX::");
        printf ("H:%ld::", GET(HDRP((void*)blk)));
        printf ("B:%p::", blk);
        printf ("P:%p::", blk->prev);
        printf ("N:%p\n", blk->next);
    }
    else
        printf("BLOCK IS NULL\n");
}

void print_list(int index)
{
    //char c;
    dlist *current = sep_list_head[index];
    while (current != NULL)
    {
        print_block((void*)current);
        current = current->next;
    }
    //scanf ("%c\n", &c);
}

int insert_node (void *new_bp)
{
    //mm_check(5);

    size_t size = GET_SIZE(HDRP(new_bp));

    int index = 0, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (size >= SEG_SIZES[i])
            index = i;
    }

#ifdef DEBUG
    printf("Inserting node: %ld\n", size);
#endif

    utilization[index]++;

    dlist *new_node = (dlist*)new_bp;
    new_node->prev = NULL;
    new_node->next = NULL;

    // dlist *head_node = (dlist*)sep_list_head[index];
    // if (head_node != NULL) {
    //     head_node->prev = new_node;
    // }
    // new_node->next = head_node;
    // sep_list_head[index] = new_bp;

    if (sep_list_head[index] == NULL) {
        sep_list_head[index] = new_bp;
        sep_list_tail[index] = new_bp;
        // mm_check();

        return index;
    }

    dlist *head_node = (dlist*)sep_list_head[index];
    // dlist *curr_node = (dlist*)sep_list_head[index];

    // char ch;
    // while (curr_node->next != NULL) {
    //     if (GET_SIZE(HDRP((void*)curr_node)) < GET_SIZE(HDRP((void*)(curr_node->next)))) {
    //         printf("Encountered error\n");
    //         scanf("%c\n", &ch);
    //     }
    //     curr_node = curr_node->next;
    // }
      
    if (size >= GET_SIZE(HDRP(sep_list_head[index]))) {
        //insert before head

#ifdef DEBUG
        printf("insert first\n");
#endif


        head_node->prev = new_node;
        new_node->next = head_node;
        sep_list_head[index] = new_bp;
        // mm_check();

        return index;
    }

    dlist* current = head_node;

    if (current->next == NULL) {
        // one node

#ifdef DEBUG
        printf("insert second\n");
#endif


        head_node->next = new_node;
        new_node->prev = head_node;

        // mm_check();
        sep_list_tail[index] = new_bp;

        return index;
    }

    while (current->next != NULL && size < GET_SIZE(HDRP(current->next)))
        current = current->next;

#ifdef DEBUG
    printf("insert mid or last\n");
#endif

    new_node->next = current->next;
    new_node->prev = current;

    if (current->next != NULL)
        current->next->prev = new_node;
    else
        sep_list_tail[index] = new_bp;

    current->next = new_node;

    return index;

    // mm_check();
}

void remove_node (int index, void *del_bp)
{
    dlist *current = (dlist*) del_bp;

    if (current->prev == NULL && current->next == NULL)
        if (sep_list_head[index] != current)
            return;

    // printf("Removing node\n");
    // printf ("H:%ld::", GET(HDRP((void*)current)));
    // printf ("B:%p::", current);
    // printf ("P:%p::", current->prev);
    // printf ("N:%p\n", current->next);

    if (current->prev != NULL)
        current->prev->next = current->next;
    else
        sep_list_head[index] = current->next;

    if (current->next != NULL)
        current->next->prev = current->prev;

    current->prev = NULL;
    current->next = NULL;

    //mm_check();
}

void* search_node (size_t req_size)
{
    //mm_check();
#ifdef DEBUG
    printf("Searching: %ld\n", req_size);
#endif
    // int counter = 0;

    int sl_index = 0, i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        if (req_size >= SEG_SIZES[i])
            sl_index = i;
        else
            break;
    }

    //int rev_loop = 0;

    while (sl_index < NUM_SEG_LIST) {
        dlist *current = (dlist*)sep_list_head[sl_index];
        //while (current != NULL) {
            if (current != NULL && req_size <= GET_SIZE(HDRP((void*)current))) {
                remove_node(sl_index, (void*)current);
#ifdef DEBUG
                printf("COUNTER: %d ---- %d ------\n", sl_index, counter);
#endif
                return (void*)current;
            }
            //counter++;
            //current = current->next;
        //}
        sl_index++;
    }
    return NULL;
}

void *coalesce(void *bp, size_t extendsize)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */

#ifdef DEBUG
        printf("case 1\n");
#endif

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

    if (prev_alloc && !next_alloc) { /* Case 2 */

#ifdef DEBUG
        printf("case 2: %d\n", next_index);
#endif

        if (extendsize > (size + next_size))
            return NULL;

        remove_node(next_index, NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */

#ifdef DEBUG
        printf("case 3: %d\n", prev_index);
#endif

        //mm_check(2);

        if (extendsize > (size + prev_size))
            return NULL;

        remove_node(prev_index, PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */

#ifdef DEBUG
        printf("case 4: %d :: %d\n", prev_index, next_index);
#endif
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

    epilogue_bp = heap_listp + (4 * WSIZE);
    heap_listp += DSIZE;

    //printf("HEAP_P: %p\n", heap_listp);
    //printf("EPILOGUE P: %p\n", epilogue_bp);

    // Init all structures
    int i;
    for(i = 0; i < NUM_SEG_LIST; i++) {
        sep_list_head[i] = NULL;
    }

    // for(i = 0; i < NUM_SEG_LIST; i++) {
    //     printf("UTIL [%d]: %d\n", i, utilization[i]);
    // }

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
void *deferred_coalesce(size_t asize)
{
    void *bp = heap_listp;
    size_t bp_size = GET_SIZE(HDRP(bp)), next_size;

    int next_index, curr_index, i;
    void *best_fit = NULL;
    int bf_index = 0;
    int bf_flag;

    while (bp_size > 0) {
        if (!GET_ALLOC(HDRP(bp)))
        {
            bf_flag = 1;
            //  && (asize <= GET_SIZE(HDRP(bp)))
            while (bf_flag && !GET_ALLOC(HDRP(NEXT_BLKP(bp)))) {
                // gotta coelesce
                curr_index = 0;
                next_index = 0;
                next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));

                for(i = 0; i < NUM_SEG_LIST; i++) {
                    if (next_size >= SEG_SIZES[i])
                        next_index = i;
                    if (bp_size >= SEG_SIZES[i])
                        curr_index = i;
                }

                remove_node(curr_index, bp);
                remove_node(next_index, NEXT_BLKP(bp));

                bp_size += next_size;
                PUT(HDRP(bp), PACK(bp_size, 0));
                PUT(FTRP(bp), PACK(bp_size, 0));

                if (asize <= bp_size) {
                    if (best_fit == NULL) {
                        best_fit = bp;
                        bf_index = insert_node(bp);
                        bf_flag = 0;
                    }
                    else if (bp_size < GET_SIZE(HDRP(best_fit))) {
                        best_fit = bp;
                        bf_index = insert_node(bp);
                        bf_flag = 0;
                    }
                    else
                        insert_node (bp);
                }
                else
                    insert_node (bp);
            }
        }
        bp = NEXT_BLKP(bp);
        bp_size = GET_SIZE(HDRP(bp));
    }
    if (best_fit != NULL)
        remove_node (bf_index, best_fit);

    return best_fit;
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    //printf("LAST BLK: %ld\n", GET_SIZE(FTRP(PREV_BLKP(epilogue_bp))));
    // if (!GET_ALLOC(HDRP(PREV_BLKP(epilogue_bp)))) {
    //     words -= (GET_SIZE(FTRP(PREV_BLKP(epilogue_bp))) / WSIZE);
    // }

    // printf("Extend heap: %ld\n", words * WSIZE);
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

    epilogue_bp = NEXT_BLKP(bp);

    // dlist *ext_bp = (dlist*)bp;
    // ext_bp->prev = NULL;
    // ext_bp->next = NULL;

    /* Coalesce if the previous block was free */
    return coalesce(bp, 0);
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

#ifdef DEBUG
    printf("placing %ld -> %ld\n", asize, bsize);
#endif

    if (bsize - asize >= 4 * WSIZE) {
        // min block size
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(bsize - asize, 0));
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

#ifdef DEBUG
    printf("mm_free invoked: %p\n", bp);
#endif

    //mm_check();

    if(bp == NULL){
      return;
    }

    // double free?! - check if bp is already free
    if (!GET_ALLOC(HDRP(bp))) {
        // printf("double free called!!\n");
        return;
    }

    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    // dlist *ext_bp = (dlist*)bp;
    // ext_bp->prev = NULL;
    // ext_bp->next = NULL;

    void *cbp = coalesce(bp, 0);

    //mm_check(4);

    insert_node(cbp);

    //mm_check(4);
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
    // mm_check();

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

#ifdef DEBUG
    printf("mm_malloc invoked: %ld :: %ld\n", size, asize);
#endif

    /* Search the free list for a fit */
    if ((bp = search_node(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    // Nothing found. coalesce to see if we can get space.
    // if ((bp = deferred_coalesce(asize)) != NULL) {
    //     place(bp, asize);
    //     return bp;
    // }

    /* No fit found. Get more memory and place the block */
    // Check if last block is not allocated
//     int csize = asize;

//     if (!GET_ALLOC(HDRP(PREV_BLKP(epilogue_bp)))) {
//         size_t lb_size = GET_SIZE(FTRP(PREV_BLKP(epilogue_bp)));

// #ifdef DEBUG
//         printf("Last block free: %ld\n", lb_size);
// #endif

//         size -= lb_size;
//         if (size <= DSIZE)
//             csize = 2 * DSIZE;
//         else
//             csize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);
//     }

//     extendsize = MAX(csize, CHUNKSIZE);
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
    // printf("mm_realloc called %p : %ld\n", ptr, size);
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    if (!GET_ALLOC(HDRP(ptr)))
        return (mm_malloc(size));

    // printf("BEFORE\n");
    // mm_check(3);

    void *oldptr = ptr;
    size_t copySize = GET_SIZE(HDRP(oldptr));

    /* Adjust block size to include overhead and alignment reqs. */
    size_t asize;
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    if (asize <= copySize) {
        // need to shrink block
        // copySize = size;
        place(oldptr, asize);
        // printf("AFTER place\n");
        // mm_check(3);
        return oldptr;
    }
    // else {
        // size_t prev_size = GET_SIZE(HDRP(PREV_BLOCK(oldptr)));
    // PUT(HDRP(oldptr), PACK(copySize,0));
    // PUT(FTRP(oldptr), PACK(copySize,0));

        void *cptr = coalesce(oldptr, asize);

        if (cptr == NULL) {
            // printf("worst scenario: %d\n", ++re_ws);
        //     // could not coalesce

            // if (oldptr == PREV_BLKP(epilogue_bp)) {
            //     // it is the last block on heap

            //     char *bp;
            //     size_t size;
            //     size_t words = (asize - copySize) / WSIZE;

            //     /* Allocate an even number of words to maintain alignments */
            //     size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
            //     if ( (bp = mem_sbrk(size)) == (void *)-1 )
            //         return NULL;

            //     /* Initialize free block header/footer and the epilogue header */
            //     PUT(HDRP(bp), PACK(size, 0));                // free block header
            //     PUT(FTRP(bp), PACK(size, 0));                // free block footer
            //     PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

            //     epilogue_bp = NEXT_BLKP(bp);

            //     size += copySize;
            //     PUT(FTRP(bp), PACK(size, 1));
            //     PUT(HDRP(oldptr), PACK(size, 1));

            //     place(oldptr, asize);
            //     return oldptr;
            // }
            // else {
                void *newptr = mm_malloc(size);
                if (newptr == NULL)
                  return NULL;

                // Copy the old data.
                memcpy(newptr, oldptr, copySize);
                mm_free(oldptr);
                return newptr;
            // }
        }
        else if (cptr < oldptr) {

            // printf("worst scenario: %d\n", ++re_ws);
            // prev blk pointer returned
            // Copy the old data.
            // print_whole_block(oldptr, copySize);
            memmove(cptr, oldptr, copySize);
            //print_whole_block(cptr, copySize);

            place(cptr, asize);
            // print_whole_block(cptr, copySize);

            // PUT(HDRP(cptr), PACK(GET_SIZE(HDRP(cptr)),1));
            // PUT(FTRP(cptr), PACK(GET_SIZE(HDRP(cptr)),1));

        }
        else {
            // printf("best scenario: %d\n", ++re_bs);
            place(cptr, asize);
        }
        // printf("AFTER coalesce\n");
        // mm_check(3);

        return cptr;
    // }
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(int d){
    
    //char c;
    //printf("Enter number:");
    //scanf("%d\n", &d);
    printf("----------------------------------------------------\n");
    if (d == 1 || d == 3) { // print if d = 1
        void *bp;
        for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
        {
            print_block(bp);
        }
    }
    if (d == 2 || d == 3) { // print if d = 2
        int i;
        for(i = 0; i < NUM_SEG_LIST; i++) {
            if (sep_list_head[i] != NULL) {
                print_list(i);
            }
            else {
                printf("sep_list_head[%d] is NULL\n", i);
            }
        }
    }
    if (d == 5) {

    }
    printf("----------------------------------------------------\n");
    //scanf("%c\n",&c);

    return 1;
}
