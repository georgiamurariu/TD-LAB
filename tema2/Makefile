run:
	flex --header-file=lexer.h -o lexer.c eval_bnf_todo.l
	gcc -o prog main_bnf_todo.c lexer.c
	./prog

clean:
	rm -f lexer.c lexer.h prog
