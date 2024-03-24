#ifndef COMPILER_SEMANTIC_HPP
#define COMPILER_SEMANTIC_HPP

#include <parser.hpp>



/* $begin Visitors */

class SymbolTableCreationVisitor : public ASTNodeVisitor {
public:
    int tempVarCounter = 0; // temp var name for ops is t0, t1, t2, ...

    std::string getTempVarName() {
        return "t" + std::to_string(tempVarCounter++);
    }

    void visit(ProgNode& node) override {
        node.symbolTable = new SymbolTable("global", nullptr, 0);
        // propagate accepting the same visitor to all children
        // this is a depth-first traversal
        for (auto child : node.children) {
            child->parent = &node;
            // set the symbol table of the child to the symbol table of the parent
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(StructDeclNode& node) override {
        /* StructDeclNode
         * - IdNode
         * - InheritListNode
         * - MemberListNode
         */
        std::string structName = node.children[0]->value;
        auto *structTable = new SymbolTable(structName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new StructEntry(structName, structName, structTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = structTable;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    // does nothing
    void visit(IdNode& node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(InheritListNode &node) override {
        /* IneritListNode
         * - vector<IdNode>
         * */
        for (auto child : node.children) {
            auto *inheritEntry = new SymbolTableEntry(child->value, "inherit", child->value, nullptr);
            node.symbolTable->insert(inheritEntry);

            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(MemberListNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(MemberNode &node) override {
        /* MemberNode
         * - VisibilityNode
         * - funcDecl or varDecl
         * */

        auto visibility = node.children[0]->value;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

        // set the visibility of the symbol table entry
        node.children[1]->symbolTableEntry->visibility = visibility;
    }

    // does nothing
    void visit(VisibilityNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FuncDeclNode &node) override {
        /* FuncDeclNode
         * - IdNode
         * - FParamListNode
         * - TypeNode
         * */

        std::string funcName = node.children[0]->value;
        std::string funcType = node.children[2]->value;
        auto *funcTable = new SymbolTable(funcName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new FuncEntry(funcName, funcType, funcTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = funcTable;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(FParamListNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(FParamNode &node) override {
        /* FParamNode
         * - IdNode
         * - TypeNode
         * - ArraySizeListNode
         * */

        std::string paramName = node.children[0]->value;
        std::string paramType = node.children[1]->value;
        std::string dims = "";
        for (auto intlit : node.children[2]->children) {
            dims += "[" + intlit->value + "]";
        }

        auto *paramEntry = new SymbolTableEntry(paramName, "param", paramType + dims, nullptr);
        node.symbolTable->insert(paramEntry);

        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    // does nothing
    void visit(TypeNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    // does nothing
    void visit(ArraySizeListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    // does nothing
    void visit(IntlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VarDeclNode &node) override {
        /* VarDeclNOde
         * - IdNode
         * - TypeNode
         * - ArraySizeListNode
         * */

        std::string varName = node.children[0]->value;
        std::string varType = node.children[1]->value;
        std::string dims = "";
        for (auto intlit : node.children[2]->children) {
            dims += "[" + intlit->value + "]";
        }

        node.symbolTableEntry = new VarEntry(varName, varType + dims);
        node.symbolTable->insert(node.symbolTableEntry);

        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FuncDefNode &node) override {
        /* FuncDefNode
         * - IdNode
         * - FParamListNode
         * - TypeNode
         * - VarDeclOrStatBlockNode
         * */

        std::string funcName = node.children[0]->value;
        std::string funcType = node.children[2]->value;
        auto *funcTable = new SymbolTable(funcName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new FuncEntry(funcName, funcType, funcTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = funcTable;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

    }

    void visit(VarDeclOrStatBlockNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }


    void visit(IfStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(RelExprNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(RelOpNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(AddOpNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(MultOpNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(DotNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(IndiceListNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(AParamsListNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(FloatlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(NotNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(SignNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

    }

    void visit(StatBlockNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(WhileStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(ReadStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(WriteStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(ReturnStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(AssignStatNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(AssignOpNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(FunctionCallNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(VariableNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

    }

    void visit(EpsilonNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ImplDefNode &node) override {
        /* ImplDefNode
         * - IdNode
         * - ImplFuncListNode
         * */

        std::string implName = node.children[0]->value;
        auto *implTable = new SymbolTable(implName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new ImplEntry(implName, implName, implTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = implTable;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }

    void visit(ImplFuncListNode &node) override {
        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }
    }
};

class ImplToStructAddingVisitor : public ASTNodeVisitor {
public:
    void visit(ImplDefNode &node) override {
        std::string implName = node.children[0]->value;
        auto foo = node.symbolTable;
        auto *implEntry = node.symbolTable->upperScope->lookup(implName, "impl");
        auto *structEntry = node.symbolTable->upperScope->lookup(implName, "struct");

        if (implEntry == nullptr || structEntry == nullptr) {
            return;
        }

        node.symbolTable->upperScope->remove(implEntry);
        structEntry->link->symList.emplace_back(implEntry);
        node.symbolTable = structEntry->link;

        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    // now for all the other nodes, we just propagate
    void visit(ProgNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(StructDeclNode &node) override {
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

    void visit(MemberListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(MemberNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VisibilityNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FuncDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FParamListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FParamNode &node) override {
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

    void visit(IntlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VarDeclNode &node) override {
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

    void visit(RelExprNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(RelOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(AddOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(MultOpNode &node) override {
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

    void visit(FloatlitNode &node) override {
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

    void visit(FunctionCallNode &node) override {
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

    void visit(ImplFuncListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

};


/* $end Visitors */


#endif //COMPILER_SEMANTIC_HPP
