#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "value.h"
#include <assert.h>
#include <string.h>
#include "talloc.h"


#ifndef _LINKEDLIST
#define _LINKEDLIST

typedef struct Vector Vector;

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *null_value = talloc(sizeof(Value));
    (*null_value).type = NULL_TYPE;
    return null_value;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *cons_value = talloc(sizeof(Value));
    (*cons_value).type = CONS_TYPE;
    //newCdr->type = CONS_TYPE;
    (*cons_value).c.car = newCar;
    (*cons_value).c.cdr = newCdr;
    return cons_value;
}

// Display the contents of the linked list to the screen in some kind of
// readable format 
/* *list will be the head of the list, we want to traverse over every cons cell in the linked list,
    printing out the car of each, until we reach the end, which the cdr of the last cons will be NULL_TYPE
    */
void display(Value *list){
    //printf("displaying...\n");
    //while((*list).type != NULL_TYPE){ 
    switch(list->type){
        case INT_TYPE:
            printf("%i", list->i);
            break;
        case DOUBLE_TYPE:
            printf("%lf", list->d);
            break;
        case STR_TYPE:
            printf("%s", list->s);
            break;
        case NULL_TYPE:
            printf("()");
            break;
        case CONS_TYPE:
            //printf("(");
            while(list->type == CONS_TYPE){
                display(list->c.car);
                if(list->c.cdr->type != NULL_TYPE){
                    printf(" ");
                }
                list = list->c.cdr;
            }
            if(list->type != NULL_TYPE){
                printf(". ");
                display(list);
            }
            //printf(")");
            break;
        case PTR_TYPE:
            display(list->p);
            break;
        case OPEN_TYPE:
            printf("(");
            break;
        case CLOSE_TYPE:
            printf(")");
            break;
        case SYMBOL_TYPE:
            printf("%s", list->s);
            break;
        case BOOL_TYPE:
            if(list->i == 0){
                printf("#t");
            }else{
                printf("#f");
            }
            break;
        default: 
            //printf("NO ITEM\n");
            break;
    }
    //list = (*list).c.cdr;
    
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    assert(value != NULL && "Error (car): list is null");
    int counter = 0;
    Value *copy = value;
    if(copy->type != CONS_TYPE){
        return 1;
    }
    while(copy->type != NULL_TYPE){
        counter++;
        copy = copy->c.cdr;
    }
    return counter;
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
//
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list){
    //printf("start reverse");
    if(list->type == NULL_TYPE){
        printf("LIST IS NULL");
        return NULL;
    }
    Value *copy = makeNull();
    //int l = length(list);

    while(list->type != NULL_TYPE){
        Value *newValue = makeNull();
        //Value *newValue = talloc(sizeof(Value));
        //newValue = list->c.car;
        (*newValue).type = list->c.car->type;
        //Value *copy = list->c.car;
        char *t = list->c.car->s;

        switch((*(*list).c.car).type){
            case OPEN_TYPE:
                break;
            case CLOSE_TYPE:
                break;
            case INT_TYPE:
                (*newValue).i = (*(*list).c.car).i;
                break;
            case DOUBLE_TYPE:
                (*newValue).d = (*(*list).c.car).d;
                break;
            case STR_TYPE:
                (*newValue).s = talloc(sizeof(char)*(strlen(t)+1));
                strcpy((*newValue).s, t);
                break;
            case BOOL_TYPE:
                (*newValue).i = (*(*list).c.car).i;
                break;
            case SYMBOL_TYPE:
                (*newValue).s = talloc(sizeof(char)*(strlen(t)+1));
                strcpy((*newValue).s, t);
                break;
            case CONS_TYPE:
                newValue = list->c.car;
                break;
            case CLOSURE_TYPE:
                newValue->cl.functionCode = list->c.car->cl.functionCode;
                newValue->cl.paramNames = list->c.car->cl.paramNames;
                newValue->cl.frame = list->c.car->cl.frame;
                break;
            default:
                printf("");      
        }
        
        copy = cons(newValue, copy);
        list = (*list).c.cdr;
    }

    Value *revList = copy;
    //printf("reverse success");
    return revList;
        
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert(list != NULL && "Error (car): list is null");
    return (*list).c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list != NULL && "Error (car): list is null");
    return (*list).c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value != NULL && "Error (car): list is null");
    return (*value).type == NULL_TYPE;
}


#endif