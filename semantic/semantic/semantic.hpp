#ifndef COMPILER_SEMANTIC_HPP
#define COMPILER_SEMANTIC_HPP

#include <parser.hpp>
#include <iostream>

/*
 * Removes the array size from a type or semantic type
 */
inline std::string trimVariableType(const std::string& type) {
    std::string trimmedType;
    for (char c : type) {
        if (c == '[') break;
        trimmedType += c;
    }
    return trimmedType;
}

/*
 * Checks if two variable types are equal in type and dimension size
 * Input: two variable types or semantic types
 */
inline bool areTwoVarsTypesEqual(std::string &a, std::string &b) {
    if (trimVariableType(a) != trimVariableType(b)) {
        return false;
    }

    // check if both have the same number of dimensions
    int numDimsA = 0;
    for (char c : a) {
        if (c == '[') {
            numDimsA++;
        }
    }

    int numDimsB = 0;
    for (char c : b) {
        if (c == '[') {
            numDimsB++;
        }
    }

    return numDimsA == numDimsB;
}

/*
 * Semantic checking and type propagation for a variable node (or id node used as a variable)
 * */
inline void variableCheck(ASTNode &node, std::ostream &symerrors) {
    auto *functionScope = node.symbolTable;

    std::string id;
    if (node.type == 22) {
        id = node.value;
    } else {
        id = node.children[0]->value;
    }

    // lookup the variable in the symbol table
    auto *varEntry = functionScope->lookupVarEntryFromFunctionScope(id, symerrors);
    if (varEntry == nullptr) {
        node.semanticType = "errortype";
        return;
    }

    node.semanticType = varEntry->type;

    if (node.semanticType == "errortype") {
        return; // this return can be issue, might remove
    }

    if (node.type == 22) return;

    std::string indiceList;
    for (auto indice : node.children[1]->children) {
        indiceList += "[" + indice->value + "]";
    }

    std::string scope;
    if (functionScope->level == 1) {
        scope = functionScope->name;
    } else {
        scope = functionScope->upperScope->name + "::" + functionScope->name;
    }


    if (node.children[1]->children.empty()) { // parent shouldn't be an aparamslist so this isn't a functioncall
        if (node.semanticType.find('[') != std::string::npos && node.parent->type != 7) {
            if (varEntry->kind == "param") {
                symerrors << "13.3 [error] array access " << id << indiceList << " on non-array parameter " << id << " with wrong number of dimensions, in " << scope << std::endl;
            } else {
                symerrors << "13.1 [error] array access " << id << indiceList << " on non-array variable " << id << " with wrong number of dimensions, in " << scope << std::endl;
            }

            node.semanticType = "errortype";
        }
    } else {
        int numDims = node.children[1]->children.size();
        int numDimsInType = 0;
        for (char c : node.semanticType) {
            if (c == '[') {
                numDimsInType++;
            }
        }
        if (numDims != numDimsInType) {
            if (varEntry->kind == "param") {
                symerrors << "13.3 [error] use of array parameter with definition " << node.semanticType << " with wrong number of dimensions " << id << indiceList << " in " << scope << std::endl;
            } else {
                symerrors << "13.1 [error] use of array variable with definition " << node.semanticType << " with wrong number of dimensions " << id << indiceList << " in " << scope << std::endl;
            }

            node.semanticType = "errortype";
        }
    }

}

/*
 * Semantic checking of a member function declaration or a free function definition
 * */
inline void functionCheck(ASTNode &node, SymbolTableEntry *existingFuncEntry, std::string &funcType, std::string &funcName, std::ostream &symerrors) {
    // multiply declared
    if (existingFuncEntry != nullptr) {
        if (existingFuncEntry->type == funcType) {
            bool sameFParams = true;
            for (int i = 0; i < node.symbolTable->symList.size(); i++) {
                if (node.symbolTable->symList[i]->kind != "param") continue;
                if (node.symbolTable->symList[i]->type != existingFuncEntry->link->symList[i]->type) {
                    sameFParams = false;
                    break;
                }
            }

            if (sameFParams) {
                if (node.symbolTable->upperScope->level == 0) {
                    symerrors << "8.2 [error] multiply declared free function " << funcName << std::endl;
                } else {
                    symerrors << "8.3 [error] multiply declared member function " << funcName << std::endl;
                }
                return;
            }
        }

        if (node.symbolTable->upperScope->level == 0) {
            symerrors << "9.1 [warning] overloaded free function " << funcName << std::endl;
        } else {
            symerrors << "9.2 [warning] overloaded member function " << funcName << std::endl;
        }
    }
}

/*
 * Semantic checking of a variable declaration in a struct member list or member function implementation
 * for shadowing of inherited variables
 * */
inline void inheritanceVariableDeclCheck(ASTNode &node, SymbolTable *structTable, SymbolTable *globalTable, std::ostream &symerrors, bool isLocal, std::string &currentScopeName) {
    // check for shadowing of inherited variables
    std::vector<std::string> inheritNames = structTable->lookupAllNamesOfKind("inherit");
    if (!inheritNames.empty()) {
        for (const auto &inheritName : inheritNames) {
            // look for the struct in the global table
            auto *inheritedStructEntry = globalTable->lookup(inheritName, "struct");
            if (inheritedStructEntry == nullptr) return;
            auto *inheritedStructTable = inheritedStructEntry->link;
            auto *matchingInheritedVarEntry = inheritedStructTable->lookup(node.children[0]->value, "var");
            if (matchingInheritedVarEntry != nullptr) {
                if (isLocal) {
                    symerrors << "8.6 [warning] local variable " << structTable->name << "::" << currentScopeName << "::" << node.children[0]->value
                              << " shadows inherited variable " << inheritedStructTable->name << "::"
                              << node.children[0]->value << std::endl;
                } else {
                    symerrors << "8.5 [warning] member variable " << structTable->name << "::" << node.children[0]->value
                              << " shadows inherited variable " << inheritedStructTable->name << "::"
                              << node.children[0]->value << std::endl;
                }

            }
        }
    }

}

/* $begin symbol table creation visitors */
/*
 * Visitor to generate symbol tables for the AST intermediate representation.
 * Also checks for semantic errors such as multiply defined variables, functions, etc.
 * */
class SymbolTableCreationVisitor : public ASTNodeVisitor {
public:
    int tempVarCounter = 0; // temp var name for ops is t0, t1, t2, ...
    std::ostream &symerrors;

    explicit SymbolTableCreationVisitor(std::ostream &symerrors) : symerrors(symerrors) {}

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

        auto *existingEntry = node.symbolTable->lookup(structName, "struct");
        if (existingEntry != nullptr) {
            symerrors << "8.1 [error] multiply defined struct " << structName << std::endl;
        }

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

        node.children[1]->symbolTableEntry->visibility = visibility;
    }

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

        auto *existingFuncEntry = node.symbolTable->lookup(funcName, "func");

        std::string funcType = node.children[2]->value;
        auto *funcTable = new SymbolTable(funcName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new FuncEntry(funcName, funcType, funcTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = funcTable;

        for (auto child: node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

        functionCheck(node, existingFuncEntry, funcType, funcName, symerrors);
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
        std::string dims;
        for (auto intlit : node.children[2]->children) {
            dims += "[" + intlit->value + "]";
        }

        auto *existingParamEntry = node.symbolTable->lookup(paramName, "param");
        if (existingParamEntry != nullptr) {
            if (node.symbolTable->level == 1) {
                symerrors << "8.4 [error] multiply defined parameter in a free function " << node.symbolTable->name << "::" << paramName << std::endl;
            } else {
                symerrors << "8.4 [error] multiply defined parameter in a member function " << node.parent->symbolTable->name << "::" << node.symbolTable->name << "::" << paramName << std::endl;
            }
        }

        auto *paramEntry = new SymbolTableEntry(paramName, "param", paramType + dims, nullptr);
        node.symbolTable->insert(paramEntry);

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
        /* VarDeclNOde
         * - IdNode
         * - TypeNode
         * - ArraySizeListNode
         * */

        std::string varName = node.children[0]->value;
        std::string varType = node.children[1]->value;
        std::string dims;
        for (auto intlit : node.children[2]->children) {
            dims += "[" + intlit->value + "]";
        }

        auto *existingVarEntry = node.symbolTable->lookup(varName, "var");
        if (existingVarEntry != nullptr) {
            if (node.parent->type == 10) { // VarDeclOrStatBlock
                if (node.symbolTable->level == 1) {
                    symerrors << "8.4 [error] multiply defined local variable in a free function " << node.symbolTable->name << "::" << varName << std::endl;
                } else {
                    symerrors << "8.4 [error] multiply defined local variable in a member function " << node.parent->symbolTable->name << "::" << node.symbolTable->name << "::" << varName << std::endl;
                }

            } else {
                symerrors << "8.3 [error] multiply defined member variable " << node.symbolTable->name << "::" << varName << std::endl;
            }
        }

        auto *existingParamEntry = node.symbolTable->lookup(varName, "param");
        if (existingParamEntry != nullptr) {
            if (node.symbolTable->level == 1) {
                symerrors << "8.4 [error] multiply defined identifier in a free function: "
                          << node.symbolTable->name << "::" << varName << " is a param and a variable" << std::endl;
            } else {
                symerrors << "8.4 [error] multiply defined local variable in a member function "
                          << node.parent->symbolTable->name << "::" << node.symbolTable->name << "::" << varName << " is a param and a variable " << std::endl;
            }
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

        auto *existingFuncEntry = node.symbolTable->lookup(funcName, "func");

        auto *funcTable = new SymbolTable(funcName, node.symbolTable, node.symbolTable->level + 1);
        node.symbolTableEntry = new FuncEntry(funcName, funcType, funcTable);
        node.symbolTable->insert(node.symbolTableEntry);
        node.symbolTable = funcTable;

        for (auto child : node.children) {
            child->parent = &node;
            child->symbolTable = node.symbolTable;
            child->accept(*this);
        }

        functionCheck(node, existingFuncEntry, funcType, funcName, symerrors);
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

/*
 * Visitor to add the implementation functions to the struct symbol table. This is done after the symbol table creation
 * to allow for forward referencing.
 * */
class ImplToStructAddingVisitor : public ASTNodeVisitor {
public:
    std::ostream &symerrors;

    explicit ImplToStructAddingVisitor(std::ostream &symerrors) : symerrors(symerrors) {}

    void visit(ImplDefNode &node) override {
        std::string implName = node.children[0]->value;
        auto *globalTable = node.symbolTable->upperScope;
        auto *implEntry = globalTable->lookup(implName, "impl");
        auto *structEntry = globalTable->lookup(implName, "struct");

        if (implEntry == nullptr) {
            return;
        }

        if (structEntry == nullptr) {
            symerrors << "6.3 [error] undeclared struct definition " << implName << std::endl;
            return;
        }

        globalTable->remove(implEntry);
        structEntry->link->symList.emplace_back(implEntry);
        implEntry->link->upperScope = structEntry->link;
        node.symbolTable = structEntry->link;

        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(ImplFuncListNode &node) override {
        auto *structTable = node.symbolTable->upperScope;

        for (auto child : node.children) {
            auto *funcEntry = structTable->lookup(child->children[0]->value, "func");
            if (funcEntry == nullptr) {
                symerrors << "6.1 [error] definition provided for undeclared member function " << node.parent->children[0]->value << "::" << child->children[0]->value << std::endl;
            }

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
};

/* $end symbol table creation visitors */

/* $begin semantic checking visitiors */

/*
 * Visitor to check for semantic errors in the AST intermediate representation.
 * */
class SemanticCheckingVisitor : public ASTNodeVisitor {
public:
    std::ostream &symerrors;

    explicit SemanticCheckingVisitor(std::ostream &symerrors) : symerrors(symerrors) {}

    void visit(FuncDeclNode &node) override {
        auto *structTable = node.parent->symbolTable;
        auto *implEntry = structTable->lookup(structTable->name, "impl");
        if (implEntry == nullptr) return;
        auto *funcEntry =  implEntry->link->lookup(node.children[0]->value, "func");

        if (funcEntry == nullptr) {
            symerrors << "6.2 [error] undefined member function declaration "
                      << node.parent->parent->children[0]->value << "::" << node.children[0]->value << std::endl;
        } else {
            // check for override
            std::vector<std::string> inheritNames = structTable->lookupAllNamesOfKind("inherit");
            if (!inheritNames.empty()) {
                auto *globalTable = structTable->upperScope;

                for (const auto &inheritName: inheritNames) {
                    // look for the struct in the global table
                    auto *inheritedStructEntry = globalTable->lookup(inheritName, "struct");
                    if (inheritedStructEntry == nullptr) return;
                    auto *inheritedStructTable = inheritedStructEntry->link;
                    // consider the fact that the inherited struct might use overloaded member functions
                    auto matchingFuncEntries = inheritedStructTable->lookupAll(funcEntry->name, "func");
                    if (matchingFuncEntries.empty()) continue;
                    // look for function with matching signature
                    for (auto *matchingFuncEntry : matchingFuncEntries) {
                        if (matchingFuncEntry->type == funcEntry->type) {
                            bool sameFParams = true;
                            for (int i = 0; i < funcEntry->link->symList.size(); i++) {
                                if (funcEntry->link->symList[i]->kind != "param") continue;
                                if (funcEntry->link->symList[i]->type != matchingFuncEntry->link->symList[i]->type) {
                                    sameFParams = false;
                                    break;
                                }
                            }

                            if (sameFParams) {
                                symerrors << "9.3 [warning] member function "
                                          << structTable->name << "::" << funcEntry->name
                                          << " overrides inherited function "
                                          << inheritedStructTable->name << "::" << matchingFuncEntry->name << std::endl;
                            }
                        }
                    }
                }
            }
        }

        for (auto child : node.children) {
            child->accept(*this);
        }
    }


    void visit(VarDeclNode &node) override {
        // check if class is declared, doesn't matter the current scope we're in
        auto *currentScope = node.symbolTable;
        auto *globalTable = currentScope;
        while (globalTable->level != 0) {
            globalTable = globalTable->upperScope;
        }

        if (node.children[1]->value != "integer" && node.children[1]->value != "float") {
            auto *structEntry = globalTable->lookup(trimVariableType(node.children[1]->value), "struct");
            if (structEntry == nullptr) {
                symerrors << "11.5 [error] undeclared struct " << node.children[1]->value << " in " << currentScope->name << std::endl;
                return;
            }
        }

        if (node.parent->type == 10 && node.symbolTable->level != 1) {
            auto *implTable = currentScope->upperScope;
            auto *structTable = implTable->upperScope;
            auto *matchingVarEntry = structTable->lookup(node.children[0]->value, "var");
            if (matchingVarEntry != nullptr) {
                symerrors << "8.6 [warning] local variable " << structTable->name << "::" << currentScope->name << "::" << node.children[0]->value
                          << " shadows member variable " << structTable->name << "::" << node.children[0]->value << std::endl;
            }

            inheritanceVariableDeclCheck(node, structTable, globalTable, symerrors, true, currentScope->name);

        } else if (node.parent->type == 26) { // Member
            inheritanceVariableDeclCheck(node, currentScope, globalTable, symerrors, false, currentScope->name);
        }

        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(VariableNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        variableCheck(node, symerrors);
    }

    void visit(IndiceListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        // all children should be integer
        for (auto child : node.children) {
            if (child->semanticType != "integer") {
                symerrors << "13.2 [error] array index " << child->value << " is not an integer at " << node.symbolTable->name << "::" << node.parent->children[0]->value << std::endl;
            }
        }
    }

    void visit(FunctionCallNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        // if the parent node is not a dotnode, then it's a free function call
        if (node.parent->type != 12) {
            // lookup the function in the symbol table
            auto *functionScope = node.symbolTable;
            // of course we can call the free function from anywhere so we have to go to global scope
            auto *globalTable = functionScope->upperScope;
            while (globalTable->level != 0) {
                globalTable = globalTable->upperScope;
            }

            std::vector<SymbolTableEntry*> matchingFuncEntries = globalTable->lookupAll(node.children[0]->value, "func");
            // look for the function with the right number of parameters
            if (matchingFuncEntries.empty()) {
                symerrors << "11.4 [error] undeclared/undefined free function " << node.children[0]->value << std::endl;
                node.semanticType = "errortype";
                return;
            }

            std::vector<ASTNode*> aparams = node.children[1]->children;
            if (matchingFuncEntries.size() == 1) {
                // if the number of matching functions is 1, then we can confidently say that the function call was made with incorrect number of parameters or types
                // which is why I separate this case from multiple matching functions
                auto *funcEntry = matchingFuncEntries[0];
                auto *funcTable = funcEntry->link;
                std::vector<SymbolTableEntry*> fparams = funcTable->lookupAllOfKind("param");
                if (aparams.size() != fparams.size()) {
                    symerrors << "12.1 [error] free function call with wrong number of parameters at " << functionScope->name << std::endl;
                    node.semanticType = "errortype";
                    return;
                }

                bool error = false;
                for (int i = 0; i < aparams.size(); i++) {
                    if (!areTwoVarsTypesEqual(aparams[i]->semanticType, fparams[i]->type)) {
                        error = true;
                    }
                }

                if (error) {
                    std::string aparamList;
                    for (auto aparam : aparams) {
                        aparamList += aparam->semanticType + " ";
                    }

                    symerrors << "12.2 [error] free function call with wrong type of parameters in " << functionScope->name << ". Params: ( " << aparamList << ")"
                              << ", call of " << globalTable->name << "::" << funcEntry->name << std::endl;
                    node.semanticType = "errortype";
                    return;
                }

                node.semanticType = funcEntry->type;
                return;
            }

            node.semanticType = "errortype";
            for (auto *funcEntry : matchingFuncEntries) {
               auto *funcTable = funcEntry->link;
                std::vector<SymbolTableEntry*> fparams = funcTable->lookupAllOfKind("param");
                if (aparams.size() != fparams.size()) {
                    continue;
                }

                for (int i = 0; i < aparams.size(); i++) {
                    if (!areTwoVarsTypesEqual(aparams[i]->semanticType, fparams[i]->type)) {
                        break;
                    }
                }

                node.semanticType = funcEntry->type;
                return;
            }

            // there is no function with name and good parameters
            // so I give the generic error message
            std::string aparamString;
            for (auto aparam : aparams) {
                aparamString += aparam->semanticType + " ";
            }
            symerrors << "12.2 [error] free function call with wrong type of parameters in " << functionScope->name << ". Params: ( " << aparamString << ")"
                      << " call of " << globalTable->name << "::" << node.children[0]->value << std::endl;
        }
    }

    void visit(DotNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *dotParam1 = node.children[0];
        auto *dotParam2 = node.children[1]; // member function call or member variable access

        if (dotParam1->semanticType == "errortype") {
            symerrors << "15.1 [error] . operator used on non-struct type " << dotParam1->value << std::endl;
            node.semanticType = "errortype";
            return;
        }

        if (dotParam1->type == 22) {
            variableCheck(*dotParam1, symerrors);
        }

        if (dotParam1->semanticType == "errortype") {
            node.semanticType = "errortype";
            return;
        } else if (dotParam1->semanticType == "integer" || dotParam1->semanticType == "float") {
            symerrors << "15.1 [error] . operator used on non-struct " << dotParam1->value << " of type " << dotParam1->semanticType << std::endl;
            node.semanticType = "errortype";
            return;
        } else if (dotParam1->type != 12) {
            auto *functionScope = node.symbolTable;
            auto *globalTable = functionScope->upperScope;
            while (globalTable->level != 0) {
                globalTable = globalTable->upperScope;
            }
            auto *structEntry = globalTable->lookup(dotParam1->semanticType, "struct");
            if (structEntry == nullptr) {
                symerrors << "15.1 [error] . operator used on non-struct " << dotParam1->value << " of type " << dotParam1->semanticType << std::endl;
                node.semanticType = "errortype";
                return;
            }
        }

        // the result will be the result of the second parameter
        if (dotParam2->type == 22) {
            variableCheck(*dotParam2, symerrors);
            node.semanticType = dotParam2->semanticType;
        } else if (dotParam2->type == 17) {
            // is a member function call
            std::vector<ASTNode*> aparams = dotParam2->children[1]->children;

            auto *functionScope = node.symbolTable;
            auto *globalTable = functionScope->upperScope;
            while (globalTable->level != 0) {
                globalTable = globalTable->upperScope;
            }

            auto *structEntry = globalTable->lookup(dotParam1->semanticType, "struct");
            auto *structTable = structEntry->link;

            std::vector<SymbolTableEntry*> matchingFuncEntries = structTable->lookupAll(dotParam2->children[0]->value, "func");
            // in theory we can have overloaded functions in the inheritee struct that aren't actually overridden per se
            // check local
            if (!matchingFuncEntries.empty()) {
                if (matchingFuncEntries.size() == 1) {
                    auto *funcEntry = matchingFuncEntries[0];
                    auto *funcTable = funcEntry->link;
                    std::vector<SymbolTableEntry*> fparams = funcTable->lookupAllOfKind("param");
                    if (aparams.size() != fparams.size()) {
                        symerrors << "12.1 [error] member function call with wrong number of parameters at " << structTable->name << "::" << functionScope->name << std::endl;
                        node.semanticType = "errortype";
                        return;
                    }

                    bool error = false;
                    for (int i = 0; i < aparams.size(); i++) {
                        if (!areTwoVarsTypesEqual(aparams[i]->semanticType, fparams[i]->type)) {
                            error = true;
                        }
                    }

                    if (error) {
                        std::string aparamList;
                        for (auto aparam : aparams) {
                            aparamList += aparam->semanticType + " ";
                        }

                        symerrors << "12.2 [error] member function call with wrong type of parameters at " << functionScope->name << " " << structTable->name << "::" << funcEntry->name << std::endl;
                        node.semanticType = "errortype";
                        return;
                    }

                    node.semanticType = funcEntry->type;
                    return;
                }

                for (auto *funcEntry : matchingFuncEntries) {
                    std::vector<SymbolTableEntry*> fparams = funcEntry->link->lookupAllOfKind("param");
                    if (aparams.size() != fparams.size()) {
                        continue;
                    }

                    for (int i = 0; i < aparams.size(); i++) {
                        if (!areTwoVarsTypesEqual(aparams[i]->semanticType, fparams[i]->type)) {
                            break;
                        }
                    }

                    // found a match
                    node.semanticType = funcEntry->type;
                    return;
                }
            }

            // check in inherited
            std::vector<std::string> inheritNames = structTable->lookupAllNamesOfKind("inherit");
            if (inheritNames.empty()) {
                if (matchingFuncEntries.empty())
                    symerrors << "11.2 [error] undeclared member function " << dotParam1->semanticType << "::" << dotParam2->children[0]->value << std::endl;
                else
                    symerrors << "12.2 [error] member function call with wrong type of parameters at " << structTable->name << "::" << functionScope->name << std::endl;
            }

            for (const auto &inheritName : inheritNames) {
                auto *inheritedStructEntry = globalTable->lookup(inheritName, "struct");
                if (inheritedStructEntry == nullptr) return; // already checked for this
                auto *inheritedStructTable = inheritedStructEntry->link;
                matchingFuncEntries = inheritedStructTable->lookupAll(dotParam2->children[0]->value, "func");
                if (!matchingFuncEntries.empty()) {
                    break;
                }

                if (matchingFuncEntries.size() == 1) {
                    auto *funcEntry = matchingFuncEntries[0];
                    auto *funcTable = funcEntry->link;
                    std::vector<SymbolTableEntry*> fparams = funcTable->lookupAllOfKind("param");
                    if (aparams.size() != fparams.size()) {
                        symerrors << "12.1 [error] inherited member function call with wrong number of parameters at " << structTable->name << "." << inheritedStructTable->name << "::" << functionScope->name << std::endl;
                        node.semanticType = "errortype";
                        return;
                    }

                    bool error = false;
                    for (int i = 0; i < aparams.size(); i++) {
                        if (trimVariableType(aparams[i]->semanticType) != trimVariableType(fparams[i]->type)) {
                            error = true;
                        }
                    }

                    if (error) {
                        std::string aparamList;
                        for (auto aparam : aparams) {
                            aparamList += aparam->semanticType + " ";
                        }

                        symerrors << "12.2 [error] inherited member function call with wrong type of parameters at " << structTable->name << "." << inheritedStructTable->name << "::" << functionScope->name << std::endl;
                        node.semanticType = "errortype";
                        return;
                    }

                    node.semanticType = funcEntry->type;
                    return;
                }

                for (auto *funcEntry : matchingFuncEntries) {
                    std::vector<SymbolTableEntry*> fparams = funcEntry->link->lookupAllOfKind("param");
                    if (aparams.size() != fparams.size()) {
                        continue;
                    }

                    for (int i = 0; i < aparams.size(); i++) {
                        if (trimVariableType(aparams[i]->semanticType) != trimVariableType(fparams[i]->type)) {
                            break;
                        }
                    }

                    // found a match
                    node.semanticType = funcEntry->type;
                    return;
                }
            }

            // exhausted all inherited structs
            symerrors << "12.2 [error] member function call with wrong type of parameters at " << structTable->name << "::" << functionScope->name << std::endl;
            node.semanticType = "errortype";
        } else {
            symerrors << "15.1 [error] . operator right hand side is not a member function call or member variable access at " << dotParam1->value << "." << dotParam2->value << std::endl;
            node.semanticType = "errortype";
        }

    }

    void visit(AParamsListNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(AssignStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *left = node.children[0];
        // middle is assignop node
        auto *right = node.children[2];

        if (left->semanticType == "errortype" || right->semanticType == "errortype") {
            node.semanticType = "errortype";
            symerrors << "10.2 [error] assignment of " << left->semanticType << " to " << right->semanticType << " in " << node.symbolTable->name << std::endl;
            return;
        }

        if (left->type == 22) {
            variableCheck(*left, symerrors);
        }

        std::string leftSemanticTypeTrimmed = trimVariableType(left->semanticType);
        std::string rightSemanticTypeTrimmed = trimVariableType(right->semanticType);

        if (leftSemanticTypeTrimmed != rightSemanticTypeTrimmed) {
            symerrors << "10.2 [error] assignment of " << left->semanticType << " to " << right->semanticType << " in " << node.symbolTable->name << std::endl;

            node.semanticType = "errortype";
        }
    }

    void visit(ReturnStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        std::string type = node.parent->parent->symbolTableEntry->type;

        if (node.children[0]->semanticType != type) {
            symerrors << "10.3 [error] return type mismatch " << node.children[0]->semanticType << " and " << type << std::endl;
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

        auto *left = node.children[0];
        auto *right = node.children[1];

        if (left->semanticType == "errortype" || right->semanticType == "errortype") {
            node.semanticType = "errortype";
            return;
        } else if (left->semanticType != right->semanticType) {
            symerrors << "10.1 [error] type mismatch in addition/subtraction operation " << left->semanticType << " and " << right->semanticType << std::endl;
        } else {
            node.semanticType = left->semanticType;
        }
    }

    void visit(MultOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *left = node.children[0];
        auto *right = node.children[1];
        if (left->semanticType == "errortype" || right->semanticType == "errortype") {
            node.semanticType = "errortype";
            return;
        } else if (left->semanticType != right->semanticType) {
            symerrors << "10.1 [error] type mismatch in multiplication/division operation " << left->semanticType << " and " << right->semanticType << std::endl;
        } else {
            node.semanticType = left->semanticType;
        }
    }

    void visit(AssignOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
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

    void visit(MemberListNode &node) override {
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
        node.semanticType = "integer";
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

        auto *left = node.children[0];
        auto *right = node.children[2];

        if (left->semanticType == "errortype" || right->semanticType == "errortype") {
            node.semanticType = "errortype";
            return;
        } else if (left->semanticType != right->semanticType) {
            symerrors << "10.1 [error] type mismatch in relational operation " << left->semanticType << " and " << right->semanticType << std::endl;
        } else {
            node.semanticType = "boolean";
        }

    }

    void visit(FloatlitNode &node) override {
        node.semanticType = "float";
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

    void visit(EpsilonNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }
};

/* $end semantic checking visitors */


#endif //COMPILER_SEMANTIC_HPP
