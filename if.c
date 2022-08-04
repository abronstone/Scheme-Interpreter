#include "value.h"
#include "if.h"

#ifndef _IF
#define _IF

Value *evalIf(Value *tree, Frame *frame){
    printf("EVALIF\n");
    switch(tree->c.car->type){
        case CONS_TYPE:
            printf("IF: CONS\n");
            evalIf(tree->c.car, frame);
            break;
        case BOOL_TYPE:{
            printf("IF: BOOL\n");
            if(tree->c.car->i == 1){
                return tree->c.cdr->c.car;
            }else{
                return tree->c.cdr->c.cdr->c.car;
            }
        }
        default:
            printf("Default\n");
            return NULL;
    }
    return NULL;
}

#endif
