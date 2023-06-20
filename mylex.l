%{
#define DEBUG 0
#include "y.tab.h"
#include <stdio.h>
#define LIST     strcat(buf,yytext)
#define token(s, t) {LIST;if(DEBUG) printf("<%s>\n",s);return t;}
#define tokenNumericalConstants(t, number, token) {LIST;if(DEBUG) printf("<%s:%s>\n",t,number);return token;}
#define tokenString(t, s, token) {LIST;if(DEBUG) printf("<%s:%s>\n",t,s); return token;}
#include "JavaGenerator.cpp"
#include <string>

#define MAX_LINE_LENG 256
extern "C"
{
    int yywrap(void);
    int yylex(void);
}
extern int linenum;
extern JavaGenerater generator;
char buf[MAX_LINE_LENG];
void newLine() {
    LIST;
    string line = to_string(linenum) + ": " + buf;
    generator.WriteCode(line);
    linenum++;
    buf[0] = '\0';
}

char* removeQuote(char* str) {
    if (strlen(str) <= 2) {
        return str;
    }
    char* result = str + 1;
    str[strlen(str) - 1] = '\0';
    return result;
}
%}
digit [0-9]+
letter [a-zA-Z]
NumericalConstants [0-9]+(\.[0-9]+)?
identifier {letter}({letter}|{digit}|_)*
%x COMMENT

%%

"{%"          { LIST; BEGIN(COMMENT); }
<COMMENT>"%}" { LIST; BEGIN(INITIAL); }
<COMMENT>\n   { newLine();}
<COMMENT>.    { LIST;}



%[^\n]* {LIST;}
\"([^\"\n])*\" {yylval.sval = yytext; tokenString("StringConstants", removeQuote(yytext), STRINGCONSTANT);} 

"."     {token(".", DOT);}
","     {token(",", COMMA);}
":"     {token(":", COLON);}
";"     {token(";", SEMICOLON);}
"("     {token("(", LPAREN);}
")"     {token(")", RPAREN);}
"["     {token("[", LBRACKET);}
"]"     {token("]", RBRACKET);}
"{"     {token("{", LBRACE);}
"}"     {token("}", RBRACE);}
"+"     {yylval.type=PLUS; token("+", PLUS);}
"-"     {yylval.type=MINUS; token("-", MINUS);}
"*"     {yylval.type=TIMES; token("*", TIMES);}
"/"     {yylval.type=DIV; token("/", DIV);}
"mod"   {yylval.type=MOD; token("mod", MOD);}
":="    {token(":=", ASSIGN);}
"<"     {yylval.type=LT; token("<", LT);}
"<="    {yylval.type=LE; token("<=", LE);}
">="    {yylval.type=GE; token(">=", GE);}
">"     {yylval.type=GT; token(">", GT);}
"="     {yylval.type=EQ; token("=", EQ);}
"not="  {yylval.type=NE; token("not=", NE);}
"and"   {yylval.type=AND; token("and", AND);}
"or"    {yylval.type=OR; token("or", OR);}
"not"   {yylval.type=NOT; token("not", NOT);}
{NumericalConstants} {yylval.sval=strdup(yytext); tokenNumericalConstants("NumericalConstants", yytext, NUMERICALCONSTANT); }



"begin" {token("BEGIN", _BEGIN);}
"char"  {token("CHAR", CHAR);}
"const" {token("CONST", CONST);}
"decreasing" {token("DECREASING", DECREASING);}
"default" {token("DEFAULT", DEFAULT);}
"do"    {token("DO", DO);}
"else"  {token("ELSE", ELSE);}
"end"   {token("END", END);}
"exit"  {token("EXIT", EXIT);}
"false" {token("FALSE", FALSE);}
"for"   {token("FOR", FOR);}
"function" {token("FUNCTION", FUNCTION);}
"get"   {token("GET", GET);}
"if"    {token("IF", IF);}
"loop"  {token("LOOP", LOOP);}
"of"    {token("OF", OF);}
"put"   {token("PUT", PUT);}
"procedure" {token("PROCEDURE", PROCEDURE);}
"result" {token("RESULT", RESULT);}
"return" {token("RETURN", RETURN);}
"skip"  {token("SKIP", SKIP);}
"then"  {token("THEN", THEN);}
"true"  {token("TRUE", TRUE);}
"var"   {token("VAR", VAR);}
"when"  {token("WHEN", WHEN);}

"bool"  {token("BOOL", BOOL);}
"int"   {token("INT", INT);}
"real"  {token("REAL", REAL);}
"string" {token("STRING", STRING);}
"array" {token("ARRAY", ARRAY);}

{identifier} {yylval.sval=strdup(yytext); tokenString("IDENTIFIER", yytext, IDENTIFIER);}
\n      {newLine();}

[ \t]*  {LIST;}

.       {
        LIST;
        printf("%d:%s\n", linenum+1, buf);
        printf("bad character:'%s'\n",yytext);
        exit(-1);
        }
%%
int yywrap(void)
{
    return 1;
}
