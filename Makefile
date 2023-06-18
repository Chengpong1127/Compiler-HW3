##################################################################
#
#	Makefile -- P Parser
#
##################################################################

CC=g++
CFLAGS=-Wall

myparser.out: lex.yy.o y.tab.o symbol.o
	$(CC) $(CFLAGS) -o myparser.out lex.yy.o y.tab.o symbol.o -ll

lex.yy.o: lex.yy.c y.tab.h util.cpp
	$(CC) $(CFLAGS) -c lex.yy.c 

y.tab.o: y.tab.c symbol.o util.cpp
	$(CC) $(CFLAGS) -c y.tab.c 

lex.yy.c: mylex.l
	flex mylex.l

y.tab.c: myyacc.y symbol.cpp JavaGenerator.cpp
	bison -d myyacc.y

y.tab.h: myyacc.y symbol.cpp JavaGenerator.cpp
	yacc -d myyacc.y 
symbol.o: symbol.cpp
	$(CC) -c -g symbol.cpp

clean:
	rm -f myparser.out lex.yy.c *.tab.c *.tab.h *.o

test: test/test1.st
	./myparser.out test/test1.st && ./javaa/javaa output.jasm && java output