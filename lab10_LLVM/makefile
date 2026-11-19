LLVM_CONFIG ?= $(shell command -v llvm-config-17 || command -v llvm-config || echo /opt/homebrew/opt/llvm@17/bin/llvm-config)
CXX := clang++
LLVM_FLAGS = $(shell $(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs core)
LLI = $(shell $(LLVM_CONFIG) --bindir)/lli
EXPR ?= if (1) { 2+3 } else { 4*5 }
export EXPR

.DEFAULT_GOAL := run
.PHONY: build_and_run run clean
build_and_run: main

run: main
	@ir=$$(printf '%s\n' "$$EXPR" | ./main 2>&1) || { printf '%s\n' "$$ir" >&2; exit 1; }
	@test -x "$(LLI)" || { printf 'Nu găsesc lli: %s\n' "$(LLI)" >&2; exit 1; }
	@"$(LLI)" output.ll; printf 'Rezultat: %s\n' "$$?"

main: main.cpp lexer.cpp tokens.h
	$(CXX) -g -O3 main.cpp lexer.cpp $(LLVM_FLAGS) -fexceptions -o $@

lexer.cpp: lexer.l
	lex -o $@ $<

clean:
	rm -f lexer.cpp main output.ll
