

eins.out: eins.tlt scanner 
	./scanner < test.tlt > test.out

lex.yy.o:  lex.yy.c token.h symtab.h
	gcc -c lex.yy.c

lex.yy.c: turtle_rpr5x.l
	flex turtle_rpr5x.l
	
scanner:  lex.yy.o symtab.c
	gcc lex.yy.o symtab.c -lfl -o scanner 

clean:
	rm -f scanner lex.* 

