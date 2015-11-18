
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define randtrack_reduction

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

// #define DEBUG_BUILD

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Team Name",                  /* Team name */

    "Rushab Ramesh Kumar",              /* First member full name */
    "1000117919",                       /* First member student number */
    "rushab.kumar@mail.utoronto.ca",    /* First member email address */

    "Ismail Hossain",                   /* Second member full name */
    "998340175",                           /* Second member student number */
    "ridoy.hossain@mail.utoronto.ca"    /* Second member email address */
};

pthread_mutex_t global_lock;

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> big_h;

typedef struct thread_data
{
  int thread_id;
  int start;
  int end;
  hash<sample,unsigned> h;
}t_data;

void *process_streams(void *arg) {
  t_data* data = (t_data*) arg;

  data->h.setup(14);
  int i,j,k;
  int rnum;
  unsigned key;
  sample *s;

#ifndef DEBUG_BUILD
  printf("This is thread: %d\n", data->thread_id);
#endif

  // process streams starting with different initial numbers
  for (i = data->start; i < data->end; i++){
    rnum = i;

    // collect a number of samples
    for (j=0; j<SAMPLES_TO_COLLECT; j++){

      // skip a number of samples
      for (k=0; k<samples_to_skip; k++){
        rnum = rand_r((unsigned int*)&rnum);
      }

      // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
      key = rnum % RAND_NUM_UPPER_BOUND;

      // pthread_mutex_lock(&global_lock);

      // if this sample has not been counted before
      if (!(s = data->h.lookup(key))){
  
        // insert a new element for it into the hash table
        s = new sample(key);
        data->h.insert(s);
      }

      // increment the count for the sample
      s->count++;

      // pthread_mutex_unlock(&global_lock);
    }
  }
}

int  
main (int argc, char* argv[]){
  int i;

#ifndef DEBUG_BUILD
  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );
#endif

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);  
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  // initialize a 16K-entry (2**14) hash of empty lists
  big_h.setup(14);

  //loop through threads, create pthreads, call process stream

  pthread_t *threads;
  threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);


  if (pthread_mutex_init(&global_lock, NULL) != 0)
    {
        printf("\n global_lock init failed\n");
        return 1;
    }

  t_data *new_data;
  new_data = new t_data[num_threads];

  int ems = NUM_SEED_STREAMS / num_threads;
  int start = 0;

  for (i = 0; i < num_threads; i++){

    //t_data *new_data = (t_data*) malloc(sizeof(t_data));
    new_data[i].thread_id = i;
    new_data[i].start = start;
    new_data[i].end = start + ems;

    pthread_create(&threads[i], NULL, process_streams, (void*)&new_data[i]);
    
    start = start + ems;
  }

  for (i = 0; i < num_threads; i++){
    pthread_join(threads[i], NULL);
  }

  for (i = 0; i < num_threads; i++){
    new_data[i].h.merge(&big_h);
  }
  // print a list of the frequency of all samples
  big_h.print();
}
