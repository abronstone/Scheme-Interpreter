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

#ifndef _TOKENIZER
#define _TOKENIZER 

//returns an 'OPEN_TYPE' Value
Value *isOpen(char nextChar){
    //printf("Character entered into isOpen: %c\n", nextChar);
    Value *newValue = talloc(sizeof(Value));
    //newValue->s = talloc(sizeof(char));
    newValue->type = OPEN_TYPE;
    //newValue->s = &nextChar;
    //printf("open token added\n");
    return newValue;
}

//returns a 'CLOSE_TYPE' Value
Value *isClose(char nextChar){
    Value *newValue = talloc(sizeof(Value));
    //newValue->s = talloc(sizeof(char));
    newValue->type = CLOSE_TYPE;
    //newValue->s = &nextChar;
    //printf("close token added\n");
    return newValue;
}

//returns a 'STRING_TYPE' value with a string in the s value, contained in quotes
Value *isString(char nextChar){
    char str[300] = "\"";
    int length = 1;
    nextChar = (char)fgetc(stdin);
    while(nextChar != '\"'){
        strncat(str, &nextChar, 1);
        nextChar = (char)fgetc(stdin);
        length++;
    }
    strncat(str, &nextChar, 1);
    length++;
    Value *newValue = talloc(sizeof(Value));
    newValue->s = talloc(sizeof(char)*300);
    newValue->type = STR_TYPE;
    strcpy(newValue->s, str);
    //printf("string token added\n");
    return newValue;
}

//returns a 'BOOLEAN_TYPE' Value with an integer value of 1 if the boolean
//is true and 0 if the boolean is false
Value *isBoolean(char nextChar){
    Value *newValue = talloc(sizeof(Value));
    newValue->type = BOOL_TYPE;
    nextChar = (char)fgetc(stdin);
    if(nextChar == 't'){
        newValue->i = 0;
    }else if(nextChar == 'f'){
        newValue->i = 1;
    }else{
        //printf("Bad boolean\n");
        return NULL;
    }
    //printf("boolean token added\n");
    return newValue;
}

//returns a 'DOUBLE_TYPE' Value
Value *isDouble(char sign, char curr[], char nextChar){
    //printf("current string: %s\n", curr);
    //printf("current char: %c\n", nextChar);
    //nextChar = (char)fgetc(stdin);
    //nextChar = (char)fgetc(stdin);
    //printf("Dcurrent string: %s\n", curr);
    //printf("Dcurrent char: %c\n", nextChar);
    strncat(curr, &nextChar, 1);
    nextChar = (char)fgetc(stdin);
    //printf("Zcurrent string: %s\n", curr);
    //printf("Zcurrent char: %c\n", nextChar);
    int counter = 1;
    while(isdigit(nextChar)){
        //printf("Adding %c to %s at position %i", nextChar, curr, counter);
        strncat(curr, &nextChar, 1);
        nextChar = (char)fgetc(stdin);
        counter++;
    }
    ungetc(nextChar, stdin);
    double d;
    for(int i=0; i<3; i++){
        //printf("Character in position %i in double: %c\n", i, curr[i]);
    }
    sscanf(curr, "%lf", &d);
    Value *newValue = talloc(sizeof(Value));
    newValue->type = DOUBLE_TYPE;
    if(sign == '-'){
        newValue->d = -d;
        //printf("Negative double token added\n");
    }else{
        newValue->d = d;
        //printf("Positive double token added\n");
    }
    //nextChar = (char)fgetc(stdin);
    return newValue;
}

//returns an 'INT_TYPE' character
Value *isInteger(char sign, char nextChar){
    char tempStr[300] = "";
    //nextChar = (char)fgetc(stdin);
    strncat(tempStr, &nextChar, 1);
    char previous;
    while(isdigit(nextChar)){
        //printf("current string: %s\n", tempStr);
        //printf("current char: %c\n", nextChar);
        previous = nextChar;
        nextChar = (char)fgetc(stdin);
        if(nextChar == '.'){
            //printf("Proceeding to make double...\n");
            //ungetc('.', stdin);
            //strncat(tempStr, &nextChar, 1);
            return isDouble(sign, tempStr, nextChar);
        }
        if(isdigit(nextChar)){
            strncat(tempStr, &nextChar, 1);
        }
        //nextChar = (char)fgetc(stdin);
    }
    ungetc(nextChar, stdin);
    int i;
    sscanf(tempStr, "%i", &i);
    //printf("complete number: %i\n", i);
    //printf("Current char after completion: %c", nextChar);
    Value *newValue = talloc(sizeof(Value));
    newValue->type = INT_TYPE;
    if(sign == '-'){
        newValue->i = -i;
        //printf("Negative Int token added\n");
    }else{
        newValue->i = i;
        //printf("Positive Int token added\n");
    }
    return newValue;
}

//returns a 'SYMBOL_TYPE' character
Value *isSymbol(char nextChar){
    char curr[300] = "";
    char* initials = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%%&*/:<=>?~";
    char* subsequents = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%%&*/:<=>?~0123456789.+-";
    Value *newValue = talloc(sizeof(Value));
    int length = 1;
    if(strchr(initials, nextChar) != NULL){
        //printf("INITIAL DETECTED: %c...\n", nextChar);
        strncat(curr, &nextChar, 1);
        //printf("After adding initial: %s\n", curr);
        nextChar = (char)fgetc(stdin);
        length++;
        while(strchr(subsequents, nextChar) != NULL){
            //printf("SUBSEQUENT DETECTED: %c...\n", nextChar);
            strncat(curr, &nextChar, 1);
            length++;
            nextChar = (char)fgetc(stdin);
        }
        ungetc(nextChar, stdin);
    }else if(nextChar == '+'){
        //printf("PLUS DETECTED\n");
        nextChar = (char)fgetc(stdin);
        if(isdigit(nextChar)){
            //printf("Proceeding to make positive int...\n");
            return isInteger('+', nextChar);
        }else{
            char plus = '+';
            strncat(curr, &plus, 1);
            ungetc(nextChar, stdin);
        }
    }else if(nextChar == '-'){
        nextChar = (char)fgetc(stdin);
        if(isdigit(nextChar)){
            //printf("Proceeding to make negative int...\n");
            return isInteger('-', nextChar);
        }else{
            char minus = '-';
            strncat(curr, &minus, 1);
            ungetc(nextChar, stdin);
        }
    }else{
        printf("Syntax error9");
        return NULL;
    }
    //printf("Resulting symbol string: -%s-\n", curr);
    newValue->s = talloc(sizeof(char)*300);
    newValue->type = SYMBOL_TYPE;
    strcpy(newValue->s, curr);

    //printf("Symb token added\n");
    return newValue;
}

Value *newLineValue(){
    Value *newLV = talloc(sizeof(Value));
    newLV->type = NULL_TYPE;
    return newLV;
}

// Display the contents of the list of tokens, along with associated type information.
// The tokens are displayed one on each line, in the format specified in the instructions.
void displayTokens(Value *list){
    //printf("PRINTING TOKENS...\n");
    if(list == NULL || list->type == NULL_TYPE){
        printf("Syntax error11\n");
        return;
    }
    while(list->type != NULL_TYPE){
        switch(list->c.car->type){
            case INT_TYPE:
                printf("%i:integer\n", list->c.car->i);
                break;
            case DOUBLE_TYPE:
                printf("%lf:double\n", list->c.car->d);
                break;
            case STR_TYPE:
                printf("%s:string\n", list->c.car->s);
                break;
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            case BOOL_TYPE:
                if(list->c.car->i == 0){
                    printf("#t:boolean\n");
                    
                }else{
                    printf("#f:boolean\n");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", list->c.car->s);
                break;
            case CONS_TYPE:
                //printf("DISPLAYING SUBTREE:\n");
                displayTokens(list->c.car);
                //printf("END OF SUBTREE\n");
                break;
            
            default:
                break;
        }
        list = list->c.cdr;
    }
}



// Read source code that is input via stdin, and return a linked list consisting of the
// tokens in the source code. Each token is represented as a Value struct instance, where
// the Value's type is set to represent the token type, while the Value's actual value
// matches the type of value, if applicable. For instance, an integer token should have
// a Value struct of type INT_TYPE, with an integer value stored in struct variable i.
// See the assignment instructions for more details.

Value *tokenize() {

    // Prepare list of tokens
    Value *tokensList = makeNull();

    //displayTokens(tokensList);

    // Prepare the character stream
    char nextChar;
    nextChar = (char)fgetc(stdin);
    int nP = 0;
    /*if(nextChar != ';' && nextChar != '(' && nextChar != '#' && !isdigit(nextChar) && nextChar != '\n' && !isspace(nextChar)){
        //printf("initial SE\n");
        //return NULL;
        //printf("Syntax error");
        printf("Syntax error");
        texit(0);
    }
    */
    if(nextChar == ')'){
        printf("Syntax error8");
        texit(0);
    }

    // Start tokenizing!
    while(nextChar != EOF) {
        //tokensList->type = CONS_TYPE;
        //int newLine = 0;
        nP = 0;
        //printf("CHARACTER TO READ: -%c-\n", nextChar);
        if(nextChar == ';'){
            while(nextChar != '\n'){
                nextChar = (char)fgetc(stdin);
            }
            //printf("Comment detected...\n");
        }else if(nextChar == '\n'){
            //printf("New line detected...\n");
            nP = 1;
            //tokensList = cons(newLineValue(), tokensList);
            nextChar = (char)fgetc(stdin);
            //printf("CHARACTER TO READ: -%c-\n", nextChar);
            /*if(nextChar != EOF && !isspace(nextChar) && nextChar != ';' && nextChar != '(' && nextChar != '#' && !isdigit(nextChar) && nextChar != '\n' && nextChar != ')'){
                //printf("tokenize syntax: -%c-\n", nextChar);
                //printf("newline SE\n");
                printf("Syntax error7");
                texit(0);
                //return NULL;
            }
            */
        }else if(isspace(nextChar) || nextChar == ' '){
            //printf("White space character detected...\n");
            //tokensList = cons(newLineValue(), tokensList);
        }else if(nextChar == '('){
            //printf("Making open type...\n");
            Value *open = isOpen(nextChar);
            if(open == NULL){
                //printf("open SE\n");
                //printf("System Syntax error");
                printf("Syntax error6");
                texit(0);
                //return NULL;
            }
            tokensList = cons(open, tokensList);
        }else if (nextChar == ')'){
            //printf("Making close type...\n");
            Value *close = isClose(nextChar);
            if(close == NULL){
                //printf("close SE\n");
                //printf("System Syntax error");
                printf("Syntax error5");
                texit(0);
                //return NULL;
            }
            tokensList = cons(close, tokensList);
        }else if(nextChar == '\"'){
            //printf("Making string type...\n");
            Value *str = isString(nextChar);
            if(str == NULL){
                //printf("str SE\n");
                //printf("System Syntax error");
                printf("Syntax error4");
                texit(0);
                //return NULL;
            }
            tokensList = cons(str, tokensList);
        }else if(isdigit(nextChar)){
            //printf("Making digit type...\n");
            Value *dig = isInteger('+', nextChar);
            if(dig == NULL){
                //printf("dig SE\n");
                //printf("System Syntax error");
                printf("Syntax error3");
                texit(0);
                //return NULL;
            }
            tokensList = cons(dig, tokensList);
        }else if(nextChar == '#'){
             //printf("Making boolean type...\n");
            Value *bo = isBoolean(nextChar);
            if(bo == NULL){
                //printf("bo SE\n");
                //printf("System Syntax error");
                printf("Syntax error2");
                texit(0);
                //return NULL;
            }
            tokensList = cons(bo, tokensList);
        }else if(nextChar != ' '){
            //printf("Making symbol type...\n");
            Value *symb = isSymbol(nextChar);
            if(symb == NULL){
                //printf("symb SE\n");
                printf("Syntax error1");
                texit(0);
                //return NULL;
            }
            tokensList = cons(symb, tokensList);
        }
        // Read next character
        if(tokensList->type == NULL_TYPE){
            //printf("NULL TYPE MADE\n");
        }

        //printf("\n\n");
        //displayTokens(tokensList);
        //printf("\n\n");
        //printf("Current Character: -%c-\n", nextChar);
        if(nP == 0){
            nextChar = (char)fgetc(stdin);
        }
        //printf("Next Character: -%c-\n", nextChar);
        
        //nextChar = (char)fgetc(stdin);
    }

    // Reverse the tokens list, to put it back in order
    Value *reversedList = reverse(tokensList);
    if(reversedList != NULL && reversedList->type == NULL_TYPE){
        //printf("NULL LIST\n");
    }
    //reversedList = reversedList->c.cdr;

    //printf("TOKENS:\n\n");
    //displayTokens(reversedList);
    //printf("\n\n");
    return reversedList;
}

#endif