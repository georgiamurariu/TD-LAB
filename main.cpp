#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

#define NUMBER 256

static unique_ptr<LLVMContext> TheContext;
static unique_ptr<IRBuilder<NoFolder>> Builder;
static unique_ptr<Module> TheModule;

static void InitializeModule()
{
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("MyModule", *TheContext);
    Builder = std::make_unique<IRBuilder<NoFolder>>(*TheContext);
}

//===----------------------------------------------------------------------===//
// AST nodes
//===----------------------------------------------------------------------===//
class GenericASTNode {
public:
    virtual ~GenericASTNode() = default;
    virtual void toString() = 0;
    virtual Value *codegen() = 0;
};

class NumberASTNode : public GenericASTNode {
  int Val;

public:
    NumberASTNode(int Val)
    {
        this->Val = Val;
    }

    void toString() override { printf("%d", Val); }

    // Tutorialul foloseste ConstantFP pentru double; noi folosim ConstantInt.
    Value* codegen() override {
        return ConstantInt::getSigned(Type::getInt32Ty(*TheContext), Val);
    }
};

class BinaryExprAST : public GenericASTNode {
    char Op;
    unique_ptr<GenericASTNode> LHS, RHS;

public:
    BinaryExprAST(char Op, unique_ptr<GenericASTNode> LHS, unique_ptr<GenericASTNode> RHS)
    {
        this->Op = Op;
        this->LHS = std::move(LHS);
        this->RHS = std::move(RHS);
    }

    void toString() override {
        printf("(");
        LHS->toString();
        printf(" %c ", Op);
        RHS->toString();
        printf(")");
    }

    Value* codegen() override {
        Value *L = LHS->codegen();
        Value *R = RHS->codegen();
        if (!L || !R) return nullptr;

        // Operatii pe intregi: SDiv si SRem interpreteaza numerele cu semn.
        switch (Op) {
        case '+': return Builder->CreateAdd(L, R, "addtmp");
        case '-': return Builder->CreateSub(L, R, "subtmp");
        case '*': return Builder->CreateMul(L, R, "multmp");
        case '/': return Builder->CreateSDiv(L, R, "divtmp");
        case '%': return Builder->CreateSRem(L, R, "remtmp");
        default: return nullptr;
        }
    }
};

bool CodeGenTopLevel(unique_ptr<GenericASTNode> AST_Root)
{
    // Create an anonymous function with no parameters
    vector<Type *> ArgumentsTypes(0);
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(*TheContext), ArgumentsTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    // Create a label 'entry' and set it to the current position in the builder
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    // Generate the code for the body of the function and return the result
    if (Value *RetVal = AST_Root->codegen()) {
        Builder->CreateRet(RetVal);
    } else {
        return false;
    }
    if (verifyFunction(*F, &errs())) return false;

    auto Filename = "output.ll";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC);

    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return false;
    }

    F->print(errs());
    F->print(dest);
    return true;
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//
int yylex();
int symbol;
extern int yylval;

unique_ptr<GenericASTNode> Z();
unique_ptr<GenericASTNode> E_AS(); // Addition and subtraction
unique_ptr<GenericASTNode> E_MDR(); // Multiplication, Division and Remainder
unique_ptr<GenericASTNode> T();

void next_symbol()
{
	symbol = yylex();
}

// La eroare intoarcem nullptr; apelantul propaga eroarea pana in main.
unique_ptr<GenericASTNode> error(const char *message)
{
    fprintf(stderr, "Eroare: %s\n", message);
    return nullptr;
}

// T ::= numar | '(' E_AS ')'
unique_ptr<GenericASTNode> T()
{
    if (symbol == NUMBER) {
        auto node = make_unique<NumberASTNode>(yylval);
        next_symbol();
        return node;
    }
    if (symbol == '(') {
        next_symbol(); // consumam '('
        auto node = E_AS();
        if (!node) return nullptr;
        if (symbol != ')') return error("Lipseste paranteza ')'.");
        next_symbol(); // consumam ')'
        return node;
    }
    return error("Se astepta un numar intreg sau '('.");
}

// E_MDR ::= T (('*' | '/' | '%') T)*
unique_ptr<GenericASTNode> E_MDR()
{
    auto left = T();
    if (!left) return nullptr;
    while (symbol == '*' || symbol == '/' || symbol == '%') {
        char op = static_cast<char>(symbol);
        next_symbol();
        auto right = T();
        if (!right) return nullptr;
        left = make_unique<BinaryExprAST>(op, std::move(left), std::move(right));
    }
    return left;
}

// E_AS ::= E_MDR (('+' | '-') E_MDR)*
// Apelam E_MDR mai intai: inmultirea are prioritate fata de adunare.
unique_ptr<GenericASTNode> E_AS()
{
    auto left = E_MDR();
    if (!left) return nullptr;
    while (symbol == '+' || symbol == '-') {
        char op = static_cast<char>(symbol);
        next_symbol();
        auto right = E_MDR();
        if (!right) return nullptr;
        left = make_unique<BinaryExprAST>(op, std::move(left), std::move(right));
    }
    return left;
}

// Z ::= E_AS; acceptam optional '=' inainte de Enter sau EOF.
unique_ptr<GenericASTNode> Z()
{
    auto root = E_AS();
    if (!root) return nullptr;
    if (symbol == '=') next_symbol();
    if (symbol != '\n' && symbol != 0)
        return error("Caracter neasteptat dupa expresie.");
    return root;
}

int main()
{
    // Nu lasam un output vechi sa para rezultatul unei intrari gresite.
    std::remove("output.ll");
    InitializeModule();
    printf("Introdu o expresie cu intregi (+ - * / %% si paranteze):\n");
    next_symbol();
    auto root = Z();
    if (!root) return 1;

    printf("AST: ");
    root->toString();
    printf("\n");
    if (!CodeGenTopLevel(std::move(root))) return 1;
    printf("Codul LLVM a fost salvat in output.ll.\n");
    return 0;
}
