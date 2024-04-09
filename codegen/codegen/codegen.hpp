#ifndef COMPILER_CODEGEN_HPP
#define COMPILER_CODEGEN_HPP

#include <ast.hpp>
#include <stack>
#include <semantic.hpp>

const int INT_SIZE = 4;
const int FLOAT_SIZE = 8;

void computeSizes(ASTNode &root);

int sizeofTable(SymbolTable *table, bool isStruct);
int sizeofEntry(SymbolTableEntry *entry, SymbolTable *currentScope);
int sizeofType(std::string &type, SymbolTable *currentScope);

inline int getDimsSize(std::string &type) {
    int size = 1;
    for (char c : type) {
        if (c == '[') {
            size *= std::stoi(type.substr(type.find('[') + 1, type.find(']') - type.find('[') - 1));
        }
    }
    return size;
}

/*
 * Visitor to compute memory size of AST nodes, and generate temp vars
 * */
class ComputeMemSizeVisitor : public ASTNodeVisitor {
public:
    int tempVarCounter = 0; // temp var name for ops is t0, t1, t2, ...

    explicit ComputeMemSizeVisitor() = default;

    std::string getTempVarName() {
        return "t" + std::to_string(tempVarCounter++);
    }

    void visit(FuncDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        if (node.symbolTableEntry->name == "main") {
            for (auto entry : node.symbolTable->symList) {
                entry->offset = node.symbolTable->size - entry->size;
                node.symbolTable->size -= entry->size;
            }
        } else {
            // stack frame contains the return value at the bottom of the stack
            node.symbolTable->size = - sizeofType(node.symbolTableEntry->type, node.symbolTable);
            // return address
            node.symbolTable->size -= 4;

            for (auto entry : node.symbolTable->symList) {
                entry->offset = node.symbolTable->size - entry->size;
                node.symbolTable->size -= entry->size;
            }
        }
    }

    void visit(StructDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        for (auto entry : node.symbolTable->symList) {
            entry->offset = node.symbolTable->size - entry->size;
            node.symbolTable->size -= entry->size;
        }
    }

    void visit(FParamNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        node.symbolTableEntry->size = sizeofEntry(node.symbolTableEntry, node.symbolTable);
    }

    void visit(VarDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        node.symbolTableEntry->size = sizeofEntry(node.symbolTableEntry, node.symbolTable);
    }



    void visit(IntlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", "integer", nullptr);
        tempVarEntry->size = INT_SIZE;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(FloatlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", "float", nullptr);
        tempVarEntry->size = FLOAT_SIZE;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(AddOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(MultOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(RelExprNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(FunctionCallNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        // temp var for function call
        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTable->insert(tempVarEntry);

    }



    void visit(ImplDefNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ImplFuncListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(MemberNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(MemberListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ProgNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(IdNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(InheritListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VisibilityNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FParamListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(TypeNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ArraySizeListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FuncDefNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VarDeclOrStatBlockNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(IfStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(RelOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(DotNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(IndiceListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(AParamsListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(NotNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(SignNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(StatBlockNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(WhileStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ReadStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(WriteStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ReturnStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(AssignStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(AssignOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VariableNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(EpsilonNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }
};


/*
 * Stack-based code generation visitor
 * */
class CodeGenerationVisitor : public ASTNodeVisitor {
public:
    std::stack<std::string> registerPool;
    std::string moonExecCode;
    std::string moonDataCode;

};

#endif //COMPILER_CODEGEN_HPP
