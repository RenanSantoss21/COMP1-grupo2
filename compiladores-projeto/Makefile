SRC = src/ast.c src/tabela.c src/semantica.c src/codegen_intermediario.c
HEADERS = src/ast.h src/tabela.h src/tipos.h src/semantica.h src/codegen_intermediario.h
CFLAGS = -Wall
ifdef CI
CFLAGS += -Werror
endif

all: compilador

compilador: lexer/lexer.l parser/parser.y $(SRC) $(HEADERS)
	flex lexer/lexer.l
	bison -d parser/parser.y
	gcc $(CFLAGS) -o compilador lex.yy.c parser.tab.c $(SRC) -lfl

clean:
	rm -f compilador lex.yy.c parser.tab.c parser.tab.h ast.dot ast.png testes/tdd/teste_tabela testes/tdd/teste_ast testes/tdd/teste_semantica testes/tdd/_ast_test_output.tmp testes/tdd/_ast_test_dot.tmp testes/tdd/teste_tac

test: test_ast test_tabela test_semantica test_integracao test_tac

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
test_tac: testes/tdd/teste_tac.c src/codegen_intermediario.c
	gcc $(CFLAGS) -o testes/tdd/teste_tac testes/tdd/teste_tac.c src/codegen_intermediario.c
	./testes/tdd/teste_tac
