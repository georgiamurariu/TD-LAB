LLVM_CONFIG ?= /opt/homebrew/opt/llvm@17/bin/llvm-config
CXX = $(shell $(LLVM_CONFIG) --bindir)/clang++

.PHONY: build_and_run clean check-llvm

build_and_run: check-llvm
	@lex -o lexer.cpp lexer.l
	@$(CXX) -g -O3 main.cpp lexer.cpp `$(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs core` -o main -ll
	@./main

check-llvm:
	@test -x "$(LLVM_CONFIG)" || { echo "Lipseste LLVM 17. Pe macOS: brew install llvm@17"; exit 1; }

clean:
	rm -f lexer.cpp main
