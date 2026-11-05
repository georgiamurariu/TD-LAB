# Detectam LLVM 17 pe macOS (Homebrew) sau Linux.
LLVM_CONFIG := $(shell command -v llvm-config-17 2>/dev/null || command -v llvm-config 2>/dev/null)
ifeq ($(LLVM_CONFIG),)
LLVM_CONFIG := $(firstword $(wildcard /opt/homebrew/opt/llvm@17/bin/llvm-config /usr/local/opt/llvm@17/bin/llvm-config))
endif
LLVM_BIN := $(shell "$(LLVM_CONFIG)" --bindir 2>/dev/null)
CXX := $(LLVM_BIN)/clang++
LLVM_FLAGS = $(shell "$(LLVM_CONFIG)" --cxxflags --ldflags --system-libs --libs core)

.PHONY: all build_and_run run clean
all: main
build_and_run: main

main: main.cpp lexer.cpp
	@test -n "$(LLVM_CONFIG)" || (echo "LLVM lipseste. Instaleaza LLVM 17 sau seteaza LLVM_CONFIG=/cale/llvm-config"; exit 1)
	"$(CXX)" -g -O0 main.cpp lexer.cpp $(LLVM_FLAGS) -std=c++17 -o main

lexer.cpp: lexer.l
	flex -o lexer.cpp lexer.l

# Ruleaza IR-ul numai daca parsarea si generarea au reusit.
run: main
	@./main && { "$(LLVM_BIN)/lli" output.ll; result=$$?; echo "Result is: $$result"; }

clean:
	rm -f lexer.cpp main output.ll
