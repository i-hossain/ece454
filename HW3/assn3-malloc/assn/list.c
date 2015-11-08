#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"

int mm_init(void) {return 1;}
void *mm_malloc(size_t size) {return NULL;}
void mm_free(void *ptr) {return;}
void *mm_realloc(void *ptr, size_t size) {return NULL;}

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

#define WSIZE       sizeof(int *)            /* word size (bytes) */
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
#define HDRP(bp)        ((uintptr_t *)(bp) - WSIZE)
#define FTRP(bp)        ((uintptr_t *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((uintptr_t *)(bp) + GET_SIZE(((uintptr_t *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((uintptr_t *)(bp) - GET_SIZE(((uintptr_t *)(bp) - DSIZE)))

void* heap_listp = NULL;

#define NUM_SEG_LIST 1

size_t SEG_SIZES[NUM_SEG_LIST] = {1};
void* sep_list_head[NUM_SEG_LIST];

typedef struct double_list
{
    struct double_list* prev;
    struct double_list* next;
}dlist;

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

    // dlist* current2 = head_node->next;

    // if (current2->next == NULL) {
    //     // one node
    //     printf("insert third\n");

    //     head_node->next->next = new_node;
    //     new_node->prev = head_node->next;
    //     return;
    // }

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

void* seek_and_destroy (size_t req_size)
{
    void *bp = search_node(req_size);

    if (bp != NULL) {
        size_t size_bp = GET_SIZE(HDRP(bp));
    
        PUT(HDRP(bp), PACK(req_size, 1));
        PUT(FTRP(bp), PACK(req_size, 1));

        if (req_size < size_bp) {
            PUT(HDRP(NEXT_BLKP(bp)), PACK(size_bp - req_size, 1));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(size_bp - req_size, 1));
            insert_node(NEXT_BLKP(bp));
        }
    }
    // else {
    //     // extend heap
    //     if ((bp = extend_heap(req_size/WSIZE)) == NULL)
    //         return NULL;

    //     PUT(HDRP(bp), PACK(req_size, 1));
    //     PUT(FTRP(bp), PACK(req_size, 1));
    // }

    return bp;
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


int main() {
    char * bp = (char*)mem_sbrk(DSIZE*2);

    PUT(HDRP(bp), PACK(DSIZE*2, 1));
    PUT(FTRP(bp), PACK(DSIZE*2, 1));

    //dlist *headCast = (dlist*)bp;
    //headCast->prev = NULL;
    //headCast->next = NULL;

    //sep_list_head[0] = bp;

    // printf("prev: %d\n", (headCast->prev == NULL)?0:1);
    // printf("next: %d\n", (headCast->next == NULL)?0:1);

    char * bp2 = (char*)mem_sbrk(DSIZE*2);
    char * bp3 = (char*)mem_sbrk(DSIZE*2);

    PUT(HDRP(bp2), PACK(DSIZE*2, 1));
    PUT(FTRP(bp2), PACK(DSIZE*2, 1));

    PUT(HDRP(bp3), PACK(DSIZE*2, 1));
    PUT(FTRP(bp3), PACK(DSIZE*2, 1));

    //insert_node (bp);
    //insert_node (bp2);
    //insert_node (bp3);

    // dlist *bp2cast = (dlist*)bp2;
    // dlist *bp3cast = (dlist*)bp3;

    // printf("prev: %d\n", (bp2cast->prev == NULL)?0:1);
    // printf("next: %d\n", (bp2cast->next == NULL)?0:1);

    // printf("prev: %d\n", (bp3cast->prev == NULL)?0:1);
    // printf("next: %d\n", (bp3cast->next == NULL)?0:1);

    // remove_node (0,bp);
    // printf("Remove\n");

    // printf("prev: %d\n", (bp2cast->prev == NULL)?0:1);
    // printf("next: %d\n", (bp2cast->next == NULL)?0:1);

    // printf("prev: %d\n", (bp3cast->prev == NULL)?0:1);
    // printf("next: %d\n", (bp3cast->next == NULL)?0:1);

    return 0;
}