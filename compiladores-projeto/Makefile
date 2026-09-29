SRC = src/ast.c src/tabela.c src/semantica.c
HEADERS = src/ast.h src/tabela.h src/tipos.h src/semantica.h
CFLAGS = -Wall

all: compilador

compilador: lexer/lexer.l parser/parser.y $(SRC) $(HEADERS)
	flex lexer/lexer.l
	bison -d parser/parser.y
	gcc $(CFLAGS) -o compilador lex.yy.c parser.tab.c $(SRC) -lfl

clean:
	rm -f compilador lex.yy.c parser.tab.c parser.tab.h testes/tdd/teste_tabela testes/tdd/teste_ast testes/tdd/teste_semantica testes/tdd/_ast_test_output.tmp

test: test_ast test_tabela test_semantica test_integracao

test_integracao: compilador
	./testes/rodar_testes.sh

test_tabela: testes/tdd/teste_tabela.c src/tabela.c
	gcc $(CFLAGS) -o testes/tdd/teste_tabela testes/tdd/teste_tabela.c src/tabela.c
	./testes/tdd/teste_tabela

test_ast: testes/tdd/teste_ast.c src/ast.c
	gcc $(CFLAGS) -o testes/tdd/teste_ast testes/tdd/teste_ast.c src/ast.c
	./testes/tdd/teste_ast

test_semantica: testes/tdd/teste_semantica.c src/semantica.c src/ast.c src/tabela.c
	gcc $(CFLAGS) -o testes/tdd/teste_semantica testes/tdd/teste_semantica.c src/semantica.c src/ast.c src/tabela.c
	./testes/tdd/teste_semantica
