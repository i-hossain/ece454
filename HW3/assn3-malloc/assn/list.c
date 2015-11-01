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

#define GET_NEXTV(bp) ((bp != 0)?(*(int *)(bp + WSIZE)):0)
#define GET_PREVV(bp) ((bp != 0)?(*(int *)(bp)):0)

#define GET_NEXTP(bp) ((int *)(bp) + WSIZE)
#define GET_PREVP(bp) ((int *)(bp))

#define SET_NEXTP(bp, p) (*(int *)(bp + WSIZE) = (intptr_t)p) 
#define SET_PREVP(bp, p) (*(int *)(bp) = (intptr_t)p)

int* head;

void insert (int *new_bp) {

	printf("IN INsert\n");
	printf("head: %8x\n", head);
	printf("%8x\n", GET_NEXTV(head));
	printf("%8x\n", GET_PREVV(head));

	printf("%8x\n", GET_NEXTP(head));
	printf("%8x\n", GET_PREVP(head));
    
  //   if (new_bp < head) {
  //       //insert before head
  //   	printf("IN first\n");

  //       SET_PREVP(head, new_bp);
  //       SET_NEXTP(new_bp, head);
  //       SET_PREVP(new_bp, 0);
  //       head = new_bp;
  //   }
  //   else {
  //       // insert at sorted location in list
  //       int* current = GET_NEXTP(head);

  //       printf("curr: %8x\n", *current);

  //       printf("%8x\n", GET_NEXTV(current));
		// printf("%8x\n", GET_PREVV(current));

  //       if (current == 0) {
  //           // one node
  //       	printf("IN one node\n");

  //           SET_NEXTP(head, new_bp);
  //           SET_PREVP(new_bp, head);
  //           SET_NEXTP(new_bp, 0);
  //       }

  //       while (GET_NEXTP(current) != 0 && new_bp > GET_NEXTP(current))
  //           current = GET_NEXTP(current);

  //       printf("IN last\n");

  //       SET_NEXTP(new_bp, GET_NEXTV(current));
  //       SET_PREVP(new_bp, current);
  //       SET_NEXTP(current, new_bp);
  //   }
}

void main() {
	int * bp = sbrk(sizeof(int *) * 2);
	printf("bp: %8x\n", bp);
	SET_NEXTP(bp, 0);
    SET_PREVP(bp, 0);

	printf("%8x\n", GET_NEXTV(bp));
	printf("%8x\n", GET_PREVV(bp));

	head = bp;

	int * bp2 = sbrk(sizeof(int *) * 2);
	printf("bp2: %8x\n", bp2);
	SET_NEXTP(bp2, 0);
    SET_PREVP(bp2, 0);

	printf("%8x\n", GET_NEXTV(bp2));
	printf("%8x\n", GET_PREVV(bp2));

	insert (bp2);

	printf("%8x\n", GET_NEXTV(bp));
	printf("%8x\n", GET_PREVV(bp));

	printf("%8x\n", GET_NEXTV(bp2));
	printf("%8x\n", GET_PREVV(bp2));

	//uintptr_t * nbp = malloc(sizeof(uintptr_t *) * 2);

	//insert (nbp);

	//free (bp2);
	//free (bp);
}