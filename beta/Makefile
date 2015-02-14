compile: lex-it yacc-it
	gcc lex.yy.c y.tab.c -o output -lfl

yacc-it:
	bison -dy yacc.y

lex-it:
	flex lex.l

clean:
	rm -rf lex.yy.c lex output y.tab.c y.tab.h
