
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define randtrack_list_lock

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

#define DEBUG_BUILD

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
hash<sample,unsigned> h;

typedef struct thread_data
{
  int thread_id;
  int start;
  int end;
}t_data;

void *process_streams(void *arg) {
  t_data* data = (t_data*) arg;

  int i,j,k;
  int rnum;
  unsigned key;
  sample *s;
  list<sample,unsigned> *l;

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

      l = h.get_list(key, -1);
      l->lock();
      if (!(s = l->lookup(key))) {
        s = new sample(key);
        l->push(s);
      }
      s->count++;
      l->unlock();

      // h.incr_or_insert_if_absent(key);
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
  h.setup(14);

  //loop through threads, create pthreads, call process stream

  pthread_t *threads;
  threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);

  int ems = NUM_SEED_STREAMS / num_threads;
  int start = 0;

  for (i = 0; i < num_threads; i++){

    t_data *new_data = (t_data*) malloc(sizeof(t_data));
    new_data->thread_id = i;
    new_data->start = start;
    new_data->end = start + ems;

    pthread_create(&threads[i], NULL, process_streams, (void*)new_data);
    
    start = start + ems;
  }

  for (i = 0; i < num_threads; i++){
    pthread_join(threads[i], NULL);
  }

  // print a list of the frequency of all samples
  h.print();
}
