all: compilador

compilador: lexer/lexer.l parser/parser.y
	flex lexer/lexer.l
	bison -d parser/parser.y
	gcc -o compilador lex.yy.c parser.tab.c -lfl

clean:
	rm -f compilador lex.yy.c parser.tab.c parser.tab.h
