#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "assert.h"
#include <ctype.h>
#include <math.h>
#include "stdbool.h"
#include "talloc.h"
#include "tokenizer.h"

#ifndef _PARSER
#define _PARSER

//Prints the value based on Guile syntax
/*void printValue(Value *v){
    //printf("Printing");
    if(v->type == CONS_TYPE){
        printf("(");
        while(v->type != NULL_TYPE){
            printValue(v->c.car);
            if(v->c.cdr->type != NULL_TYPE){
                printf(" ");
            }
            v = v->c.cdr;
        }
        printf(")");
        
    }else if(v->type == NULL_TYPE){
        printf("");
    }else if(v->type == OPEN_TYPE){
        printf("(");
    }else if(v->type == CLOSE_TYPE){
        printf(")");
    }else if(v->type == STR_TYPE){
        printf("%s ", v->s);
    }else if(v->type == INT_TYPE){
        printf("%i", v->i);
    }else if(v->type == DOUBLE_TYPE){
        printf("%lf ", v->d);
    }else if(v->type == BOOL_TYPE){
        if(v->i == 0){
            printf("#t ");
        }else if(v->i == 1){
            printf("#f ");
        }
    }else if(v->type == SYMBOL_TYPE){
        printf("%s ", v->s);
    }else if(v->type == NULL_TYPE){
        //printf("");
    }else{
        printf("Error: This value is null.");
    }
}
*/

void printValue(Value *v){
    if(v->type == CONS_TYPE){
        printf("");
    }else if(v->type == NULL_TYPE){
        printf("");
    }else if(v->type == OPEN_TYPE){
        printf("(");
    }else if(v->type == CLOSE_TYPE){
        printf(") ");
    }else if(v->type == STR_TYPE){
        printf("%s ", v->s);
    }else if(v->type == INT_TYPE){
        printf("%i ", v->i);
    }else if(v->type == DOUBLE_TYPE){
        printf("%lf ", v->d);
    }else if(v->type == BOOL_TYPE){
        if(v->i == 0){
            printf("#t ");
        }else if(v->i == 1){
            printf("#f ");
        }
    }else if(v->type == SYMBOL_TYPE){
        printf("%s ", v->s);
    }else if(v->type == NULL_TYPE){
        printf(" ");
    }else{
        printf("Error: This value is null.");
    }
}

// Stacks a value to the parse tree. If the value is a CLOSE_TYPE value, the method
// then pops off nodes of the tree until it hits an OPEN_TYPE value, putting those values
// in their own subtree, then stacks the subtree onto the original tree.
Value *addToParseTree(Value *tree, Value *token, int depth){
    //printValue(token);
    tree = cons(token, tree);

    Value *subTree = makeNull();
    if(token->type == CLOSE_TYPE){
        //printf("MAKING SUBTREE...\n");
        tree = tree->c.cdr;
        while(tree->c.car->type != OPEN_TYPE){
            subTree = cons(tree->c.car, subTree);
            if(tree->c.cdr->type != NULL_TYPE){
                tree = tree->c.cdr;
            }else{
                printf("Syntax error: too many close parens\n");
                texit(0); 
            }
        }
        //subTree = cons(tree->c.car, subTree);
        //subTree->type = CONS_TYPE;
        tree = tree->c.cdr;
        tree = cons(subTree, tree);
    }
    return tree;
}

// Print a parse tree to the screen in a readable fashion. It should look 
// just like Scheme code (use parentheses to mark subtrees).
void printTree(Value *tree, bool sub){
    //printf("Printing tree...\n");
    if(tree == NULL){
        printf("Syntax error10");
        return;
    }
    while(tree->type != NULL_TYPE){
        //printf("1");
        if(tree->c.car->type == CONS_TYPE){
            printf("(");
            printTree(tree->c.car, true);
        }else if(tree->c.car->type != NULL_TYPE){
            //printf("Printing value...\n");
            printValue(tree->c.car);
        }
        tree = tree->c.cdr;
    }
    if(sub){
        printf(")");
    }
    
    /*while(tree->type != NULL_TYPE){
        printValue(tree->c.car);
        printf(" ");
        tree = tree->c.cdr;
    }
    */
}

// Return a pointer to a parse tree representing the structure of a Scheme 
// program, given a list of tokens in the program.
Value *parse(Value *tokens){
    assert(tokens != NULL && "Error (parse): null pointer");

    Value *tree = makeNull();

    int depth = 0;

    Value *current = tokens;

    //printf("Parsing...\n");

    //display(tokens);

    //printf("Parsing...\n");

    while(current->type != NULL_TYPE){
        Value *token = car(current);
        tree = addToParseTree(tree, token, depth);
        if(token->type == OPEN_TYPE){
            //("parse(): OPEN\n");
            depth += 1;
        }else if(token->type == CLOSE_TYPE){
            //printf("parse(): CLOSE\n");
            depth -= 1;
        }
        current = cdr(current);
    }
    if(depth > 0) {
        printf("Syntax error: too few close parens\n");
        texit(0);
    }else if(depth < 0){
        printf("Syntax error: too many close parens\n");
        texit(0);
    }
    //displayTokens(tree);
    //printTree(tree);
    //printTree(reverse(tree), false);
    //printf("Parse complete\n");
    return reverse(tree);
}

#endif
