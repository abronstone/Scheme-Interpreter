#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "assert.h"
#include "stdbool.h"
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"
#include "parser.h"
#include "if.h"
#include "let.h"
#include "errors.h"

#ifndef _INTERPRETER
#define _INTERPRETER

Value *eval(Value *tree, Frame *f);

void evaluationError(int x, Value *v);

Value *evalEach(Value *args, Frame *frame);

Frame *topFrame = NULL;

void copyContents(Value *destination, Value *source){
    destination->type = source->type;
    switch(source->type){
        case INT_TYPE:
            destination->i = source->i;
            break;
        case DOUBLE_TYPE:
            destination->d = source->d;
            break;
        case STR_TYPE:
            destination->s = source->s;
            break;
        case BOOL_TYPE:
            destination->i = source->i;
            break;
        case SYMBOL_TYPE:
            destination->s = source->s;
            break;
        case CLOSURE_TYPE:
            destination->cl = source->cl;
            break;
        case PRIMITIVE_TYPE:
            destination->pf = source->pf;
            break;
        default:
            break;
    }
}

//Prints a value within the guilelines of Guile/Scheme output.
void printIValue(Value *v){
    if(v->type == CONS_TYPE){
        //printf("(");
        //printTree(v);
        //printf("");
        printf("(");
        while(v->type == CONS_TYPE){
            printIValue(v->c.car);
            if(v->c.cdr->type != NULL_TYPE){
                printf(" ");
            }
            v = v->c.cdr;
        }
        //printValueType(v);
        if(v->type != NULL_TYPE){
            printf(". ");
            printIValue(v);
        }
        printf(")");
    }else if(v->type == NULL_TYPE){
        printf("()");
    }else if(v->type == OPEN_TYPE){
        printf("(");
    }else if(v->type == CLOSE_TYPE){
        printf(")");
    }else if(v->type == STR_TYPE){
        printf("%s", v->s);
    }else if(v->type == INT_TYPE){
        printf("%i", v->i);
    }else if(v->type == DOUBLE_TYPE){
        printf("%lf", v->d);
    }else if(v->type == BOOL_TYPE){
        if(v->i == 0){
            printf("#t");
        }else if(v->i == 1){
            printf("#f");
        }
    }else if(v->type == SYMBOL_TYPE){
        printf("%s", v->s);
    }else if(v->type == NULL_TYPE){
        printf("NULL");
    }else if(v->type == CLOSURE_TYPE){
        printf("#<procedure>: ");
        if(v->cl.functionCode == NULL){
            printf("functionCode null, ");
        }
        if(v->cl.paramNames == NULL){
            printf("paramNames null,");
        }
        //printIValue(v->cl.functionCode);
    }else if(v->type == PRIMITIVE_TYPE){
        printf("#<builtInProcedure>");
    }else{
        //printf("Error: This value is null.\n");
    }
    //printf("printIValue(): END\n");
    //printf("\n");
}

// Prints out the type of the parametized value
// USED FOR DEBUGGING PURPOSES
void printValueType(Value *v){
    printf("Value type: ");
    if(v == NULL){
        printf("Error: null\n"); 
        return;  
    }
    switch(v->type){
        case INT_TYPE:
            printf("INT\n");
            break;
        case DOUBLE_TYPE:
            printf("DOUBLE\n");
            break;
        case STR_TYPE:
            printf("STR\n");
            break;
        case CONS_TYPE:
            printf("CONS\n");
            break;
        case NULL_TYPE:
            printf("NULL\n");
            break;
        case OPEN_TYPE:
            printf("OPEN\n");
            break;
        case CLOSE_TYPE:
            printf("CLOSE\n");
            break;
        case PTR_TYPE:
            printf("PTR\n");
            break;
        case BOOL_TYPE:
            printf("BOOL\n");
            break;
        case SYMBOL_TYPE:
            printf("SYMBOL\n");
            break;
        case CLOSURE_TYPE:
            printf("CLOSURE\n");
            break;
        case PRIMITIVE_TYPE:
            printf("PRIMITIVE\n");
            break;
        default:
            printf("Value type not found.\n");
    }
}

// Prints out the contents of a certain frame, including its
// children frames (not the parent frames).

// USED FOR DEBUGGING
void printFrame(Frame *f, int sub){
    Value *bindings = f->bindings;
    printf("\nFRAME # %i:\n", sub);
    if(f == NULL){
        printf("FRAME PRINT ERROR\n");
        return;    
    }
    if(bindings->type == NULL_TYPE){
        printf("Frame # %i is null\n", sub);
    }
    while(bindings->type != NULL_TYPE){
        /*if(bindings->c.car->c.car->type == CONS_TYPE){
            printFrame(bindings->c.car->c.car, sub+1);
        }
        */
        if(bindings->c.car->c.car->type == SYMBOL_TYPE){
            printf("SYMBOL: %s -- ", bindings->c.car->c.car->s);
            printf("VALUE: ");
            if(bindings->c.car->c.cdr->c.car->type == CONS_TYPE){
                printf("\nSUBFRAME: ");
                Frame *z = talloc(sizeof(Frame));
                z->parent = f;
                z->bindings = bindings->c.car->c.cdr->c.car;
                printFrame(z, sub+1);
                printf("END OF SUBFRAME\n");
            }else{
                printIValue(bindings->c.car->c.cdr->c.car);
            }
            
            //printIValue(bindings->c.car->c.cdr->c.car);
            printf("\n");
        }
        bindings = bindings->c.cdr;
    }
    if(f->parent != NULL){
        printf("PARENT: \n");
        printFrame(f->parent, sub-1);
    }
}

void evalTree(Value *args, Frame *frame){
    Value *tree = args;
    while(tree->type != NULL_TYPE){
        eval(tree->c.car, frame);
        tree = tree->c.cdr;
    }
    return;
    
}

// Searches through the parametized frame and all parent frames for
// the symbol passed through Value *tree. If the symbol exists within
// the frame, the Value that the symbol points to is returned.
Value *lookUpSymbol(Value *tree, Frame *frame){
    Frame *curr = frame;
    Value *bind;
    while(curr != NULL){
        bind = curr->bindings;
        while(bind->type != NULL_TYPE){
            if(strcmp(bind->c.car->c.car->s, tree->s) == 0){
                return bind->c.car->c.cdr->c.car;
            }
            bind = bind->c.cdr;
        }
        curr = curr->parent;
    }
    return NULL;
}

// Adds all of the integers/doubles contained in the 
// parametized list
Value *builtInAdd(Value *args){
    double d = 0.0;
    bool rD = false;
    Value *ret = talloc(sizeof(Value));
    ret->type = INT_TYPE;
    while(args->type != NULL_TYPE){
        Value *comp = args->c.car;
        if(comp->type == INT_TYPE){
            d += comp->i;
        }else if(comp->type == DOUBLE_TYPE){
            rD = true;
            d += comp->d;
        }else{
            evaluationError(100, NULL);
            return NULL;
        }
        args = args->c.cdr;
    }
    if(rD){
        ret->type = DOUBLE_TYPE;
        ret->d = d;
    }else{
        ret->i = (int)d;
    }
    return ret;
}

// Subtracts the subsequent integers/doubles contained in 
// the parametized list from the first integer/double
Value *builtInSubtract(Value *args){
    double d = 0.0;
    bool rD = false;
    Value *arg = args;
    if(arg->c.car->type == INT_TYPE){
        d = args->c.car->i;
    }else if(arg->c.car->type == DOUBLE_TYPE){
        d = arg->c.car->d;
        rD = true;
    }
    arg = arg->c.cdr;
    Value *ret = talloc(sizeof(Value));
    ret->type = INT_TYPE;
    while(arg->type != NULL_TYPE){
        Value *comp = arg->c.car;
        if(comp->type == INT_TYPE){
            d -= comp->i;
        }else if(comp->type == DOUBLE_TYPE){
            rD = true;
            d -= comp->d;
        }else{
            evaluationError(100, NULL);
            return NULL;
        }
        arg = arg->c.cdr;
    }
    if(rD){
        ret->type = DOUBLE_TYPE;
        ret->d = d;
    }else{
        ret->i = (int)d;
    }
    return ret;
}

// Multiplies all of the integers/doubles contained
// in the parametized list
Value *builtInStar(Value *args){
    double d = 1.0;
    bool rD = false;
    Value *ret = talloc(sizeof(Value));
    ret->type = INT_TYPE;
    while(args->type != NULL_TYPE){
        Value *comp = args->c.car;
        if(comp->type == INT_TYPE){
            d *= comp->i;
        }else if(comp->type == DOUBLE_TYPE){
            rD = true;
            d *= comp->d;
        }else{
            evaluationError(100, NULL);
            return NULL;
        }
        args = args->c.cdr;
    }
    if(rD){
        ret->type = DOUBLE_TYPE;
        ret->d = d;
    }else{
        ret->i = (int)d;
    }
    return ret;
    return NULL;
}

// Divides the subsequent integers/doubles in order
// from the first integer/double in the parametized list
Value *builtInDivide(Value *args){
    double d;
    Value *arg = args;
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    
    if(arg->c.car->type == INT_TYPE){
        d = arg->c.car->i;
    }else if(arg->c.car->type == DOUBLE_TYPE){
        d = arg->c.car->d;
    }
    
    arg = arg->c.cdr;
    
    if(arg->c.car->type == INT_TYPE){
        d = d / arg->c.car->i;
    }else if(arg->c.car->type == DOUBLE_TYPE){
        d = d / arg->c.car->d;
    }

    if((int)d == d){
        ret->type = INT_TYPE;
        ret->i = (int)d;
    }else{
        ret->d = d;
    }
    
    return ret;
}

// Returns the remainder after dividing the first 
// integer/double in the parametized list by the
// second integer/double
Value *builtInMod(Value *args){
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    
    double mult;
    if(args->c.cdr->c.car->type == INT_TYPE){
        mult = args->c.cdr->c.car->i;
    }else{
        mult = args->c.cdr->c.car->d;
    }
    double max;
    if(args->c.car->type == INT_TYPE){
        max = args->c.car->i;
    }else{
        max = args->c.car->d;
    }

    double f = 0.0;

    while(f < max){
        f += mult;
    }
    if(f > max){
        f -= mult;
    }

    double d = max - f;

    if((int)d == d){
        ret->type = INT_TYPE;
        ret->i = (int)d;
    }else{
        ret->d = d;
    }

    
    return ret;
}

// Returns a BOOL_TYPE Value with a true value if the first
// Value in the parametized list is less than the second
Value *builtInLess(Value *args){
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;

    double first = 0.0;
    double second = 0.0;

    if(args->c.car->type == INT_TYPE){
        first += args->c.car->i;
    }else{
        first += args->c.car->d;
    }
    if(args->c.cdr->c.car->type == INT_TYPE){
        second += args->c.cdr->c.car->i;
    }else{
        second += args->c.cdr->c.car->d;
    }

    if(first < second){
        ret->i = 0;
    }else{
        ret->i = 1;
    }
    return ret;
}

// Returns a BOOL_TYPE Value with a true value if the first
// Value in the parametized list is greater than the second
Value *builtInGreater(Value *args){
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;

    double first = 0.0;
    double second = 0.0;

    if(args->c.car->type == INT_TYPE){
        first += args->c.car->i;
    }else{
        first += args->c.car->d;
    }
    if(args->c.cdr->c.car->type == INT_TYPE){
        second += args->c.cdr->c.car->i;
    }else{
        second += args->c.cdr->c.car->d;
    }

    if(first > second){
        ret->i = 0;
    }else{
        ret->i = 1;
    }
    return ret;
}

// Returns a BOOL_TYPE Value with a true value if the first
// Value in the parametized list is equal to the second
Value *builtInEquals(Value *args){
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;

    double first = 0.0;
    double second = 0.0;

    if(args->c.car->type == INT_TYPE){
        first += args->c.car->i;
    }else{
        first += args->c.car->d;
    }
    if(args->c.cdr->c.car->type == INT_TYPE){
        second += args->c.cdr->c.car->i;
    }else{
        second += args->c.cdr->c.car->d;
    }

    if(first == second){
        ret->i = 0;
    }else{
        ret->i = 1;
    }
    return ret;
}

// Returns the car Value of the cdr of the parametized Value
Value *builtInCdr(Value *args){
    if(length(args) == 0 || args->type == NULL_TYPE){
        evaluationError(17, NULL);
        return NULL;
    }else if(length(args) > 1){
        evaluationError(18, NULL);
        return NULL;
    }
    if(args->c.car->type != CONS_TYPE){
        //printf("cdr(): ERROR\n");
        evaluationError(100, NULL);
        return NULL;
    }
    return args->c.car->c.cdr;
}

// Returns the car Value of the parametized Value
Value *builtInCar(Value *args){
    if(length(args) == 0 || args->type == NULL_TYPE){
        evaluationError(15, NULL);
        return NULL;
    }else if(length(args) > 1){
        evaluationError(16, NULL);
        return NULL;
    }
    if(args->c.car->type != CONS_TYPE){
        evaluationError(100, NULL);
        return NULL;
    }
    return args->c.car->c.car;
}

// Returns true if the parametized Value's type is NULL_TYPE
Value *builtInNull(Value *args){
    if(length(args) == 0 || args->type == NULL_TYPE){
        evaluationError(13, NULL);
        return NULL;
    }else if(length(args) > 1){
        evaluationError(14, NULL);
        return NULL;
    }
    Value *b = makeNull();
    b->type = BOOL_TYPE;
    if(args->c.car->type == NULL_TYPE){
        b->i = 0;
    }else{
        b->i = 1;
    }
    return b;
}

// Makes a new Value with the car and cdr being the car and cdr of 
// the parametized list, respectively
Value *builtInCons(Value *args){
    if(length(args)!=2){
        evaluationError(100, NULL);
    }else{
    }
    Value *val = makeNull();
    val = cons(args->c.car, args->c.cdr->c.car);
    return val;
}

Value *builtInAppend(Value *args);

// Prints out evaluation errors based on the parametized integer.
void evaluationError(int x, Value *v){
    switch(x){
        case 1:
            printf("Evaluation error: no consequent following an if.\n");
            break;
        case 2:
            printf("Evaluation error: symbol '%s' not found.\n", v->s);
            break;
        case 3:
            printf("Evaluation error: null binding in let.\n");
            break;
        case 4:
            printf("Evaluation error: no args following the bindings in let.\n");
            break;
        case 5:
            printf("Evaluation error: duplicate variable in let.\n");
            break;
        case 6:
            printf("Evaluation error: bad form in let.\n");
            break;
        case 7:
            printf("Evaluation error: no args following define.\n");
            break;
        case 8:
            printf("Evaluation error: no value following the symbol in define.\n");
            break;
        case 9:
            printf("Evaluation error: define must bind to a symbol.\n");
            break;
        case 10:
            printf("Evaluation error: no code in lambda following parameters.\n");
            break;
        case 11:
            printf("Evaluation error: formal parameters for lambda must be symbols.\n");
            break;
        case 12:
            printf("Evaluation error: no args following lambda.\n");
            break;
        case 13:
            printf("Evaluation error: no arguments supplied to null?\n");
            break;
        case 14:
            printf("Evaluation error: null? takes one argument.\n");
            break;
        case 15:
            printf("Evaluation error: no arguments supplied to car\n");
            break;
        case 16:
            printf("Evaluation error: car takes one argument\n");
            break;
        case 17:
            printf("Evaluation error: no arguments supplied to cdr\n");
            break;
        case 18:
            printf("Evaluation error: cdr takes one argument\n");
            break;
        case 19:
            printf("Evaluation error: letrec used a variable before all were computed.\n");
            break;

        default:
            printf("Evaluation error.\n");
    }
    texit(0);
}

// Evaluates 'let' statements by initializing variable arguments, then 
// assessing the content at the end of the statement.
Value *evalLet(Value *args, Frame *frame){
    Frame *f = talloc(sizeof(Frame));
    char **cF = talloc(sizeof(char)*300);
    int length = 0;
    f->parent = frame;
    f->bindings = makeNull();
    Value *arguments = args->c.car;
    Value *bindings = makeNull();
    while(arguments->type != NULL_TYPE){
        if(args->type != CONS_TYPE || args->c.car->type != CONS_TYPE || args->c.car->c.car->type == NULL_TYPE){
            evaluationError(3, args);
            return args;
        }

        if(args->c.car->c.car->type == INT_TYPE){
            evaluationError(6, args);
            return args;
        }

        if(arguments->c.car->c.car->type != SYMBOL_TYPE){
            evaluationError(100, args);
            return args;
        }

        assert(car(car(arguments))->type == SYMBOL_TYPE);

        if(length != 0){
            for(int i=0; i<length; i++){
                if(!strcmp(cF[i], arguments->c.car->c.car->s)){
                    evaluationError(5, args);
                    return args;
                }
            }
        }
        cF[length] = arguments->c.car->c.car->s;
        
        Value *val = cons(eval(arguments->c.car->c.cdr->c.car, frame), makeNull());
        val = cons(arguments->c.car->c.car, val);
        bindings = cons(val, bindings);
        arguments = cdr(arguments);
        f->bindings = bindings;
        
        
        length++;
    }
    f->bindings = bindings;
    Value *restOfArguments;
    if(args->c.cdr->type != NULL_TYPE){
        restOfArguments = args->c.cdr;
    }else{
        evaluationError(4, args);
        return args;
    }
    while(restOfArguments->c.cdr->type != NULL_TYPE){
        eval(restOfArguments, f);
        restOfArguments = restOfArguments->c.cdr;
    }
    return eval(restOfArguments->c.car, f);
}

// Evaluates 'let*' statements, and allows the use of
// pre-defined variables in the same frame 
Value *evalLetStar(Value *args, Frame *frame){ 
    char **cF = talloc(sizeof(char)*300);
    int length = 0;
    
    Frame *f = talloc(sizeof(Frame));
    f->parent = frame;
    f->bindings = makeNull();
    Value *arguments = args->c.car;
    Value *bindings = makeNull();
    
    while(arguments->type != NULL_TYPE){
        if(args->type != CONS_TYPE || args->c.car->type != CONS_TYPE || args->c.car->c.car->type == NULL_TYPE){
            //printf("NOT CONS TYPE\n");
            evaluationError(3, args);
            return args;
        }

        if(args->c.car->c.car->type == INT_TYPE || args->c.car->c.car->type == DOUBLE_TYPE){
            evaluationError(6, args);
            return args;
        }
        if(arguments->c.car->c.car->type != SYMBOL_TYPE){
            evaluationError(100, args);
            return args;
        }

        assert(car(car(arguments))->type == SYMBOL_TYPE);

        cF[length] = arguments->c.car->c.car->s;
        
        Frame *temp = talloc(sizeof(Frame));
        temp->parent = f;
        temp->bindings = makeNull();
        
        Value *val = cons(eval(arguments->c.car->c.cdr->c.car, f), makeNull());
        val = cons(arguments->c.car->c.car, val);
        bindings = cons(val, bindings);
        arguments = cdr(arguments);
        f = temp;
        f->bindings = bindings;
        
        
        length++;
    }
    f->bindings = bindings;
    Value *restOfArguments;
    if(args->c.cdr->type != NULL_TYPE){
        restOfArguments = args->c.cdr;
    }else{
        evaluationError(4, args);
        return args;
    }
    while(restOfArguments->c.cdr->type != NULL_TYPE){
        restOfArguments = restOfArguments->c.cdr;
    }
    return eval(restOfArguments->c.car, f);
}

// Evaluates 'letrec' statements, and allows for the definition
// of recursive functions within the let statement
Value *evalLetRec(Value *args, Frame *frame){
    char **cF = talloc(sizeof(char)*300);
    int length = 0;
    
    Frame *f = talloc(sizeof(Frame));
    f->parent = frame;
    Value *arguments = args->c.car;
    Value *bindings = makeNull();
    Value *list = makeNull();
    
    while(arguments->type != NULL_TYPE){
        if(args->type != CONS_TYPE || args->c.car->type != CONS_TYPE || args->c.car->c.car->type == NULL_TYPE){
            evaluationError(3, args);
            return args;
        }

        if(args->c.car->c.car->type == INT_TYPE || args->c.car->c.car->type == DOUBLE_TYPE){
            evaluationError(6, args);
            return args;
        }

        if(arguments->c.car->c.car->type != SYMBOL_TYPE){
            evaluationError(100, args);
            return args;
        }

        assert(car(car(arguments))->type == SYMBOL_TYPE);


        if(length != 0){
            for(int i=0; i<length; i++){
                if(!strcmp(cF[i], arguments->c.car->c.car->s)){
                    evaluationError(19, args);
                    return args;
                }
            }
        }
        
        cF[length] = arguments->c.car->c.car->s;
        if(arguments->c.car->c.cdr->c.car->type == SYMBOL_TYPE){
            length++;
            cF[length] = arguments->c.car->c.cdr->c.car->s;
        }
        
        list = cons(arguments->c.car->c.cdr->c.car, list);
        Value *newBind = makeNull();
        newBind = cons(arguments->c.car->c.car, newBind);
        bindings = cons(newBind, bindings);
        arguments = cdr(arguments);
        f->bindings = bindings;
        
        
        length++;
    }  
    bindings = reverse(bindings);
    f->bindings = bindings;
    list = reverse(list);
    list = evalEach(list, f);
    Value *name = bindings;
    Value *val = list;
    while(name->type != NULL_TYPE){
        name->c.car->c.cdr = cons(val->c.car, makeNull());
        name = name->c.cdr;
        val = val->c.cdr;
    }
    arguments = args->c.cdr;
    while(arguments->c.cdr->type != NULL_TYPE){
        arguments = arguments->c.cdr;
    }
    return eval(arguments->c.car, f);
}

// Changes the value of a variable accessible by the
// current frame
void evalSet(Value *args, Frame *frame){
    if(args->type == NULL_TYPE || length(args) != 2){
        evaluationError(100, NULL);
        return;
    }

    if(args->c.car->type != SYMBOL_TYPE){
        evaluationError(100, NULL);
        return;
    }
    eval(args->c.car, frame);

    Frame *search = frame;
    Value *bind = search->bindings;

    Value *result;


    while(true){
        if(bind->type == NULL_TYPE){
            search = search->parent;
            bind = search->bindings;
        }else{
            break;
        }
    }
    while(bind->type != NULL_TYPE){
        if(bind->c.car->c.car->type == SYMBOL_TYPE && !strcmp(bind->c.car->c.car->s, args->c.car->s)){
            bind->c.car->c.cdr->c.car = eval(args->c.cdr->c.car, frame);
            return;
        } 
        bind = bind->c.cdr;
        if(bind->type == NULL_TYPE){
            search = search->parent;
            bind = search->bindings;
        }
    }
    
}

// Takes zero or more arguments, evaluates each of them, 
// and returns the result of evaluating the final argument
Value *evalBegin(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        Value *v = talloc(sizeof(Value));
        v->type = VOID_TYPE;
        return v;
    }
    while(args->type != NULL_TYPE){
        if(args->c.cdr->type == NULL_TYPE){
            return eval(args->c.car, frame);
        }
        eval(args->c.car, frame);
        args = args->c.cdr;
    }
    return eval(args->c.car, frame);
}

// Returns a 'false' BOOL_TYPE value if one or more of the values
// in the parametized tree are false
Value *evalAnd(Value *args, Frame *frame){
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    ret->i = 0;
    Value *arg = args;
    Value *p = talloc(sizeof(Value));
    while(arg->type != NULL_TYPE){
        p = eval(arg->c.car, frame);
        if(p->type == BOOL_TYPE){
            if(p->i == 1){
                ret->i = 1;
                return ret;
            }
        }
        arg = arg->c.cdr;
    }
    return ret;
}

// Returns a 'true BOOL_TYPE' value if one or more of the values
// in the parametized tree are true
Value *evalOr(Value *args, Frame *frame){
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    ret->i = 1;
    Value *arg = args;
    Value *p = talloc(sizeof(Value));
    while(arg->type != NULL_TYPE){
        p = eval(arg->c.car, frame);
        if(p->type != BOOL_TYPE){
            ret->i = 0;
        }else{
            if(p->i == 0){
                ret->i = 0;
                return ret;
            }
        }
        arg = arg->c.cdr;
    }
    return ret;
}

// Evaluates a series of arguments given in the parametized list,
// and then returns the corresponding Value for the first argument
// that is truthy
Value *evalCond(Value *args, Frame *frame){
    
    int counter = 0;
    while(args->type != NULL_TYPE){
        if(args->c.car->c.car->type == SYMBOL_TYPE && !strcmp(args->c.car->c.car->s, "else")){
            return eval(args->c.car->c.cdr->c.car, frame);
        }
        Value *curr = eval(args->c.car->c.car, frame);
        if(curr->type != BOOL_TYPE || (curr->type == BOOL_TYPE && curr->i == 0)){
            return eval(args->c.car->c.cdr->c.car, frame);
        }
        args = args->c.cdr;
        counter++;
    }
    return NULL;


}

// Evaluates 'if' statements by analyzing the first argument, then returning
// the first preceding value if that first argument is true, and the second
// preceding value if the first argument is false. If the first argument is
// anything but #f, the first preceding value will be returned (anything
// else is analyzed as #t).
Value *evalIf(Value *args, Frame *frame){
    Value *returnValue = talloc(sizeof(Value));
    if(args->c.cdr->type == NULL_TYPE){
        evaluationError(1, args);
        texit(0);
        return NULL;
    }
    if(eval(args->c.car, frame)->type == BOOL_TYPE){
        int result = eval(args->c.car, frame)->i;
        if(result == 0){
            return eval(args->c.cdr->c.car, frame);
        }else if(result == 1){
            return eval(args->c.cdr->c.cdr->c.car, frame);
        }else{
            texit(0);
            return NULL;
        }
    }else{
        returnValue = args->c.cdr->c.car;
        return returnValue;
    }
    return NULL;
}

// Creates a lambda/closure function, with the parametized list containing
// the parameters for the new function
Value *evalLambda(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        evaluationError(12, NULL);
        return NULL;
    }
    if(args->c.cdr->type == NULL_TYPE){
        evaluationError(10, NULL);
        return NULL;
    }
    if(args->c.car->type == CONS_TYPE){
        if(length(args) > 2){
            Value *arg = args;
            int counter = 0;
            while(arg->c.cdr->type != NULL_TYPE){
                if(arg->c.car->c.car->type != SYMBOL_TYPE){
                    evaluationError(11, NULL);
                    return NULL;
                }
                arg = arg->c.cdr;
            }
            arg = arg->c.cdr;
        }
        Value *closure = talloc(sizeof(Value));
        closure->type = CLOSURE_TYPE;
        closure->cl.frame = frame;
        closure->cl.functionCode = args->c.cdr->c.car;
        closure->cl.paramNames = args->c.car;
        return closure;
    }else if(args->c.car->type == SYMBOL_TYPE){
        Value *closure = talloc(sizeof(Value));
        closure->type = CLOSURE_TYPE;
        closure->cl.frame = frame;
        closure->cl.functionCode = args->c.cdr->c.car;
        closure->cl.paramNames = cons(args->c.car, makeNull());
        return closure;
    }else if(args->c.car->type == NULL_TYPE){
        Value *closure = talloc(sizeof(Value));
        closure->type = CLOSURE_TYPE;
        closure->cl.frame = frame;
        closure->cl.functionCode = args->c.cdr->c.car;
        closure->cl.paramNames = makeNull();
        return closure;
    }else{
        return makeNull();
    }
}

// Evaluates each argument in the parametized list, and returns 
// a list with the evaluated arguments
Value *evalEach(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        return makeNull();
    }
    Value *list = makeNull();
    Value *arg = args;
    while(arg->type != NULL_TYPE){
        list = cons(eval(arg->c.car, frame), list);
        arg = arg->c.cdr;
    }
    return reverse(list);
}

// Evaluates a parametized function using the parametized
// arguments, and returns the resulting value
Value *apply(Value *function, Value *args, Frame *frame){
    Frame *f = talloc(sizeof(Frame));
    if(function->type == CLOSURE_TYPE){
        f->parent = function->cl.frame;
        f->bindings = makeNull();
        Value *paramName = function->cl.paramNames;
        Value *arg = args;
        if(args->type == NULL_TYPE){
        }
        if(function->cl.paramNames == NULL){
            evaluationError(100, NULL);
            return NULL;
        }
        if(length(function->cl.paramNames) < length(arg)){
            Value *valF = makeNull();
            valF = cons(args, valF);
            valF = cons(paramName->c.car, valF);
            f->bindings = cons(valF, f->bindings);
        }else{
            char **cF = talloc(sizeof(char)*300);
            int length = 0;
            while(paramName != NULL && paramName->type != NULL_TYPE){
                Value *val = makeNull();
                val = cons(arg->c.car, makeNull());
                val = cons(paramName->c.car, val);
                f->bindings = cons(val, f->bindings);
                arg = arg->c.cdr;
                paramName = paramName->c.cdr;
                length++;
            }
        }
        return eval(function->cl.functionCode, f);
    }else if(function->type == PRIMITIVE_TYPE){
        return function->pf(args);
    }else{
        printf("");
    }
    return makeNull();
}

// Evaluates 'define' statements, and creates a bind in the global
// frame between the name of the definition and the subsequent argument
void evalDefine(Value *args, Frame *frame){
    if(args->type == NULL_TYPE){
        evaluationError(7, NULL);
        return;
    }
    if(args->c.cdr->type == NULL_TYPE){
        evaluationError(8, NULL);
        return;
    }
    if(args->c.car->type != SYMBOL_TYPE){
        evaluationError(9, NULL);
        return;
    }

    if(args->c.car->type == SYMBOL_TYPE){
        Value *valF = cons(eval(args->c.cdr->c.car, frame), makeNull());
        valF = cons(args->c.car, valF);
        topFrame->bindings = cons(valF, topFrame->bindings);
    }else if(args->c.car->type == CONS_TYPE){
        Value *arg = args;
        while(arg->type != NULL_TYPE){
            if(arg->c.car->type != SYMBOL_TYPE){
                evaluationError(9, NULL);
            }
            arg = arg->c.cdr;
        }
        Value *p = cons(args->c.car->c.cdr, args->c.cdr);
        Value *valF = cons(evalLambda(p, frame), makeNull());
        valF = cons(args->c.car->c.car, valF);
        topFrame->bindings = cons(valF, topFrame->bindings);
    }
}

// Evaluates the parametized sequence of Values, and returns 
// a Value containing the result of the sequence of assessments.
Value *eval(Value *tree, Frame *frame) {
    Value *returnValue = makeNull();
    returnValue->type = VOID_TYPE;
    switch (tree->type){
        case INT_TYPE: 
            returnValue->type = INT_TYPE;
            returnValue->i = tree->i;
            break;
        case BOOL_TYPE:
            returnValue->type = BOOL_TYPE;
            returnValue->i = tree->i;
            break;
        case DOUBLE_TYPE:
            returnValue->type = DOUBLE_TYPE;
            returnValue->d = tree->d;
            break;
        case STR_TYPE:
            returnValue->type = STR_TYPE;
            returnValue->s = tree->s;
            break;
        case SYMBOL_TYPE:{
            Value *search = lookUpSymbol(tree, frame);
            if(search != NULL){
                returnValue = search;
            }else{
                evaluationError(2, tree);
            }
            break;
        }
        case CONS_TYPE:{
            Value *first = car(tree);
            Value *args = cdr(tree);
            if(first->type == BOOL_TYPE || first->type == INT_TYPE || first->type == DOUBLE_TYPE || first->type == STR_TYPE){
                returnValue = tree;
                break;
            }
            if(!strcmp(first->s, "if")){
                returnValue = evalIf(args, frame);
                break;
            }else if(!strcmp(first->s, "let")){
                returnValue = evalLet(args, frame);
                break;
            }else if(!strcmp(first->s, "let*")){
                returnValue = evalLetStar(args, frame);
                break;
            }else if(!strcmp(first->s, "letrec")){
                returnValue = evalLetRec(args, frame);
                break;
            }else if(!strcmp(first->s, "set!")){
                evalSet(args, frame);
                break;
            }else if(!strcmp(first->s, "begin")){
                returnValue = evalBegin(args, frame);
                break;
            }else if(!strcmp(first->s, "and")){
                returnValue = evalAnd(args, frame);
                break;
            }else if(!strcmp(first->s, "or")){
                returnValue = evalOr(args, frame);
                break;
            }else if(!strcmp(first->s, "cond")){
                returnValue = evalCond(args, frame);
                break;
            }else if(!strcmp(first->s, "else")){
                returnValue = args;
                break;
            }else if(!strcmp(first->s, "quote")){
                if(length(args) != 1){
                    evaluationError(100, NULL);
                }else{
                }
                returnValue = args->c.car;
                break;
                
            }else if(!strcmp(first->s, "define")){
                evalDefine(args, frame);
                returnValue->type = VOID_TYPE;
                break;
            }else if(!strcmp(first->s, "lambda")){
                returnValue = evalLambda(args, frame);
                returnValue->type = CLOSURE_TYPE;
                break;
            }else{
                Value *evaluatedOperator = eval(first, frame);
                Value *evaluatedArgs = evalEach(args, frame);
                returnValue = apply(evaluatedOperator, evaluatedArgs, frame);
                break;
            }
            
            
            
            break;
        }
        default:
            break;
    }  

    return returnValue;
}

void bind(char *name, Value *(*function)(struct Value*), Frame *frame){
    //Create the primitive type value for the function
    //printf("bind(): creating primitive for %s\n", name);
    Value *value = makeNull();
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    
    //Create the reference value
    //printf("bind(): creating reference for %s\n", name);
    Value *reference = makeNull();
    reference->type = SYMBOL_TYPE;
    reference->s = talloc(sizeof(char)*300);
    strcpy(reference->s, name);

    //Bind the reference to the primitive in the frame
    //printf("bind(): binding reference to primitive...\n");
    Value *val = makeNull();
    val = cons(value, makeNull());
    val = cons(reference, val);
    frame->bindings = cons(val, frame->bindings);
}

// Loops through the various functions defined in 'tree', and prints the
// result of each function.
void interpret(Value *tree){

    Value *evaluate;
    Frame *f = talloc(sizeof(Frame));
    f->bindings = makeNull();
    topFrame = talloc(sizeof(Frame));
    topFrame->parent = NULL;
    topFrame->bindings = makeNull();
    bind("car", &builtInCar, topFrame);
    bind("cdr", &builtInCdr, topFrame);
    bind("+", &builtInAdd, topFrame);
    bind("-", &builtInSubtract, topFrame);
    bind("*", &builtInStar, topFrame);
    bind("/", &builtInDivide, topFrame);
    bind("modulo", &builtInMod, topFrame);
    bind("<", &builtInLess, topFrame);
    bind(">", &builtInGreater, topFrame);
    bind("=", &builtInEquals, topFrame);
    bind("null?", &builtInNull, topFrame);
    bind("cons", &builtInCons, topFrame);
    f->parent = topFrame;
    while(tree->type != NULL_TYPE){
        evaluate = eval(tree->c.car, f);
        printIValue(evaluate);
        if(evaluate->type != VOID_TYPE){
            printf("\n");
        }
        tree = tree->c.cdr;
    }
}

#endif

