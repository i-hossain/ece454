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

void insert (void *new_bp) {

	printf("Inserting node\n");

  dlist *new_node = (dlist*)new_bp;
  new_node->next = NULL;
  new_node->prev = NULL;

  if (head == NULL) {
    head = new_bp;
    return;
  }

  dlist *head_node = (dlist*)head;
    
  if (new_bp < head) {
    //insert before head
    printf("insert first\n");

    head_node->prev = new_node;
    new_node->next = head_node;
    head = new_bp;
    return;
  }

  dlist* current = head_node;

  if (current == NULL) {
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