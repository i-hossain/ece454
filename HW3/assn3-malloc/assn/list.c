#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

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

int SEG_SIZES[] = {2, 3, 4, 5, 8, 16, 32};

typedef struct double_list
{
  struct double_list* prev;
  struct double_list* next;
}dlist;

uintptr_t* head;

void insert (uintptr_t *new_bp) {

	printf("IN INsert\n");

  dlist *headCast = (dlist*)head;
  dlist *bpCast = (dlist*)new_bp;
    
    if (new_bp < head) {
        //insert before head
    	printf("IN first\n");

      headCast->prev = bpCast;
      headCast->next = NULL;

      bpCast->prev = NULL;
      bpCast->next = headCast;

      head = new_bp;
    }
    else {
        // insert at sorted location in list
        dlist* current = headCast->next;

  //       printf("curr: %8x\n", *current);

  //       printf("%8x\n", GET_NEXTV(current));
		// printf("%8x\n", GET_PREVV(current));

        if (current == NULL) {
            // one node
        	  printf("IN one node\n");

            headCast->next = bpCast;

            bpCast->prev = headCast;
            bpCast->next = NULL;
            return;
        }

        while (current->next != NULL && bpCast > current->next)
            current = current->next;

        printf("IN last\n");

        bpCast->next = current->next;
        bpCast->prev = current;

        if (current->next != NULL)
          current->next->prev = bpCast;

        current->next = bpCast;
    }
}

void main() {
	uintptr_t * bp = sbrk(sizeof(int *) * 2);

  dlist *headCast = (dlist*)bp;
  headCast->prev = NULL;
  headCast->next = NULL;

	head = bp;

  printf("prev: %d\n", (headCast->prev == NULL)?0:1);
  printf("next: %d\n", (headCast->next == NULL)?0:1);

	uintptr_t * bp2 = sbrk(sizeof(int *) * 2);

	insert (bp2);

  dlist *bpcast = (dlist*)bp2;

  printf("prev: %d\n", (bpcast->prev == NULL)?0:((bpcast->prev == headCast)?1:2));
  printf("next: %d\n", (bpcast->next == NULL)?0:((bpcast->next == headCast)?1:2));
}