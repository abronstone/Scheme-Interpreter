#include <stdlib.h>
#include "value.h"
//#include "talloc.h"

#ifndef _TALLOC
#define _TALLOC

Value *activeList = NULL;

/*Value *makeNull(){
    Value *null_value = malloc(sizeof(Value));
    (*null_value).type = NULL_TYPE;
    return null_value;
}
*/

// Create a new CONS_TYPE value node.
Value *consTalloc(Value *newCar, Value *newCdr){
    Value *cons_value = malloc(sizeof(Value));
    (*cons_value).type = CONS_TYPE;
    (*cons_value).c.car = newCar;
    (*cons_value).c.cdr = newCdr;
    return cons_value;
}


// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in a linked list, and you have license here to duplicate code
// that you wrote for linkedlist.c. To be clear, don't actually call functions 
// that are defined in linkedlist.h, because then you'll end up with circular
// dependencies, since you'll be using talloc in linkedlist.c.
void *talloc(size_t size){
    Value *p = malloc(size);
    activeList = consTalloc(p, activeList);
    return p;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated for purposes of maintaining the active list. Hint: consider 
// that talloc may be called again after tfree is called...
void tfree(){
    while(activeList != NULL){
        Value *copy = (*activeList).c.cdr;
        /*if((*(*activeList).c.car).type == STR_TYPE){
            free(activeList->c.car->s);
        }
        */
        free((*activeList).c.car);
        free(activeList);
        activeList = copy;
    }
    activeList = NULL;
    
}

// Replacement for the C function 'exit' that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on, since you'll be able
// to call it to clean up memory and exit your program whenever an error occurs.
// Briefly look up exit to get a sense of what the 'status' parameter does.
void texit(int status){
    tfree();
    exit(0);
}

#endif
