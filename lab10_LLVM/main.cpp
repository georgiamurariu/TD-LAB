#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include "tokens.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

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
class GenericASTNode
{
public:
    virtual ~GenericASTNode() = default;
    virtual void toString(){};
    virtual Value *codegen() = 0;
};

class NumberASTNode : public GenericASTNode
{
    int Val;

public:
    NumberASTNode(int Val)
    {
        this->Val = Val;
    }

    void toString() override { return; }
    Value *codegen() override
    {
        return Builder->getInt32(Val);
    }
};

class BinaryExprAST : public GenericASTNode
{
    char Op;
    unique_ptr<GenericASTNode> LHS, RHS;

public:
    BinaryExprAST(char Op, unique_ptr<GenericASTNode> LHS, unique_ptr<GenericASTNode> RHS)
    {
        this->Op = Op;
        this->LHS = std::move(LHS);
        this->RHS = std::move(RHS);
    }

    void toString() override { return; }

    Value *codegen() override
    {
        Value *left = LHS->codegen();
        Value *right = RHS->codegen();
        switch (Op)
        {
        case '+': return Builder->CreateAdd(left, right);
        case '-': return Builder->CreateSub(left, right);
        case '*': return Builder->CreateMul(left, right);
        case '/': return Builder->CreateSDiv(left, right);
        case '%': return Builder->CreateSRem(left, right);
        }
        throw runtime_error("Unknown arithmetic operator");
    }
};

class IfStatementAST : public GenericASTNode
{
    unique_ptr<GenericASTNode> Cond, TrueExpr, FalseExpr;

public:
    IfStatementAST(unique_ptr<GenericASTNode> Cond, unique_ptr<GenericASTNode> TrueExpr, unique_ptr<GenericASTNode> FalseExpr)
    {
        this->Cond = std::move(Cond);
        this->TrueExpr = std::move(TrueExpr);
        this->FalseExpr = std::move(FalseExpr);
    }

    void toString() override { return; }
    Value *codegen() override
    {
        Value *condition = Builder->CreateICmpNE(Cond->codegen(), Builder->getInt32(0), "cond");
        Function *function = Builder->GetInsertBlock()->getParent();
        BasicBlock *thenBlock = BasicBlock::Create(*TheContext, "then", function);
        BasicBlock *elseBlock = BasicBlock::Create(*TheContext, "else", function);
        BasicBlock *mergeBlock = BasicBlock::Create(*TheContext, "merge", function);
        Builder->CreateCondBr(condition, thenBlock, elseBlock);

        Builder->SetInsertPoint(thenBlock);
        Value *thenValue = TrueExpr->codegen();
        Builder->CreateBr(mergeBlock);

        Builder->SetInsertPoint(elseBlock);
        Value *elseValue = FalseExpr->codegen();
        Builder->CreateBr(mergeBlock);

        Builder->SetInsertPoint(mergeBlock);
        PHINode *result = Builder->CreatePHI(Builder->getInt32Ty(), 2, "if.result");
        result->addIncoming(thenValue, thenBlock);
        result->addIncoming(elseValue, elseBlock);
        return result;
    }
};

void CodeGenTopLevel(unique_ptr<GenericASTNode> AST_Root)
{
    // Create an anonymous function with no parameters
    vector<Type *> ArgumentsTypes(0);
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(*TheContext), ArgumentsTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, "main", TheModule.get());

    // Create a label 'entry' and set it to the current position in the builder
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);

    // Generate the code for the body of the function and return the result
    Builder->CreateRet(AST_Root->codegen());
    if (verifyFunction(*F, &errs()))
        throw runtime_error("Invalid LLVM IR");

    std::error_code EC;
    raw_fd_ostream dest("output.ll", EC);
    if (EC)
        throw runtime_error("Could not open output.ll: " + EC.message());

    F->print(errs());
    F->print(dest);
    F->eraseFromParent();
}

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//
int yylex();
int symbol;
extern int yylval;

unique_ptr<GenericASTNode> Z();
unique_ptr<GenericASTNode> E_AS();  // Addition and Subtraction
unique_ptr<GenericASTNode> E_MDR(); // Multiplication, Division and Remainders
unique_ptr<GenericASTNode> E_IF();
unique_ptr<GenericASTNode> T();

void next_symbol()
{
    symbol = yylex();
}

void expect(int token)
{
    if (symbol != token)
        throw runtime_error(string("Expected '") + static_cast<char>(token) + "'");
    next_symbol();
}

unique_ptr<GenericASTNode> T()
{
    if (symbol == NUMBER)
    {
        auto number = make_unique<NumberASTNode>(yylval);
        next_symbol();
        return number;
    }
    if (symbol == '(')
    {
        next_symbol();
        auto expression = E_AS();
        expect(')');
        return expression;
    }
    throw runtime_error("Expected an integer or '('");
}

unique_ptr<GenericASTNode> E_MDR()
{
    auto expression = T();
    while (symbol == '*' || symbol == '/' || symbol == '%')
    {
        char op = symbol;
        next_symbol();
        expression = make_unique<BinaryExprAST>(op, std::move(expression), T());
    }
    return expression;
}

unique_ptr<GenericASTNode> E_AS()
{
    auto expression = E_MDR();
    while (symbol == '+' || symbol == '-')
    {
        char op = symbol;
        next_symbol();
        expression = make_unique<BinaryExprAST>(op, std::move(expression), E_MDR());
    }
    return expression;
}

unique_ptr<GenericASTNode> E_IF()
{
    next_symbol();
    expect('(');
    auto condition = E_AS();
    expect(')');
    expect('{');
    auto thenExpression = E_AS();
    expect('}');
    if (symbol != ELSE)
        throw runtime_error("Expected 'else'");
    next_symbol();
    expect('{');
    auto elseExpression = E_AS();
    expect('}');
    return make_unique<IfStatementAST>(std::move(condition), std::move(thenExpression), std::move(elseExpression));
}

unique_ptr<GenericASTNode> Z()
{
    auto expression = symbol == IF ? E_IF() : E_AS();
    if (symbol != 0)
        throw runtime_error("Unexpected input after expression");
    return expression;
}

int main()
{
    try
    {
        InitializeModule();
        next_symbol();
        CodeGenTopLevel(Z());
        return 0;
    }
    catch (const exception &error)
    {
        fprintf(stderr, "Error: %s\n", error.what());
        return 1;
    }
}
