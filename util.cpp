#include <string>
#include <vector>
#include <iostream>
#include "y.tab.h"
#include <string.h>

using namespace std;

// 這個程式是yacc 裡會用到的工具函數

extern int linenum;
void printWarning(string error){// 印出警告訊息
    cout<<"Warning: In line "<<linenum<<" has a warning: "<<error<<endl;
}
void printError(string error){// 印出錯誤訊息
    cout<<"Error: In line "<<linenum<<" has an error: "<<error<<endl;
}
void checkParameterSame(vector<int> table1, vector<int> table2){// 檢查兩個參數是否相同，用於檢查Function call 的參數是否正確

    if(table1.size() != table2.size()){
        printWarning("Parameter number mismatch");
    }
    else{
        for(int i = 0; i < table1.size(); i++){
            if(table1[i] != table2[i]){
                printWarning("Parameter type mismatch");
            }
        }
    }
}
void checkIdentifierSame(string name1, string name2){// 檢查兩個identifier是否相同
    if(name1.compare(name2) != 0){
        printWarning("Identifier mismatch");
    }
}
int checkTypeSame(int type1, int type2){// 檢查兩個type是否相同
    if(type1 != type2){
        printWarning("Type mismatch");
    }
    else{
        return type1;
    }
}
void checkAssignAvaliable(int type1, int typ2){ // 檢查assign是否相容
    if(type1 != typ2){
        printWarning("Types are not exactly the same");
    }
    switch(type1){
        case INT:
            if(typ2 == INT || typ2 == REAL){
                return;
            }
        case REAL:
            if(typ2 == INT || typ2 == REAL){
                return;
            }
        default:
            if(type1 == typ2){
                return;
            }
            else{
                printWarning("Assignment type mismatch");
            }
            break;
    }
}
int checkIntOrReal(char* numberString){ // 檢查一個string是int還是real的類別
    if(strchr(numberString, '.') != NULL){
        return REAL;
    }
    else{
        return INT;
    }
}
int checkUnaryOperation(int operater, int type){// 檢查一個unary operation的類別是否正確
    switch(operater){
        case MINUS:
            if(type == INT){
                return INT;
            }
            else if(type == REAL){
                return REAL;
            }
            else{
                printWarning("Type should be int or real");
            }
        break;
        case NOT:
            if(type == BOOL){
                return BOOL;
            }
            else{
                printWarning("Type sohuld be bool");
            }
        break;
        default:
            printWarning("Type mismatch");
    }
}
int checkTypeOperation(int type1, int operater, int type2){// 檢查一個binary operation的類別是否正確
    switch(operater){
        case PLUS:
        case MINUS:
        case TIMES:
        case DIV:
            if(type1 == INT && type2 == INT){
                return INT;
            }
            else if(type1 == REAL && type2 == REAL){
                return REAL;
            }
            else if(type1 == INT && type2 == REAL){
                printWarning("Type are not exactly the same.");
                return REAL;
            }
            else if(type1 == REAL && type2 == INT){
                printWarning("Type are not exactly the same.");
                return REAL;
            }
            else{
                printWarning("Type mismatch");
            }
        break;
        case MOD:
            if(type1 == INT && type2 == INT){
                return INT;
            }
            else{
                printWarning("Type mismatch");
            }
        break;
        case EQ:
        case NE:
            if(type1 == INT && type2 == INT){
                return BOOL;
            }
            else if(type1 == REAL && type2 == REAL){
                return BOOL;
            }
            else if(type1 == INT && type2 == REAL){
                printWarning("Type are not exactly the same.");
                return BOOL;
            }
            else if(type1 == REAL && type2 == INT){
                printWarning("Type are not exactly the same.");
                return BOOL;
            }
            else if(type1 == BOOL && type2 == BOOL){
                return BOOL;
            }
            else if(type1 == CHAR && type2 == CHAR){
                return BOOL;
            }
            else if(type1 == STRING && type2 == STRING){
                return BOOL;
            }
            else if(type1 == ARRAY && type2 == ARRAY){
                return BOOL;
            }
            else{
                printWarning("Type mismatch");
            }
            break;
        case LT:
        case LE:
        case GT:
        case GE:
            if(type1 == INT && type2 == INT){
                return BOOL;
            }
            else if(type1 == REAL && type2 == REAL){
                return BOOL;
            }
            else if(type1 == INT && type2 == REAL){
                printWarning("Type are not exactly the same.");
                return BOOL;
            }
            else if(type1 == REAL && type2 == INT){
                printWarning("Type are not exactly the same.");
                return BOOL;
            }
            else{
                printWarning("Type mismatch");
            }
            break;
        case AND:
        case OR:
            if(type1 == BOOL && type2 == BOOL){
                return BOOL;
            }
            else{
                printWarning("Type souhld be bool");
            }
        break;
        default:
            printWarning("Type mismatch");
            break;
    }
}