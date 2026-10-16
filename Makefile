run:
	flex --header-file=lexer.h -o lexer.c eval_todo.l
	gcc -o prog main_todo.c lexer.c
	./prog

clean:
	rm -f lexer.c lexer.h prog
