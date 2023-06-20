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
auto generator = JavaGenerater("output", symbolTableManager);
SymbolTable GlobalSymbolTable = generator.GetGlobalSymbolTable();


// 檢查輸入的Identifier是否存在於SymbolTable中。如果存在，回傳其type，否則報錯。
int checkIdentifierExist(string name){
    if(symbolTableManager.containsSymbol(name)){
        return symbolTableManager.getSymbolType(name);
    }
    else if(generator.GetGlobalSymbolTable().containsSymbol(name)){
        return generator.GetGlobalSymbolTable().getSymbolType(name);
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



%}

/* tokens */
// yylval 的型態為union(type, sval)，type為token的type，sval為token的值。
%union {
    int type;
    char* sval;
}

%token DOT COMMA COLON SEMICOLON LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE 
%token _BEGIN CONST DECREASING DEFAULT DO ELSE END EXIT FALSE VOID
%token FOR FUNCTION GET IF LOOP OF PUT PROCEDURE RESULT RETURN SKIP THEN TRUE VAR WHEN ASSIGN
%token <sval> IDENTIFIER NUMERICALCONSTANT STRINGCONSTANT
%token <type> BOOL CHAR INT REAL STRING ARRAY 
%left <type> PLUS MINUS TIMES DIV AND OR 
%token <type> MOD EQ NE LT LE GT GE NOT
%type <type> Type SetType Factor Term Expression AssignExpression FunctionCall CompareOperater CompareExpression AndExpression OrExpression 

%start Program
%%
Program:        // 程式的進入點
                GlobalDeclarationList
                FunctionDeclarationList
                MainBlock
                ;
GlobalDeclarationList:
                GlobalDeclaration
                | GlobalDeclarationList GlobalDeclaration
GlobalDeclaration:
                ConstantDeclaration
                | GlobalVariableDeclaration
                ;
FunctionDeclarationList:
                FunctionDeclaration
                | FunctionDeclarationList FunctionDeclaration
                ;
MainBlock:      StatementList
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
                | FunctionCall      
                | StructStatement
                | SimpleStatement  
                | VariableDeclaration
                ; 
DeclareStatement: ConstantDeclaration
                | VariableDeclaration
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
                | RETURN { generator.Return(); }
                | ExitStatement
                | SKIP { generator.Skip(); }

ConditionStatement:// if, if-else
                IF OrExpression THEN {
                    checkTypeSame($2, BOOL);
                    generator.IFInit();
                }
                
                StatementList
                ElseStatement  
                {
                    generator.IFEnd();
                }
                ;
ElseStatement:  ELSE {
                    generator.IFElse();
                }
                StatementList END IF
                | END {
                    generator.IFElse();
                }
                IF
                ;

LoopStatement:  // Loop Statement
                LOOP {
                    generator.LoopInit();
                }
                StatementList 
                END LOOP {
                    generator.LoopEnd();
                }
                ;
ForStatement:   // For Statement，包含一般increasing的和decreasing
                FOR IDENTIFIER COLON NUMERICALCONSTANT DOT DOT NUMERICALCONSTANT {
                    generator.ForInit($2, $4, $7);
                }// 將IDENTIFIER加入SymbolTable中，並檢查OrExpression的type是否為INT
                StatementList END FOR {
                    generator.ForEnd($2);
                }


                | FOR DECREASING IDENTIFIER COLON NUMERICALCONSTANT DOT DOT NUMERICALCONSTANT {
                    generator.ForInit($3, $5, $8, false);
                }// 將IDENTIFIER加入SymbolTable中，並檢查OrExpression的type是否為INT
                StatementList END FOR {
                    generator.ForEnd($3, false);
                }
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
ExitStatement:  EXIT {
                    generator.ExitLoop();
                }
                ;
                | EXIT WHEN OrExpression 
                {
                    checkTypeSame($3, BOOL);
                    generator.ExitLoopWhen();
                }
                ;

FunctionCall:   // FunctionCall Statement，
                IDENTIFIER
                LPAREN ValueList RPAREN 
                {
                    checkIdentifierExist($1);// 檢查IDENTIFIER是否存在於SymbolTable中
                    generator.FunctionCall($1);
                    $$ = generator.GetGlobalSymbolTable().getFunctionReturnType($1);// 回傳Function call return的type
                }
                ;
ValueList:      // 這是用來將FunctionCall中的參數存入FunctionTable中
                OrExpression
                | OrExpression COMMA ValueList
                |
                ;
ResultStatement:
                RESULT OrExpression{
                    generator.Result($2);
                }
                ;
FunctionDeclaration:// Function Declaration，包含Function和Procedure
                FUNCTION IDENTIFIER  {
                    generator.FunctionInit();
                    }
                    LPAREN ParameterList RPAREN SetType {
                        generator.FunctionCreate($2, FUNCTION, $7);
                    }
                    FunctionStatementList 
                    END IDENTIFIER          
                    {
                        checkIdentifierSame($2, $11);// 比對Function的名稱和END的名稱是否相同
                        generator.FunctionEnd();
                    }

                | PROCEDURE IDENTIFIER  {
                    generator.FunctionInit();
                    }
                    LPAREN ParameterList RPAREN {
                        generator.FunctionCreate($2, PROCEDURE, VOID);
                    }
                    FunctionStatementList 
                    END IDENTIFIER          
                    {
                        checkIdentifierSame($2, $10);// 比對Function的名稱和END的名稱是否相同
                        generator.FunctionEnd();
                    }
                |
                ;



ConstantDeclaration:// 常數宣告，宣告後會將型態存入SymbolTable中
                CONST IDENTIFIER OptionalSetType ASSIGN NUMERICALCONSTANT { 
                    generator.ConstDeclaration($2, INT, $5);
                }
                | CONST IDENTIFIER OptionalSetType ASSIGN STRINGCONSTANT { 
                    generator.ConstDeclaration($2, STRING, $5);
                }
                | CONST IDENTIFIER OptionalSetType ASSIGN TRUE { 
                    generator.ConstDeclaration($2, BOOL, "1");
                }
                | CONST IDENTIFIER OptionalSetType ASSIGN FALSE { 
                    generator.ConstDeclaration($2, BOOL, "0");
                }

                ;
VariableDeclaration:// 變數宣告，宣告後會將型態存入SymbolTable中
                VAR IDENTIFIER AssignExpression 
                {   
                    generator.VarDeclaration($2, $3);
                    generator.PutIdentifier($2);
                }
                | VAR IDENTIFIER SetType 
                {
                    generator.VarDeclaration($2, $3);
                }
                | VAR IDENTIFIER SetType AssignExpression 
                {
                    generator.VarDeclaration($2, $3);
                    generator.PutIdentifier($2);
                }
                |
                ;
GlobalVariableDeclaration:
                VAR IDENTIFIER OptionalSetType ASSIGN NUMERICALCONSTANT {
                    generator.VarDeclaration($2, INT, $5);
                } 
                | VAR IDENTIFIER SetType 
                {
                    generator.VarDeclaration($2, $3);
                }
                |
                ;
OptionalSetType: // 用來處理SetType的Optional
                SetType
                |
                ;

AssignmentStatement:// 賦值，將右邊的值賦予左邊的變數。會檢查左邊的變數是否存在於SymbolTable中，並檢查賦值的型態是否相同或是相容
                IDENTIFIER AssignExpression 
                    {
                        checkIdentifierExist($1);
                        checkSymbolIsEditable($1);
                        checkAssignAvaliable(checkIdentifierExist($1), $2);
                        generator.PutIdentifier($1);
                    }
                ;
AssignExpression:
                ASSIGN OrExpression         {
                    $$ = $2;
                    }
                ;
OrExpression:   // 這是Expression 的最高層，包含OrExpression, AndExpression, CompareExpression, Expression, Term, Factor，用來處理運算子的優先順序。每一層都會檢查型態是否相同或是相容，並回傳型態
                AndExpression               { $$ = $1;}
                | OrExpression OR AndExpression
                                            { 
                                                $$ = checkTypeOperation($1, $2, $3);
                                                generator.Operator($2);
                                            }
                ;
AndExpression:  CompareExpression           { $$ = $1;}
                | AndExpression AND CompareExpression
                                            { 
                                                $$ = checkTypeOperation($1, $2, $3);
                                                generator.Operator($2);
                                            }
                ;
CompareExpression:
                Expression                  { $$ = $1;}
                | NOT CompareExpression     { 
                    $$ = checkUnaryOperation($1, $2);
                    generator.Command("iconst_1");
                    generator.Operator($1);
                }
                | CompareExpression CompareOperater Expression
                                            { 
                                                $$ = checkTypeOperation($1, $2, $3);
                                                generator.CompareOperator($2);
                                            }
                ;
Expression:     Term                        { $$ = $1;}
                | Expression PLUS Term      { 
                    $$ = checkTypeOperation($1, $2, $3);
                    generator.Operator($2);
                }
                | Expression MINUS Term     { 
                    $$ = checkTypeOperation($1, $2, $3);
                    generator.Operator($2);
                }
Term:           Factor
                | Term TIMES Factor         { 
                    $$ = checkTypeOperation($1, $2, $3);
                    generator.Operator($2);
                }
                | Term DIV Factor           { 
                    $$ = checkTypeOperation($1, $2, $3);
                    generator.Operator($2);
                }
                | Term MOD Factor           { 
                    $$ = checkTypeOperation($1, $2, $3);
                    generator.Operator($2);
                }  
                ;
Factor:         MINUS Factor                { 
                    $$ = checkUnaryOperation($1, $2);
                    generator.Command("ineg");
                }
                | LPAREN OrExpression RPAREN    { $$ = $2;}
                | IDENTIFIER                
                { 
                    $$ = checkIdentifierExist($1);
                    generator.LoadIdentifier($1);
                }
                | NUMERICALCONSTANT         
                { 
                    $$ = checkIntOrReal($1);
                    generator.Push($1);
                }
                | STRINGCONSTANT            { 
                    $$ = STRING;
                    generator.Push($1);
                }
                | TRUE                      { 
                    $$ = BOOL;
                    generator.Command("iconst_1");
                }
                | FALSE                     { 
                    $$ = BOOL;
                    generator.Command("iconst_0");
                }
                | FunctionCall              { 
                    $$ = $1;
                }
                ;
ParameterList:  FunctionParameterSetType
                | ParameterList COMMA FunctionParameterSetType
                | 
                ;

FunctionParameterSetType:
                IDENTIFIER SetType {
                    generator.AddParameter($1, $2);
                }
                
                ;
Type:           INT {$$ = INT;}
                | REAL {$$ = REAL;}
                | BOOL {$$ = BOOL;}
                | STRING {$$ = STRING;}
                ;
SetType:        COLON Type {$$ = $2;}
                ;

Block:          _BEGIN {generator.BlockInit();}               
                    StatementList 
                    END {generator.BlockEnd();}             
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