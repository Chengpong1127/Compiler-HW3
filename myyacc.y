%{


#include <stdio.h>
#define Trace(t)        cout<<"Contruct\t"<<t<<endl;
#include "y.tab.h"
#include "symbol.cpp"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "util.cpp"
#include <fstream>
#include "JavaGenerator.cpp"
using namespace std;

int linenum = 1;

extern "C"
{
    void yyerror(const char *s);
    extern int yylex(void);
}
extern FILE *yyin;
// 建立SymbolTableManager
SymbolTableManager symbolTableManager = SymbolTableManager();
// 建立FunctionTable，用來存放Function的parameters
vector<int> FunctionTable = vector<int>();
int FunctionTableIndex = 0;


// 檢查輸入的Identifier是否存在於SymbolTable中。如果存在，回傳其type，否則報錯。
int checkIdentifierExist(string name){
    if(symbolTableManager.containsSymbol(name)){
        return symbolTableManager.getSymbolType(name);
    }
    else{
        printWarning("Identifier " + name + " does not exist");
    }
}

void checkSymbolIsEditable(string name){
    if(symbolTableManager.containsSymbol(name)){
        if(symbolTableManager.getSymbolIsConsistent(name) == true){
            printWarning("Identifier " + name + " is constant");
        }
    }
}

auto generator = JavaGenerater("output.jasm", symbolTableManager);

%}

/* tokens */
// yylval 的型態為union(type, sval)，type為token的type，sval為token的值。
%union {
    int type;
    char* sval;
}

%token DOT COMMA COLON SEMICOLON LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token _BEGIN CONST DECREASING DEFAULT DO ELSE END EXIT FALSE
%token FOR FUNCTION GET IF LOOP OF PUT PROCEDURE RESULT RETURN SKIP THEN TRUE VAR WHEN ASSIGN
%token STRINGCONSTANT  
%token <sval> IDENTIFIER NUMERICALCONSTANT
%token <type> BOOL CHAR INT REAL STRING ARRAY 
%left <type> PLUS MINUS TIMES DIV AND OR 
%token <type> MOD EQ NE LT LE GT GE NOT
%type <type> Type SetType Factor Term Expression AssignExpression FunctionCall CompareOperater CompareExpression AndExpression OrExpression LeftValue ConstIdentifierSetType

%start Program
%%
Program:        // 程式的進入點
                StatementList
                ;
StatementList:  // 將所有的statement組合起來
                GeneralStatement
                | StatementList GeneralStatement
                ;
UniqueSymbolTableStatementList:
                {symbolTableManager.createSymbolTable();}
                StatementList
                {symbolTableManager.destroySymbolTable();}

GeneralStatement:// 一般的statement，包含FunctionDeclaration
                DeclareStatement  
                | FunctionCall      
                | StructStatement
                | SimpleStatement  
                |
                ; 
DeclareStatement: ConstantDeclaration
                | VariableDeclaration
                | FunctionDeclaration
                ;
FunctionStatement:// 一般的statement，不包含FunctionDeclaration，因為Function 中不允許再宣告Function
                ConstantDeclaration
                | VariableDeclaration
                | StructStatement
                | SimpleStatement
                | FunctionCall
FunctionStatementList:// 將所有的FunctionStatement組合起來
                FunctionStatement
                | FunctionStatementList FunctionStatement
                ;
StructStatement: // 組合的statement，如if, loop, for, block
                ConditionStatement
                | LoopStatement
                | ForStatement
                | Block
SimpleStatement: // 單一的statement，如put, get, result, return, exit, skip
                AssignmentStatement
                | PUTStatement
                | GETStatement
                | ResultStatement
                | RETURN
                | ExitStatement
                | SKIP

ConditionStatement:// if, if-else
                IF OrExpression THEN
                UniqueSymbolTableStatementList END IF
                {checkTypeSame($2, BOOL);}
                | IF OrExpression THEN
                UniqueSymbolTableStatementList ELSE UniqueSymbolTableStatementList END IF 
                {checkTypeSame($2, BOOL);}
                ;

LoopStatement:  // Loop Statement
                LOOP 
                UniqueSymbolTableStatementList 
                END LOOP 
                ;
ForStatement:   // For Statement，包含一般increasing的和decreasing
                FOR {symbolTableManager.createSymbolTable();}
                IDENTIFIER COLON OrExpression DOT DOT OrExpression {symbolTableManager.addSymbol($3, INT); checkTypeSame($5, INT); checkTypeSame($8, INT);}// 將IDENTIFIER加入SymbolTable中，並檢查OrExpression的type是否為INT
                StatementList
                END FOR {symbolTableManager.destroySymbolTable();}
                | FOR {symbolTableManager.createSymbolTable();}
                DECREASING IDENTIFIER COLON OrExpression DOT DOT OrExpression {symbolTableManager.addSymbol($4, INT); checkTypeSame($6, INT); checkTypeSame($9, INT);}
                StatementList
                END FOR {symbolTableManager.destroySymbolTable();}
                ;
PUTStatement:   PUT {
                    generator.PutInit();
                }
                OrExpression{
                    generator.PutEnd($3);
                }
                ;
GETStatement:   GET IDENTIFIER {checkIdentifierExist($2);} // 將輸入的值存入IDENTIFIER中
                ;
ExitStatement:  EXIT // 結束程式
                | EXIT WHEN OrExpression {checkTypeSame($3, BOOL);}
                ;

FunctionCall:   // FunctionCall Statement，
                IDENTIFIER { FunctionTable.clear(); }
                LPAREN ValueList RPAREN 
                {
                    checkIdentifierExist($1);// 檢查IDENTIFIER是否存在於SymbolTable中
                    auto table = symbolTableManager.getFunctionParameters($1);// 取得IDENTIFIER的parameters
                    checkParameterSame(table, FunctionTable); // 檢查valueList是否和記錄的parameters相同
                    FunctionTable.clear();// 清空FunctionTable，讓下一次使用時不會有上一次的資料
                    $$ = symbolTableManager.getFunctionReturnType($1);// 回傳Function call return的type
                }
                ;
ValueList:      // 這是用來將FunctionCall中的參數存入FunctionTable中
                OrExpression { FunctionTable.insert(FunctionTable.begin(),1,$1); }
                | OrExpression COMMA ValueList { FunctionTable.insert(FunctionTable.begin(),1,$1); }
                |
                ;
ResultStatement:
                RESULT OrExpression
                ;
FunctionDeclaration:// Function Declaration，包含Function和Procedure
                FUNCTION IDENTIFIER  {
                    symbolTableManager.createSymbolTable();// 建立新的SymbolTable
                    FunctionTable.clear();// 清空FunctionTable
                    }
                    LPAREN ParameterList RPAREN 
                    SetType 
                    FunctionStatementList 
                    END IDENTIFIER          
                    {
                        symbolTableManager.destroySymbolTable();// 刪除函數宣告的SymbolTable
                        checkIdentifierSame($2, $10);// 比對Function的名稱和END的名稱是否相同
                        symbolTableManager.addFunctionSymbol($2, FUNCTION, FunctionTable, $7);// 將Function的資訊存入SymbolTable中
                        FunctionTable.clear();// 清空FunctionTable，讓下一次使用時不會有上一次的資料
                    }

                | PROCEDURE IDENTIFIER      {
                    symbolTableManager.createSymbolTable();
                    FunctionTable.clear();
                    }
                    LPAREN ParameterList RPAREN 
                    FunctionStatementList 
                    END IDENTIFIER          {
                        symbolTableManager.destroySymbolTable(); 
                        checkIdentifierSame($2, $9);
                        symbolTableManager.addFunctionSymbol($2, PROCEDURE, FunctionTable, -1);
                        FunctionTable.clear();
                        }
                ;



ConstantDeclaration:// 常數宣告，宣告後會將型態存入SymbolTable中
                CONST IDENTIFIER AssignExpression {symbolTableManager.addSymbol($2, $3, true);}
                | CONST ConstIdentifierSetType AssignExpression { 
                    checkAssignAvaliable($2, $3);}
                ;
VariableDeclaration:// 變數宣告，宣告後會將型態存入SymbolTable中
                VAR IDENTIFIER AssignExpression {symbolTableManager.addSymbol($2, $3);}
                | VAR IDENTIFIER SetType 
                {
                    generator.VarDeclaration($2, $3);
                }
                | VAR IDENTIFIER SetType AssignExpression 
                {
                    generator.VarDeclaration($2, $3);
                    generator.AssignLocal($2);
                }
                ;
AssignmentStatement:// 賦值，將右邊的值賦予左邊的變數。會檢查左邊的變數是否存在於SymbolTable中，並檢查賦值的型態是否相同或是相容
                LeftValue AssignExpression 
                    {
                        checkAssignAvaliable($1, $2);
                    }
                ;
LeftValue:      // 用來檢查左邊的變數是否存在於SymbolTable中
                IDENTIFIER { $$ = checkIdentifierExist($1); checkSymbolIsEditable($1);}
                ;
AssignExpression:
                ASSIGN OrExpression         {
                    $$ = $2;
                    }
                ;
OrExpression:   // 這是Expression 的最高層，包含OrExpression, AndExpression, CompareExpression, Expression, Term, Factor，用來處理運算子的優先順序。每一層都會檢查型態是否相同或是相容，並回傳型態
                AndExpression               { $$ = $1;}
                | OrExpression OR AndExpression
                                            { $$ = checkTypeOperation($1, $2, $3);}
                ;
AndExpression:  CompareExpression           { $$ = $1;}
                | AndExpression AND CompareExpression
                                            { $$ = checkTypeOperation($1, $2, $3);}
                ;
CompareExpression:
                Expression                  { $$ = $1;}
                | NOT CompareExpression     { $$ = checkUnaryOperation($1, $2);}
                | CompareExpression CompareOperater Expression
                                            { $$ = checkTypeOperation($1, $2, $3);}
                ;
Expression:     Term                        { $$ = $1;}
                | Expression PLUS Term      { $$ = checkTypeOperation($1, $2, $3);}
                | Expression MINUS Term     { $$ = checkTypeOperation($1, $2, $3);}
Term:           Factor
                | Term TIMES Factor         { $$ = checkTypeOperation($1, $2, $3);}
                | Term DIV Factor           { $$ = checkTypeOperation($1, $2, $3);}
                | Term MOD Factor           { $$ = checkTypeOperation($1, $2, $3);}  
                ;
Factor:         MINUS Factor                { $$ = checkUnaryOperation($1, $2);}
                | LPAREN OrExpression RPAREN    { $$ = $2;}
                | IDENTIFIER                
                { 
                    $$ = checkIdentifierExist($1);
                    generator.LoadLocal($1);
                }
                | NUMERICALCONSTANT         
                { 
                    $$ = checkIntOrReal($1);
                    generator.Push($1);
                }
                | STRINGCONSTANT            { $$ = STRING;}
                | TRUE                      { $$ = BOOL;}
                | FALSE                     { $$ = BOOL;}
                | FunctionCall              { $$ = $1;}
                ;
ParameterList:  FunctionParameterSetType
                | ParameterList COMMA FunctionParameterSetType
                | 
                ;
ConstIdentifierSetType:
                IDENTIFIER SetType {symbolTableManager.addSymbol($1, $2, true); $$ = $2;
                }
                ;
FunctionParameterSetType:
                IDENTIFIER SetType {FunctionTable.push_back($2); symbolTableManager.addSymbol($1, $2);}
                
                ;
Type:           INT {$$ = INT;}
                | REAL {$$ = REAL;}
                | BOOL {$$ = BOOL;}
                | STRING {$$ = STRING;}
                ;
SetType:        COLON Type {$$ = $2;}
                ;

Block:          _BEGIN                      
                    UniqueSymbolTableStatementList 
                    END                     
                ;  
CompareOperater:
                EQ {$$ = $1;}
                | NE {$$ = $1;}
                | LT {$$ = $1;}
                | LE {$$ = $1;}
                | GT {$$ = $1;}
                | GE {$$ = $1;}
                ;   
%%


void yyerror(const char *s)
{
   printError(s);   // 印出錯誤訊息，並結束程式。會說明錯誤的行數和錯誤的原因。
}

int main(int argc, char **argv)
{

    
    

    /* open the source program file */
    if (argc != 2) {
        printf ("Usage: sc filename\n");
        exit(1);
    }
    yyin = fopen(argv[1], "r");         /* open input file */
    generator.ProgramInit(); 

    auto result = yyparse();

    generator.ProgramEnd();
    if (result == 1)                 /* parsing */
        cout<<"Parsing failed!"<<endl;
    else
        cout<<"Parsing success!"<<endl;
    cout<<endl;


    // DEBUG 時將SymbolTable印出來
    // cout<<"Dump symbol table:"<<endl;
    // symbolTableManager.dumpAll();
}