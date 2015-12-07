/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/


/*****************************************************************************
 * Parallel Game of life implementation
 ****************************************************************************/
// #define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#define DEBUG_PAUSE() do { printf("Continue?\n"); char c; scanf("%c\n", &c); } while(0)
#define DEBUG_CTR_INCR() do { pthread_mutex_lock(&count_lock); count++; pthread_mutex_unlock(&count_lock); } while(0)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PAUSE()
#define DEBUG_CTR_INCR()
#endif

#define NUM_THREADS 8
#define DIM 32
#define SET 4

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

typedef struct thread_data
{
    char *inboard;
    char *outboard;
    int nrows;
    int ncols;
    int LDA;
    int indx;
    struct thread_data *link; 
}t_data;

typedef struct task_queue {
    t_data *head;
    t_data *tail;
    int size;
}t_queue;

/* t_data Buffer functions */

t_data *
request_create(char *inboard, char *outboard, int nrows, int ncols, int lda, int indx) {
    t_data *req = malloc(sizeof(t_data));
    req->inboard = inboard;
    req->outboard = outboard;
    req->nrows = nrows;
    req->ncols = ncols;
    req->LDA = lda;
    req->indx = indx;
    req->link = NULL;
    return req;
}

void
request_modify(t_data *req, char *inboard, char *outboard, int nrows, int ncols, int lda, int indx) {
    req->inboard = inboard;
    req->outboard = outboard;
    req->nrows = nrows;
    req->ncols = ncols;
    req->LDA = lda;
    req->indx = indx;
    req->link = NULL;
}

void
request_clear(t_data *req) {
    req->link = NULL;
}

void
request_delete(t_data *req) {
    req->link = NULL;
    free(req);
}

/* QUEUE functions */

void
queue_destroy(t_queue *qp)
{
    t_data *curr = qp->head;
    t_data *head = qp->head;
    while(head != NULL) {
        curr = head;
        head = head->link;
        free(curr);
    }
    qp->head = NULL;
    qp->tail = NULL;
    qp->size = 0;
}

void
queue_print(t_queue *qp)
{
    t_data *curr = qp->head;
    while(curr != NULL) {
        // TODO: Print t_data here
        curr = curr->link;
    }
}

t_data*
queue_remove(t_queue *qp)
{
    if(qp != NULL && qp->head != NULL) {        
        t_data *pop = qp->head;
        qp->head = qp->head->link;
        if(qp->head == NULL)
            qp->tail = NULL;
        qp->size--;
        return pop;
    }
    return NULL;
}

int
queue_insert(t_queue *qp, t_data *put)
{
    if(qp != NULL) {        
        t_data *curr = qp->head;
        if(curr == NULL) //queue is empty
            qp->head = put;
        else
            qp->tail->link = put;

        qp->tail = put;
        qp->size++;
        return 1;
    }
    return 0;
}

// Locks
pthread_mutex_t buffer_lock;

// Barrier
pthread_barrier_t barrier_start;
pthread_barrier_t barrier_stop;

// Thread pool
pthread_t thread_pool[NUM_THREADS];
t_data new_data[NUM_THREADS];

// Buffer queue for requests
t_queue request_buffer;

int count;
pthread_mutex_t count_lock;

void 
process_board(t_data* data, int index)
{
    // t_data *data = (t_data*)arg;
    int rstart = index * data->LDA;
    int rstop = rstart + data->LDA;

    DEBUG_PRINT("consuming req: (%d) -> [%d, %d]\n", index, rstart, rstop);

    int r, c, ri;

    int rnorth;
    int rsouth;
    int cwest;
    int ceast;

    int w_mul, c_mul, e_mul;

    char neighbor_count;

    // for (ri = rstart; ri < rstop && ri < data->nrows; ri += DIM) 
    // {
        // rnorth = mod (r-1, data->nrows);
        // rsouth = mod (r+1, data->nrows);

        cwest = data->ncols - 1;
        ceast = 1;

        for (c = 0; c < data->ncols; c++)
        {
            //row_mul_i = i * data->LDA;
            w_mul = data->nrows * cwest;
            c_mul = data->nrows * c;
            e_mul = data->nrows * ceast;

            rnorth = (rstart == 0) ? data->nrows - 1 : rstart - 1;
            rsouth = (rstart == data->nrows - 1) ? 0 : rstart + 1;

            for (r = rstart; r < rstop && r < data->nrows; r++)
            {   
                neighbor_count = 
                    data->inboard [ rnorth + w_mul ] + 
                    data->inboard [ rnorth + c_mul ] + 
                    data->inboard [ rnorth + e_mul ] + 
                    data->inboard [ r + w_mul ] +
                    data->inboard [ r + e_mul ] + 
                    data->inboard [ rsouth + w_mul ] +
                    data->inboard [ rsouth + c_mul ] + 
                    data->inboard [ rsouth + e_mul ];

                data->outboard [ r + c_mul ] = alivep (neighbor_count, data->inboard [ r + c_mul ]);

                DEBUG_CTR_INCR();

                rnorth = (rnorth == data->nrows - 1) ? 0 : rnorth + 1;
                rsouth = (rsouth == data->nrows - 1) ? 0 : rsouth + 1;
            }

            cwest = (cwest == data->ncols - 1) ? 0 : cwest + 1;
            ceast = (ceast == data->ncols - 1) ? 0 : ceast + 1;
        }
    // }

    // const int LDA = data->nrows;

    // int i,j;

    // int jwest = data->ncols - 1;
    // int jeast = 1;

    // for (j = 0; j < data->ncols; j++)
    // {
    //     int inorth = (rstart == 0) ? data->nrows - 1 : rstart - 1;
    //     int isouth = (rstart == data->nrows - 1) ? 0 : rstart + 1;

    //     for (i = rstart; i < rstop && i < data->nrows; i++)
    //     {
    //         const char neighbor_count = 
    //                 BOARD (data->inboard, inorth, jwest) + 
    //                 BOARD (data->inboard, inorth, j) + 
    //                 BOARD (data->inboard, inorth, jeast) + 
    //                 BOARD (data->inboard, i, jwest) +
    //                 BOARD (data->inboard, i, jeast) + 
    //                 BOARD (data->inboard, isouth, jwest) +
    //                 BOARD (data->inboard, isouth, j) + 
    //                 BOARD (data->inboard, isouth, jeast);

    //         BOARD(data->outboard, i, j) = alivep (neighbor_count, BOARD (data->inboard, i, j));

    //         inorth = (inorth == data->nrows - 1) ? 0 : inorth + 1;
    //         isouth = (isouth == data->nrows - 1) ? 0 : isouth + 1;
    //     }

    //     jwest = (jwest == data->ncols - 1) ? 0 : jwest + 1;
    //     jeast = (jeast == data->ncols - 1) ? 0 : jeast + 1;
    // }
}

/* Worker thread main function */
void *
worker_stub(void *arg) {
    int thread_no = *((int *)arg);

    while(1) {
        pthread_barrier_wait(&barrier_start);
        // DEBUG_PRINT("Start: %d\n", thread_no);
        // pthread_mutex_lock(&buffer_lock);

        // // Remove from buff queue
        // t_data *ret = queue_remove(&request_buffer);
        // if(!ret)
        //     DEBUG_PRINT("Queue empty: Remove Error?!\n");

        // pthread_mutex_unlock(&buffer_lock);

        process_board(&new_data[thread_no], thread_no);
        request_clear(&new_data[thread_no]);

        pthread_barrier_wait(&barrier_stop);
        // DEBUG_PRINT("Stop: %d\n", thread_no);
    }
    return NULL;
}

char*
parallel_game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    const int LDA = nrows / NUM_THREADS;
    int curgen, i;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        for (i = 0; i < NUM_THREADS; i++)
        {
            // t_data *new_data = request_create(
            //     inboard, 
            //     outboard,
            //     nrows,
            //     ncols,
            //     LDA, 
            //     i);

            request_modify(
                &new_data[i],
                inboard, 
                outboard,
                nrows,
                ncols,
                LDA, 
                i);

            DEBUG_PRINT("creating req: (%d)\n", i);

            // pthread_mutex_lock(&buffer_lock);

            // Add to buff queue
            // int ret = queue_insert(&request_buffer, &new_data[i]);
            // if(!ret)
            //     DEBUG_PRINT("Insert Error?!\n");

            // pthread_mutex_unlock(&buffer_lock);
        }

        pthread_barrier_wait(&barrier_start);
        // DEBUG_PRINT("Start: main\n");
        pthread_barrier_wait(&barrier_stop);
        // DEBUG_PRINT("Stop: main\n");

        DEBUG_PAUSE();

        // DEBUG_PRINT("b4 swap\n");
        SWAP_BOARDS( outboard, inboard );
        // DEBUG_PRINT("aftr swap\n");

    }
    

    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return inboard;
}


/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    //return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
    // Create buffer queue for requests
    request_buffer.head = NULL;
    request_buffer.tail = NULL;
    request_buffer.size = 0;

    // /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&buffer_lock, NULL);

    pthread_barrier_init(&barrier_start, 0, NUM_THREADS + 1);
    pthread_barrier_init(&barrier_stop, 0, NUM_THREADS + 1);

    int err, i;
    int data[NUM_THREADS];

    // Create thread pool of nr_threads
    for(i = 0; i < NUM_THREADS; i++) {
        data[i] = i;
        err = pthread_create(&thread_pool[i], NULL, worker_stub, &data[i]);
        if(err)
            DEBUG_PRINT("Thread creation failed?!: %d\n", err);
    }

    count = 0;
    pthread_mutex_init(&count_lock, NULL);

    char *ret = parallel_game_of_life (outboard, inboard, nrows, ncols, gens_max);
    // char *ret = sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);

    DEBUG_PRINT("Count: %d\n", count);
    // free stuff

    return ret;
}