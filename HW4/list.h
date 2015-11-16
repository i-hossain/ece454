
#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <pthread.h>

template<class Ele, class Keytype> class list;

template<class Ele, class Keytype> class list {
 private:
  Ele *my_head;
  unsigned long long my_num_ele;
#ifdef randtrack_list_lock
  pthread_mutex_t list_lock;
#endif
 public:
  list(){
    my_head = NULL;
    my_num_ele = 0;
#ifdef randtrack_list_lock
    pthread_mutex_init(&list_lock, NULL);
#endif
  }

  void setup();

  unsigned num_ele(){return my_num_ele;}

  Ele *head(){ return my_head; }
  Ele *lookup(Keytype the_key);

  void push(Ele *e);
  Ele *pop();
  void print(FILE *f=stdout);

  void cleanup();

#ifdef randtrack_list_lock
  void lock() { pthread_mutex_lock(&list_lock); }
  void unlock() { pthread_mutex_unlock(&list_lock); }
#endif
};

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::setup(){
  my_head = NULL;
  my_num_ele = 0;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::push(Ele *e){
  e->next = my_head;
  my_head = e;
  my_num_ele++;
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::pop(){
  Ele *e;
  e = my_head;
  if (e){
    my_head = e->next;
    my_num_ele--;
  }
  return e;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::print(FILE *f){
  Ele *e_tmp = my_head;

  while (e_tmp){
    e_tmp->print(f);
    e_tmp = e_tmp->next;
  }
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::lookup(Keytype the_key){
  Ele *e_tmp = my_head;
  
  while (e_tmp && (e_tmp->key() != the_key)){
    e_tmp = e_tmp->next;
  }
  return e_tmp;
}

template<class Ele, class Keytype> 
void
list<Ele,Keytype>::cleanup(){
  Ele *e_tmp = my_head;
  Ele *e_next;

  while (e_tmp){
    e_next = e_tmp->next;
    delete e_tmp;
    e_tmp = e_next;
  }
  my_head = NULL;
  my_num_ele = 0;
}

#endif
