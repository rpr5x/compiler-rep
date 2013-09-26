
all: turtle eins.ps

eins.ps: eins.tlt turtle
	./turtle < eins.tlt > eins.ps

lex.yy.o:  lex.yy.c turtle-rpr5x.tab.h symtab.h
	gcc -c lex.yy.c

lex.yy.c: turtle_rpr5x.l
	flex turtle_rpr5x.l
	
turtle-rpr5x.tab.h: turtle-rpr5x.y
	bison -d turtle-rpr5x.y
	
turtle-rpr5x.tab.c: turtle-rpr5x.y
	bison -d turtle-rpr5x.y
	
turtle-rpr5x.tab.o: turtle-rpr5x.tab.c symtab.h
	gcc -c turtle-rpr5x.tab.c

turtle: turtle-rpr5x.tab.o lex.yy.o symtab.c
	gcc lex.yy.o turtle-rpr5x.tab.o symtab.c -lfl -o turtle

clean:
	rm -f turtle lex.* turtle-rpr5x.tab.* *.o

