/* listIteratorG.c ... Generic List Iterator Implementation
   Written by .... Mehri Amin (z5113067)
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "listIteratorG.h"

#define TRUE 1
#define FALSE 0

typedef struct Node {
  void *value;
  struct Node *next;
  struct Node *prev;

} Node;

typedef struct IteratorGRep {
  int nitems;
  Node *first;
  Node *last;
  Node *cPrev; // Cursor->Previous
  Node *cNext; // Cursor->Next

  Node *recentCall; // Tracks last function called. 

  ElmCompareFp cmp;
  ElmNewFp copy;
  ElmFreeFp free;

} IteratorGRep;

// Creates a new generic list iterator with given functions to handle values. //
IteratorG IteratorGNew(ElmCompareFp cmp, ElmNewFp copy, ElmFreeFp free){
  
  struct IteratorGRep * it; 

  it = malloc(sizeof(struct IteratorGRep));
  assert(it != NULL);
  it->nitems = 0;
  it->first = it->last = it->cPrev = it->cNext = NULL;
  
  it->recentCall = NULL;
  
  it->free = free;
  it->copy = copy;
  it->cmp = cmp;

  return it;
}



// Inserts elemment immediately before element that would be returned by next()
// and after element returned by previous(). New element is inserted before
// implicit cursor and thus a subsequent call to next is unaffected by a
// subsequent call to previous returns the new element. Return 1 if successful,
// otherwise 0.
int add(IteratorG it, void *vp){

  assert(it != NULL);
  Node *new = malloc(sizeof(Node));

  assert(new != NULL);

  new->value = it->copy(vp);

  if(it->nitems == 0){ // List is empty
    it->first = it->last = new;
    it->cNext = NULL;
    it->cPrev = new;
    new->next = new->prev = NULL;
  }
  else if(it->cPrev == NULL){ // Insert at beginning
    new->next = it->cNext;
    it->first->prev = new;
    it->first = new;
    it->cNext = new->next;
    it->cPrev = new;
  }
  else if(it->cNext == NULL){ // Insert at tail
    new->prev = it->last;
    it->last->next = new;
    new->next = NULL;
    it->last = new;
    it->cNext = new->next;
    it->cPrev = new;

  }else{ // In the middle
    new->prev = it->cPrev;
    new->next = it->cNext;
    it->cPrev = new;
    it->cNext = new->next;
  }

  it->nitems++;
  it->recentCall = NULL; // Delete or Set cannot be called after add. 
  return 1;

}

// Returns 1 if the given list iterator has more elements when traversing the
// list in the forward direction, returns 0 otherwise.
int hasNext(IteratorG it){
  assert(it != NULL);
  it->recentCall = NULL;
  return (it->cNext == NULL ? 0 : 1);

}

// Returns 1 if the given list iterator has more elements when traversing the
// list in the reverse direction, returns 0 otherwise.
int hadPrevious(IteratorG it){
  assert(it != NULL);
  it->recentCall = NULL;
  return (it->cPrev == NULL ? 0 : 1);

}

// Returns the pointer to the next value in the given list iterator and advances
// the cursor position. 
void * next(IteratorG it){
  assert(it != NULL);

  if(it->cNext == NULL) {
    return NULL;
  }else{
  // else
  it->cPrev = it->cNext;
  it->cNext = it->cNext->next;
  it->recentCall = it->cPrev; // Delete or Set can be called after this function.
  
  return it->cPrev->value; // returns next value
  }
}

// Returns the pointer to the previous value in the given list iterator and
// moves the cursor position backwards.
void * previous(IteratorG it){
  assert(it != NULL);
  if(it->cPrev == NULL) {
    return NULL;
  }else{
  // else
  it->cNext = it->cPrev; 
  it->cPrev = it->cPrev->prev;
  it->recentCall = it->cNext; // Delete or Set can be called after this function. 

  return it->cNext->value; // returns previous value
  }
}

// Deletes from the list iterator the last value that was returned by next or
// previous or findNext or findPrevious. 
// Precondition: A call to delete must be IMMEDIATELY preceded by a successful call
// to one of next or previous or findNext or findPrevious. 
// Returns 1 if successful, 0 otherwise (for example, invalid delete call).
int delete(IteratorG it){

  assert(it != NULL);
  
  // checks recent call is invalid
  if(it->recentCall == NULL){
    printf("Cannot call delete.\n");
    return 0;
  }
  // checks is able to delete
  else if(it->nitems == 0){
    printf("No items in list.\n");
    return 0;
  }
  else if(it->nitems == 1){
    it->first = it->last = it->cPrev = it->cNext = NULL;
  }
  // if last value returned was at head of list
  else if(it->recentCall == it->first){ // delete @ head
    it->first = it->first->next;
    it->first->prev = NULL;
    it->cNext = it->first;
    it->cPrev = NULL;
  }
  // if last value returned was at tail of list
  else if(it->recentCall == it->last){ // delete @ tail
    it->last = it->last->prev;
    it->last->next = NULL;
    it->cNext = NULL;
    it->cPrev = it->last;
  }
  // if last call was previous() or findPrev()
  else if(it->recentCall == it->cNext){
    it->recentCall->next->prev = it->recentCall->prev;
    it->recentCall->prev->next = it->recentCall->next;
    it->cNext = it->cNext->next;
 } 
 // if last call was next() or findNext()
  else if(it->recentCall == it->cPrev){
    it->recentCall->next->prev = it->recentCall->prev;
    it->recentCall->prev->next = it->recentCall->next;
    it->cPrev = it->cPrev->prev; 
  } 
  // update and free
  it->nitems--;
  it->free(it->recentCall->value);
  free(it->recentCall);
  it->recentCall = NULL; // update recent call 
  return 1;
  
}

// Replaces the last element returned by next or previous or findNext or
// findPrevious with the specified element (*vp). 
// Precondition: A call to set must be IMMEDIATELY preceded by a successful call to
// one of next or previous or findNext or findPrevious. 
// Returns 1 if successful, 0 otherwise (for example, invalid set call).
int set(IteratorG it, void * vp){
  // check last call
  if(it->recentCall == NULL){
    printf("Can't call set.\n");
    return 0;
  }
  // cant replace any element
  else if(it->nitems == 0){
    printf("No items to replace.\n");
    return 0;
  }
  // else
  it->free(it->recentCall->value);
  it->recentCall->value = it->copy(vp); // replace last element returned with vp
  it->recentCall = NULL; // update recent call 
  return 1;

}


// Returns pointer to the next value in it that matches the value pointed to by
// vp. and advances the cursor position past the value returned. 
void * findNext(IteratorG it, void * vp){
  
  // while theres more elements to iterate through in forward direction,
  // and value of next value does not equal value we want to find ...
  while(it->cNext != NULL && !(it->cmp(it->cNext->value, vp))){
    it->cNext = it->cNext->next; // iterate through
  }
  // if value found
  if(it->cmp(it->cNext->value, vp) == TRUE){
    it->recentCall = it->cNext; // update recent call
    it->cPrev = it->cNext; // advance position
    it->cNext = it->cNext->next;
    return it->cPrev->value; // return next value

  }else{
    return NULL;
  }

}

// Returns pointer to the previous value in it that matches the value pointed to
// by vp and moves the cursor position backwards before the value returned. 
void * findPrevious(IteratorG it, void * vp){
  // finding value in backward direction
  while(it->cPrev->prev != NULL && !(it->cmp(it->cPrev->value, vp))){
    it->cPrev = it->cPrev->prev;
  }
  // once found:
  if(it->cmp(it->cPrev->value, vp) == TRUE){
    it->recentCall = it->cPrev; // update recent call
    it->cNext = it->cPrev; // advance position
    it->cPrev = it->cPrev->prev;
    return it->cNext->value; // return previous value

  }else{
    return NULL;
  }
}

// Resets it to the start of the list.
void reset(IteratorG it){
 
  assert(it != NULL);
  // set iterator back to head of list.
  it->cNext = it->first;
  it->cPrev = NULL;
  it->recentCall = NULL;

}

// Deletes all nodes in it and frees associated memory.
void freeIt(IteratorG it){
  assert(it != NULL);
  // set iterator back to head of list
  it->cNext = it->first;
  // while not at end of list
  while(it->cNext != NULL){
    it->cPrev = it->cNext; // advance through list
    it->cNext = it->cNext->next;
    free(it->cPrev->value); // free previous value
    free(it->cPrev);
  }
  free(it); // free entire iterator
}

