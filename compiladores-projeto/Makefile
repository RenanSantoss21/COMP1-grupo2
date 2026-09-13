all: compilador

compilador: lexer/lexer.l parser/parser.y
	flex lexer/lexer.l
	bison -d parser/parser.y
	gcc -o compilador lex.yy.c parser.tab.c -lfl

clean:
	rm -f compilador lex.yy.c parser.tab.c parser.tab.h testes/tdd/teste_tabela testes/tdd/teste_ast

test_tabela: testes/tdd/teste_tabela.c src/tabela.c
	gcc -o testes/tdd/teste_tabela testes/tdd/teste_tabela.c src/tabela.c
	./testes/tdd/teste_tabela

test_ast: testes/tdd/teste_ast.c src/ast.c
	gcc -o testes/tdd/teste_ast testes/tdd/teste_ast.c src/ast.c
	./testes/tdd/teste_ast

