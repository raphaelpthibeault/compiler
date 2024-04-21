#ifndef COMPILER_CODEGEN_HPP
#define COMPILER_CODEGEN_HPP

#include <ast.hpp>
#include <stack>
#include <semantic.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

const int INT_SIZE = 4;
const int FLOAT_SIZE = 8;
const std::string indent = "          "; // len = 10
const int indentLength = 10;

void computeSizes(ASTNode &root);
void generateCode(ASTNode &root, std::ostream &out);

int sizeofTable(SymbolTable *table, bool isStruct);
int sizeofEntry(SymbolTableEntry *entry, SymbolTable *currentScope);
int sizeofType(std::string &type, SymbolTable *currentScope);


inline bool isArrayType(const std::string &type) {
    return type.find('[') != std::string::npos;
}

inline bool isBaseType(const std::string &type) {
    return type == "integer" || type == "float";
}


/*
 * print the scope
 * */
inline void printScope(SymbolTable *table, std::ostream &out) {
    out << "Scope: " << table->name << " size: " << table->size << " offset: " << table->offset << std::endl;
    for (auto entry : table->symList) {
        out << "  " << entry->name << " " << entry->type << " size: " << entry->size << " offset: " << entry->offset << std::endl;
    }
    out << std::endl;
}

/*
 * Calculate the dimensions of an array
 * */
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

    void visit(FuncDefNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        // unnecessary if, but for clarity
        if (node.symbolTableEntry->name == "main") {
            for (auto entry : node.symbolTable->symList) {
                entry->offset = node.symbolTable->size - entry->size;
                node.symbolTable->size -= entry->size;
            }
        } else {
            // stack frame contains the return value at the bottom of the stack
            node.symbolTable->size = - sizeofType(node.symbolTableEntry->type, node.symbolTable);
            // return address
            node.symbolTable->size -= INT_SIZE;
            // old frame pointer
            node.symbolTable->size -= INT_SIZE;


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

        // skip if in struct member decl
        if (node.symbolTable->upperScope->lookup(node.symbolTable->name, "struct") != nullptr) {
            return;
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", "integer", nullptr);
        tempVarEntry->size = INT_SIZE;
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(FloatlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", "float", nullptr);
        tempVarEntry->size = FLOAT_SIZE;
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(VariableNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
        /*
        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", node.semanticType, nullptr);
        tempVarEntry->size = sizeofType(node.semanticType, node.symbolTable);
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);*/
    }

    void visit(AddOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(MultOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(RelExprNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *tempVarEntry = new SymbolTableEntry(getTempVarName(), "tempvar", trimVariableType(node.semanticType), nullptr);
        std::string type = trimVariableType(node.semanticType);
        tempVarEntry->size = sizeofType(type, node.symbolTable);
        node.symbolTableEntry = tempVarEntry;
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
        node.symbolTableEntry = tempVarEntry;
        node.symbolTable->insert(tempVarEntry);
    }

    void visit(FuncDeclNode &node) override {
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
    std::ostream &moonExecCode;
    std::ostream &moonDataCode;

    explicit CodeGenerationVisitor(std::ostream &moonExecCode = std::cout, std::ostream &moonDataCode = std::cout)
            : moonExecCode(moonExecCode), moonDataCode(moonDataCode) {
        for (int i = 11; i >= 1; i--) {
            registerPool.push("r" + std::to_string(i));
        }
    }


    void visit(FuncDefNode &node) override {
        if (node.symbolTableEntry->name == "main") {
            exec() << "align\n";
            exec() << + "entry\n";
            exec() << "% initialize stack pointer\n";
            addi(SP, ZR, "topaddr");
            exec() << "% initialize frame pointer\n";
            addi(FP, ZR, "topaddr");

            exec() << + "% program begins\n";
            for (auto child : node.children) {
                child->accept(*this);
            }

            exec() << + "% program ends\n";
            exec() << + "hlt\n";
            exec() << "align\n";

            data() << "% buffer space used for console output\n";
            data("buf") << "res 20\n";
            data("cr") << "db 13,10,0\n";
        } else {
            bool isFree = node.symbolTable->upperScope->upperScope == nullptr;
            if (isFree) {
                exec() << "align\n";
                exec(node.symbolTableEntry->name) << "% funcdef " << node.symbolTableEntry->name << "\n";

                /* function prolog */
                // put JL on stack frame


                exec("% function prolog\n");
                exec("% save old frame pointer\n");
                sw(16, SP, FP);

                exec("% change frame pointer\n");
                addi(FP, SP, 24); // hard-coded TODO



                for (auto child : node.children) {
                    child->accept(*this);
                }



                /* function epilog */
                exec("% function epilog");
                exec("% restore old stack pointer");
                addi(SP, SP, node.symbolTable->size);
                exec("% restore old frame pointer");
                lw(FP, 16, SP);


                // get r15 from stack frame



                jr(JL);
                exec("% end of funcdef " + node.symbolTableEntry->name) << "\n";
            }

        }
    }

    void visit(VarDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        if (isArrayType(node.symbolTableEntry->type) && isBaseType(trimVariableType(node.symbolTableEntry->type))) {
            // integer or float array
            exec() << "% allocate space for array " << node.symbolTableEntry->name << "\n";
            subi(SP, SP, node.symbolTableEntry->size);

        } else if (isArrayType(node.symbolTableEntry->type)) {
            // array of struct objects
            exec() << "% allocate space for struct array " << node.symbolTableEntry->name << " type " << node.symbolTableEntry->type << "\n";
            subi(SP, SP, node.symbolTableEntry->size);
        } else if (!isBaseType(node.symbolTableEntry->type)) {
            exec() << "% allocate space for struct " << node.symbolTableEntry->name << " type " << node.symbolTableEntry->type << "\n";
            subi(SP, SP, node.symbolTableEntry->size);

        } else {
            // int or float
            if (node.symbolTableEntry->type == "integer") {
                subi(SP, SP, INT_SIZE);
            } else if (node.symbolTableEntry->type == "float") {
                // TODO
                //subi(SP, SP, FLOAT_SIZE);
            }
        }

    }

    void visit(IntlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        if (node.symbolTable->upperScope->lookup(node.symbolTable->name, "struct") != nullptr) {
            return;
        }

        exec() << "% allocate space for int literal " << node.symbolTableEntry->name << ":=" << node.value << "\n";

        std::string localRegister1 = getRegister();
        addi(localRegister1, ZR, node.value);
        sw(node.symbolTableEntry->offset, FP, localRegister1);
        freeRegister(localRegister1);

        subi(SP, SP, INT_SIZE);
    }


    void visit(AssignStatNode &node) override {
        ASTNode *lhs = node.children[0];
        ASTNode *rhs = node.children[2];

        for (auto child : node.children) {
            child->accept(*this);
        }

        /* options:
         * x[a] = y[b]
         * x[a] = y
         * x = y[b]
         * x.a = y.b
         * x.a = y
         * x = y.b
         * x = y
         * */
        if (lhs->type == ASTNodeType::Dot && rhs->type == ASTNodeType::Dot) {
            // TODO
        } else if (lhs->type == ASTNodeType::Dot) {
            // get struct table
            auto *globalScope = lhs->symbolTable->upperScope;
            while (globalScope->upperScope != nullptr) {
                globalScope = globalScope->upperScope;
            }

            auto *structEntry = globalScope->lookup(lhs->children[0]->semanticType, "struct");
            auto *structTable = structEntry->link;

            auto *lhsChild1 = lhs->children[0];
            auto *lhsChild2 = lhs->children[1];

            if (lhsChild2->type == ASTNodeType::Variable) {
                SymbolTableEntry *arrEntry = nullptr;
                int offset = lhsChild1->symbolTableEntry->offset;

                for (auto entry : structTable->symList) {
                    if (entry->name == lhsChild2->children[0]->value) {
                        arrEntry = entry;
                        break;
                    }
                    offset += entry->size;
                }

                std::string localRegister1 = getRegister();
                std::string localRegister2 = getRegister();
                std::string localRegister3 = getRegister();

                int dims = getDimsSize(arrEntry->type);
                int size = arrEntry->size;
                int sizeofElement = size / dims;

                lw(localRegister2, rhs->symbolTableEntry->offset, FP);

                std::vector<ASTNode *> indices = lhs->children[1]->children[1]->children;

                if (indices.size() == 1) {
                    addi(localRegister1, ZR, offset); // get array offset
                    lw(localRegister3, indices[0]->symbolTableEntry->offset, FP); // get index

                    muli(localRegister3, localRegister3, sizeofElement); // calculate offset
                    add(localRegister3, localRegister1, localRegister3); // get element offset

                    add(localRegister3, FP, localRegister3); // actual address

                    sw(0, localRegister3, localRegister2); // store value
                }

                freeRegister(localRegister3);
                freeRegister(localRegister2);
                freeRegister(localRegister1);

            } else if (lhsChild2->type == ASTNodeType::Id) {
                int offset = lhsChild1->symbolTableEntry->offset;

                for (auto entry : structTable->symList) {
                    if (entry->name == lhsChild2->value) {
                        break;
                    }
                    offset += entry->size;
                }

                std::string localRegister1 = getRegister();
                std::string localRegister2 = getRegister();

                lw(localRegister1, rhs->symbolTableEntry->offset, FP);
                sw(offset, FP, localRegister1);

                freeRegister(localRegister2);
                freeRegister(localRegister1);
            }
        } else if (rhs->type == ASTNodeType::Dot) {
            // TODO
        } else if (isArrayType(lhs->semanticType) && isArrayType(rhs->semanticType)) {
            // TODO
        }
        else if (isArrayType(lhs->semanticType)) {
            exec() << "% assignment of " << lhs->symbolTableEntry->name << " = " << rhs->symbolTableEntry->name << "\n";

            std::string localRegister1 = getRegister();
            std::string localRegister2 = getRegister();
            std::string localRegister3 = getRegister();

            // load rhs
            lw(localRegister2, rhs->symbolTableEntry->offset, FP);

            int dims = getDimsSize(lhs->semanticType);
            int size = lhs->symbolTableEntry->size;
            int sizeofElement = size / dims;

            // load lhs, has to be calculated
            std::vector<ASTNode *> indices = lhs->children[1]->children; // intlitnodes

            if (indices.size() == 1) {
                addi(localRegister1, ZR, lhs->symbolTableEntry->offset); // get array offset
                lw(localRegister3, indices[0]->symbolTableEntry->offset, FP); // get index

                muli(localRegister3, localRegister3, sizeofElement); // calculate offset
                add(localRegister3, localRegister1, localRegister3); // get element offset

                add(localRegister3, FP, localRegister3); // actual address

                sw(0, localRegister3, localRegister2); // store value
            }


            freeRegister(localRegister3);
            freeRegister(localRegister2);
            freeRegister(localRegister1);
        }
        else if (isArrayType(rhs->semanticType)) {
            /* x = y[a][b]...
             * */
            exec() << "% assignment of " << lhs->symbolTableEntry->name << " = " << rhs->symbolTableEntry->name << "\n";

            std::string localRegister1 = getRegister();
            std::string localRegister2 = getRegister();
            std::string localRegister3 = getRegister();

            // load rhs
            int dims = getDimsSize(rhs->semanticType);
            int size = rhs->symbolTableEntry->size;
            int sizeofElement = size / dims;

            std::vector<ASTNode *> indices = rhs->children[1]->children;

            if (indices.size() == 1) {
                addi(localRegister1, ZR, rhs->symbolTableEntry->offset); // get array offset
                lw(localRegister3, indices[0]->symbolTableEntry->offset, FP); // get index

                muli(localRegister3, localRegister3, sizeofElement); // calculate offset
                add(localRegister3, localRegister1, localRegister3); // get element offset

                add(localRegister3, FP, localRegister3); // actual address

                lw(localRegister2, 0, localRegister3); // get value
                sw(lhs->symbolTableEntry->offset, FP, localRegister2); // store value
            }

            freeRegister(localRegister3);
            freeRegister(localRegister2);
            freeRegister(localRegister1);
        } else {
            exec() << "% assignment of " << lhs->symbolTableEntry->name << " = " << rhs->symbolTableEntry->name << "\n";
            std::string localRegister1 = getRegister();
            lw(localRegister1, rhs->symbolTableEntry->offset, FP);
            sw(lhs->symbolTableEntry->offset, FP, localRegister1);
            freeRegister(localRegister1);
        }
    }

    void visit(WriteStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        /* options
         *  write(member);
         *  write(array);
         *  write(local);
         * */

        auto *writtenNode = node.children[0];

        std::string localRegister1 = getRegister();
        std::string localRegister2 = getRegister();
        if (writtenNode->type == ASTNodeType::Dot) {
            std::string localRegister3 = getRegister();
            auto *globalScope = writtenNode->symbolTable->upperScope;
            while (globalScope->upperScope != nullptr) {
                globalScope = globalScope->upperScope;
            }

            auto *structEntry = globalScope->lookup(writtenNode->children[0]->semanticType, "struct");
            auto *structTable = structEntry->link;

            auto *lhsChild1 = writtenNode->children[0];
            auto *lhsChild2 = writtenNode->children[1];

            int offset = lhsChild1->symbolTableEntry->offset;

            // if lhsChild2 is an array
            std::vector<ASTNode *> indices = lhsChild2->children[1]->children;
            if (indices.empty()) {
                for (auto entry : structTable->symList) {
                    if (entry->name == lhsChild2->children[0]->value) {
                        break;
                    }
                    offset += entry->size;
                }

                lw(localRegister1, offset, FP);
                exec() << "% put value on stack \n";
                addi(SP, SP, node.symbolTable->size);
                exec() << "sw -8(r14), " << localRegister1 << "\n";
                exec() << "% link buffer to stack\n";
                addi(localRegister1, ZR, "buf");
                exec() << "% convert int to string\n";
                sw(-12, SP, localRegister1);
                jl(JL, "intstr");
                exec() << "% receive return value\n";
                sw(-8, SP, RV);
                exec() << "% print string\n";
                jl(JL, "putstr");
                subi(SP, SP, node.symbolTable->size);
            } else if (indices.size() == 1) {
                SymbolTableEntry *arrEntry = nullptr;
                for (auto entry : structTable->symList) {
                    if (entry->name == lhsChild2->children[0]->value) {
                        arrEntry = entry;
                        break;
                    }
                    offset += entry->size;
                }

                int dims = getDimsSize(arrEntry->type);
                int size = arrEntry->size;
                int sizeofElement = size / dims;


                addi(localRegister1, ZR, offset); // get array offset
                lw(localRegister3, indices[0]->symbolTableEntry->offset, FP); // get index

                muli(localRegister3, localRegister3, sizeofElement); // calculate offset
                add(localRegister3, localRegister1, localRegister3); // get element offset

                add(localRegister3, FP, localRegister3); // actual address

                lw(localRegister1, 0, localRegister3);

                exec() << "% put value on stack \n";
                addi(SP, SP, node.symbolTable->size);
                exec() << "sw -8(r14), " << localRegister1 << "\n";
                exec() << "% link buffer to stack\n";
                addi(localRegister1, ZR, "buf");
                exec() << "% convert int to string\n";
                sw(-12, SP, localRegister1);
                jl(JL, "intstr");
                exec() << "% receive return value\n";
                sw(-8, SP, RV);
                exec() << "% print string\n";
                jl(JL, "putstr");
                subi(SP, SP, node.symbolTable->size);
            }

            freeRegister(localRegister3);
        } else if (isArrayType(writtenNode->symbolTableEntry->type)) {
            std::string localRegister3 = getRegister();

            exec() << "% write " << writtenNode->symbolTableEntry->name << "\n";
            lw(localRegister1, writtenNode->symbolTableEntry->offset, FP);

            int dims = getDimsSize(writtenNode->semanticType);
            int size = writtenNode->symbolTableEntry->size;
            int sizeofElement = size / dims;

            std::vector<ASTNode *> indices = writtenNode->children[1]->children;

            if (indices.size() == 1) {
                addi(localRegister1, ZR, writtenNode->symbolTableEntry->offset); // get array offset
                lw(localRegister3, indices[0]->symbolTableEntry->offset, FP); // get index

                muli(localRegister3, localRegister3, sizeofElement); // calculate offset
                add(localRegister3, localRegister1, localRegister3); // get element offset

                add(localRegister3, FP, localRegister3); // actual address

                lw(localRegister1, 0, localRegister3);
                freeRegister(localRegister3);

                exec() << "% put value on stack \n";
                addi(SP, SP, node.symbolTable->size);
                exec() << "sw -8(r14), " << localRegister1 << "\n";
                exec() << "% link buffer to stack\n";
                addi(localRegister1, ZR, "buf");
                exec() << "% convert int to string\n";
                sw(-12, SP, localRegister1);
                jl(JL, "intstr");
                exec() << "% receive return value\n";
                sw(-8, SP, RV);
                exec() << "% print string\n";
                jl(JL, "putstr");
                subi(SP, SP, node.symbolTable->size);
            }
        } else {
            exec() << "% write " << writtenNode->symbolTableEntry->name << "\n";
            lw(localRegister1, writtenNode->symbolTableEntry->offset, FP);
            exec() << "% put value on stack \n";
            addi(SP, SP, node.symbolTable->size);
            exec() << "sw -8(r14), " << localRegister1 << "\n";
            exec() << "% link buffer to stack\n";
            addi(localRegister1, ZR, "buf");
            exec() << "% convert int to string\n";
            sw(-12, SP, localRegister1);
            jl(JL, "intstr");
            exec() << "% receive return value\n";
            sw(-8, SP, RV);
            exec() << "% print string\n";
            jl(JL, "putstr");
            subi(SP, SP, node.symbolTable->size);
        }

        printNewLine(localRegister1, localRegister2);

        freeRegister(localRegister2);
        freeRegister(localRegister1);
    }


    void visit(AddOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
        /* <addop> ::= + | - | '|'
         * */

        auto *lhs = node.children[0];
        std::string op = node.value;
        auto *rhs = node.children[1];

        if (op == "+" || op == "-") {
            if (isArrayType(lhs->semanticType) && isArrayType(rhs->semanticType)) {
                // TODO
            } else if (isArrayType(lhs->semanticType)) {
                // TODO
            } else if (isArrayType(rhs->semanticType)) {
                // TODO
            } else if (lhs->type == ASTNodeType::Dot && rhs->type == ASTNodeType::Dot) {
                // TODO
            } else if (lhs->type == ASTNodeType::Dot) {
                // TODO
            } else if (rhs->type == ASTNodeType::Dot) {
                // TODO
            } else {
                /* x + y
                 * lw r1,x_offset(r14)
                 * lw r2,y_offset(r14)
                 * add r3,r1,r2
                 * sw temp_offset(r14),r3
                 * */
                exec() << "% " << node.symbolTableEntry->name << " ::= " << lhs->symbolTableEntry->name << " " << op << " " << rhs->symbolTableEntry->name << "\n";
                std::string localRegister1 = getRegister();
                std::string localRegister2 = getRegister();
                std::string localRegister3 = getRegister();

                lw(localRegister1, lhs->symbolTableEntry->offset, FP);
                lw(localRegister2, rhs->symbolTableEntry->offset, FP);

                if (op == "+")
                    add(localRegister3, localRegister1, localRegister2);
                else
                    sub(localRegister3, localRegister1, localRegister2);

                sw(node.symbolTableEntry->offset, FP, localRegister3);

                freeRegister(localRegister3);
                freeRegister(localRegister2);
                freeRegister(localRegister1);
            }
        } else if (op == "|") {
            /* x | y
             * lw r1,x_offset(r14)
             * lw r2,y_offset(r14)
             * bnz r1, tag1
             * bnz r2, tag1
             * addi r3,r0,0
             * j tag2
             * tag1: addi r3,r0,1
             * tag2: sw temp_offset(r14),r3
             * */

            exec() << "% " << node.symbolTableEntry->name << " ::= " << lhs->symbolTableEntry->name << op << rhs->symbolTableEntry->name << "\n";
            std::string localRegister1 = getRegister();
            std::string localRegister2 = getRegister();
            std::string localRegister3 = getRegister();

            std::string tag1 = getTag();
            std::string tag2 = getTag();

            lw(localRegister1, lhs->symbolTableEntry->offset, FP);
            lw(localRegister2, rhs->symbolTableEntry->offset, FP);
            bnz(localRegister1, tag1);
            bnz(localRegister2, tag1);
            addi(localRegister3, ZR, 0);
            j(tag2);
            exec(tag1) << "addi " << localRegister3 << "," << ZR << ",1\n";
            exec(tag2) << "sw " << node.symbolTableEntry->offset << "(" << FP << ")," << localRegister3 << "\n";

            freeRegister(localRegister3);
            freeRegister(localRegister2);
            freeRegister(localRegister1);
        }
    }

    void visit(MultOpNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
        /* <multop> ::= * | / | &
         * */

        auto *lhs = node.children[0];
        std::string op = node.value;
        auto *rhs = node.children[1];

        if (op == "*" || op == "/") {
            if (isArrayType(lhs->semanticType) && isArrayType(rhs->semanticType)) {
                // TODO
            } else if (isArrayType(lhs->semanticType)) {
                // TODO
            } else if (isArrayType(rhs->semanticType)) {
                // TODO
            } else if (lhs->type == ASTNodeType::Dot && rhs->type == ASTNodeType::Dot) {
                // TODO
            } else if (lhs->type == ASTNodeType::Dot) {
                // TODO
            } else if (rhs->type == ASTNodeType::Dot) {
                // TODO
            } else {
                /* x * y
                 * lw r1,x_offset(r14)
                 * lw r2,y_offset(r14)
                 * mul r3,r1,r2
                 * sw temp_offset(r14),r3
                 * */

                std::string localRegister1 = getRegister();
                std::string localRegister2 = getRegister();
                std::string localRegister3 = getRegister();

                exec() << "% " << node.symbolTableEntry->name << " ::= " << lhs->symbolTableEntry->name << " " << op << " " << rhs->symbolTableEntry->name << "\n";
                lw(localRegister1, lhs->symbolTableEntry->offset, FP);
                lw(localRegister2, rhs->symbolTableEntry->offset, FP);

                if (op == "*")
                    mul(localRegister3, localRegister1, localRegister2);
                else
                    div(localRegister3, localRegister1, localRegister2);

                sw(node.symbolTableEntry->offset, FP, localRegister3);

                freeRegister(localRegister3);
                freeRegister(localRegister2);
                freeRegister(localRegister1);
            }
        } else if (op == "&") {
            /* x & y
             * lw r1,x_offset(r14)
             * lw r2,y_offset(r14)
             * bz r1, tag1
             * bz r2, tag1
             * addi r3,r0,1
             * j tag2
             * tag1: addi r3,r0,0
             * tag2: sw temp_offset(r14),r3
             * */

            std::string localRegister1 = getRegister();
            std::string localRegister2 = getRegister();
            std::string localRegister3 = getRegister();

            std::string tag1 = getTag();
            std::string tag2 = getTag();

            exec() << "% " << node.symbolTableEntry->name << " ::= " << lhs->symbolTableEntry->name << op << rhs->symbolTableEntry->name << "\n";

            lw(localRegister1, lhs->symbolTableEntry->offset, FP);
            lw(localRegister2, rhs->symbolTableEntry->offset, FP);
            bz(localRegister1, tag1);
            bz(localRegister2, tag1);
            addi(localRegister3, ZR, 1);
            j(tag2);
            exec(tag1) << "addi " << localRegister3 << "," << ZR << ",0\n";
            exec(tag2) << "sw " << node.symbolTableEntry->offset << "(" << FP << ")," << localRegister3 << "\n";

            freeRegister(localRegister3);
            freeRegister(localRegister2);
            freeRegister(localRegister1);
        }
    }

    void visit(RelExprNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        auto *lhs = node.children[0];
        auto *op = node.children[1];
        auto *rhs = node.children[2];

        if (isArrayType(lhs->semanticType) && isArrayType(rhs->semanticType)) {
            // TODO
        } else if (isArrayType(lhs->semanticType)) {
            // TODO
        } else if (isArrayType(rhs->semanticType)) {
            // TODO
        } else if (lhs->type == ASTNodeType::Dot && rhs->type == ASTNodeType::Dot) {
            // TODO
        } else if (lhs->type == ASTNodeType::Dot) {
            // TODO
        } else if (rhs->type == ASTNodeType::Dot) {
            // TODO
        } else {
            /* x compare_op y
             *
             * lw r1,x_offset(r14)
             * lw r2,y_offset(r14)
             *
             * compare_instr r3,r1,r2
             * sw temp_offset(r14),r3
             * */

            std::string localRegister1 = getRegister();
            std::string localRegister2 = getRegister();
            std::string localRegister3 = getRegister();

            exec() << "% " << node.symbolTableEntry->name << " ::= " << lhs->symbolTableEntry->name << " " << op->value << " " << rhs->symbolTableEntry->name << "\n";
            lw(localRegister1, lhs->symbolTableEntry->offset, FP);
            lw(localRegister2, rhs->symbolTableEntry->offset, FP);

            if (op->value == "==") {
                ceq(localRegister3, localRegister1, localRegister2);
            } else if (op->value == "!=") {
                cne(localRegister3, localRegister1, localRegister2);
            } else if (op->value == "<") {
                clt(localRegister3, localRegister1, localRegister2);
            } else if (op->value == "<=") {
                cle(localRegister3, localRegister1, localRegister2);
            } else if (op->value == ">") {
                cgt(localRegister3, localRegister1, localRegister2);
            } else if (op->value == ">=") {
                cge(localRegister3, localRegister1, localRegister2);
            }

            sw(node.symbolTableEntry->offset, FP, localRegister3);

            freeRegister(localRegister3);
            freeRegister(localRegister2);
            freeRegister(localRegister1);
        }
    }

    void visit(IfStatNode &node) override {
        exec() << "% if statement\n";
        // visit relexpr
        node.children[0]->accept(*this);

        int offset = node.children[0]->symbolTableEntry->offset;
        std::string elseTag = getTag();
        std::string endifTag = getTag();

        std::string localRegister1 = getRegister();
        lw(localRegister1, offset, FP);

        bz(localRegister1, elseTag);
        freeRegister(localRegister1);
        // code for statblock
        node.children[1]->accept(*this);
        j(endifTag);

        // code for statblock
        exec(elseTag) << "% else statement\n";
        node.children[2]->accept(*this);
        exec(endifTag) << "% end if statement\n";
    }

    void visit(WhileStatNode &node) override {
        int offset = node.children[0]->symbolTableEntry->offset;
        std::string whileTag = getTag();
        std::string endwhileTag = getTag();

        exec(whileTag) << "% while statement\n";
        // visit relexpr
        node.children[0]->accept(*this);

        std::string localRegister1 = getRegister();
        lw(localRegister1, offset, FP);
        bz(localRegister1, endwhileTag);
        freeRegister(localRegister1);

        // code for statblock
        node.children[1]->accept(*this);
        j(whileTag);
        exec(endwhileTag) << "% end while statement\n";
    }


    void visit(FunctionCallNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }

        SymbolTableEntry *funcEntry;

        auto *globalScope = node.symbolTable->upperScope;
        while (globalScope->upperScope != nullptr) {
            globalScope = globalScope->upperScope;
        }
        funcEntry = globalScope->lookup(node.children[0]->value, "func");

        /* reserve return value space */
        exec("% reserve return value space\n");
        subi(SP, SP, sizeofType(funcEntry->type, funcEntry->link));

        /* reserve old frame pointer space */
        exec("% reserve old frame pointer space\n");
        subi(SP, SP, INT_SIZE);

        /* reserve return address space */
        exec("% reserve return address space\n");
        subi(SP, SP, INT_SIZE);
        exec("% store return address\n");
        sw(0, SP, JL); // store return address

        /* push params */
        exec("% push params\n");
        std::vector<ASTNode *> aparams = node.children[1]->children;

        for (auto & aparam : aparams) {
            auto *aparamEntry = aparam->symbolTableEntry;
            exec("% push param " + aparamEntry->name + "\n");
            std::string localRegister2 = getRegister();
            subi(SP, SP, aparamEntry->size); // reserve space
            lw(localRegister2, aparamEntry->offset, FP);
            sw(0, SP, localRegister2);
            freeRegister(localRegister2);
        }
        exec("% params done\n");



        /* jump to function */
        exec("% jump to function\n");
        jl(JL, funcEntry->name);


        /*
         * get return value
         * */
        /* pop params */
        exec("% pop params\n");
        for (auto & aparam : aparams) {
            auto *aparamEntry = aparam->symbolTableEntry;
            exec("% pop param " + aparamEntry->name + "\n");
            std::string localRegister2 = getRegister();
            lw(localRegister2, 0, SP);
            sw(aparamEntry->offset, FP, localRegister2);
            addi(SP, SP, aparamEntry->size); // free space
            freeRegister(localRegister2);
        }

        /* free return address space */
        exec("% free return address space\n");
        addi(SP, SP, INT_SIZE);

        /* free old frame pointer space */
        exec("% free old frame pointer space\n");
        addi(SP, SP, INT_SIZE);


        /* get return value */
        exec("% get return value\n");
        std::string localRegister1 = getRegister();
        lw(localRegister1, 0, SP);
        sw(node.symbolTableEntry->offset, FP, localRegister1);
        freeRegister(localRegister1);

        /* free return value space */
        exec("% free return value space\n");
        addi(SP, SP, sizeofType(funcEntry->type, funcEntry->link));



    }

    void visit(ReturnStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
        exec("% return statement\n");

        std::string localRegister1 = getRegister();
        exec("% load return value\n");
        lw(localRegister1, node.children[0]->symbolTableEntry->offset, FP); // correct return value
        exec("% store return value at offset 0\n");
        sw(0, FP, localRegister1);
        freeRegister(localRegister1);
    }


    void visit(ReadStatNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }






    void visit(StructDeclNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FParamNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FloatlitNode &node) override {
        for (auto child : node.children) {
            child->accept(*this);
        }
    }

    void visit(FuncDeclNode &node) override {
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

    void visit(VarDeclOrStatBlockNode &node) override {
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

private:
    const std::string ZR = "r0"; // zero register
    const std::string FP  = "r12"; // frame pointer
    const std::string RV = "r13"; // return value
    const std::string SP = "r14"; // stack pointer
    const std::string JL = "r15"; // jump link

    int tagCounter = 0;

    std::stack<std::string> registerPool;

    std::string getRegister() {
        std::string reg = registerPool.top();
        registerPool.pop();
        return reg;
    }

    void freeRegister(const std::string &reg) {
        registerPool.push(reg);
    }

    std::ostream& exec() {
        return moonExecCode << indent;
    }

    std::ostream& data() {
        return moonDataCode << indent;
    }

    std::ostream& exec(const std::string &label) {
        return moonExecCode << std::left << std::setw(indentLength) << (' ' + label);
    }

    std::ostream& data(const std::string &label) {
        return moonDataCode << std::left << std::setw(indentLength) << (' ' + label);
    }

    /* $BEGIN ARITHMETIC INSTRUCTIONS */

    // add
    void add(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << "add " + dest + "," + op1 + "," + op2 + "\n";
    }
    // addi
    void addi(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << "addi " + dest + "," + op1 + "," + op2 + "\n";
    }
    // addi
    void addi(const std::string &dest, const std::string &op1, int op2) {
        addi(dest, op1, std::to_string(op2));
    }
    // sub
    void sub(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "sub " + dest + "," + op1 + "," + op2 + "\n";
    }
    // subi
    void subi(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "subi " + dest + "," + op1 + "," + op2 + "\n";
    }
    // subi
    void subi(const std::string &dest, const std::string &op1, int op2) {
        subi(dest, op1, std::to_string(op2));
    }
    // mul
    void mul(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "mul " + dest + "," + op1 + "," + op2 + "\n";
    }
    // muli
    void muli(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "muli " + dest + "," + op1 + "," + op2 + "\n";
    }
    // muli
    void muli(const std::string &dest, const std::string &op1, int op2) {
        muli(dest, op1, std::to_string(op2));
    }
    // div
    void div(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "div " + dest + "," + op1 + "," + op2 + "\n";
    }
    // and
    void andOp(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "and " + dest + "," + op1 + "," + op2 + "\n";
    }
    // or
    void orOp(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "or " + dest + "," + op1 + "," + op2 + "\n";
    }
    // not
    void notOp(const std::string &dest, const std::string &op1) {
        exec() << + "not " + dest + "," + op1 + "\n";
    }
    // less
    void clt(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "clt " + dest + "," + op1 + "," + op2 + "\n";
    }
    // less or equal
    void cle(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "cle " + dest + "," + op1 + "," + op2 + "\n";
    }
    // greater
    void cgt(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "cgt " + dest + "," + op1 + "," + op2 + "\n";
    }
    // greater or equal
    void cge(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "cge " + dest + "," + op1 + "," + op2 + "\n";
    }
    // equal
    void ceq(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "ceq " + dest + "," + op1 + "," + op2 + "\n";
    }
    // not equal
    void cne(const std::string &dest, const std::string &op1, const std::string &op2) {
        exec() << + "cne " + dest + "," + op1 + "," + op2 + "\n";
    }

    /* $END ARITHMETIC INSTRUCTIONS */

    /* $BEGIN DATA ACCESS INSTRUCTIONS */

    // load word
    void lw(const std::string &dest, const int &offset, const std::string &base) {
        exec() << + "lw " + dest + "," << offset << "(" + base + ")\n";
    }
    // load byte
    void lb(const std::string &dest, const std::string &offset, const std::string &base) {
        exec() << + "lb " + dest + "," << offset << "(" + base + ")\n";
    }
    // store word
    void sw(const std::string &offset, const std::string &base, const std::string &src) {
        exec() << + "sw " << offset << "(" + base + ")," + src + "\n";
    }
    // store word
    void sw(const int &offset, const std::string &base, const std::string &src) {
        sw(std::to_string(offset), base, src);
    }


    /* $END DATA ACCESS INSTRUCTIONS */

    /* $BEGIN CONTROL INSTRUCTIONS */

    // branch if zero
    void bz(const std::string &dest, const std::string &base) {
        exec() << + "bz " + dest + "," + base + "\n";
    }
    // branch if non-zero
    void bnz(const std::string &dest, const std::string &base) {
        exec() << + "bnz " + dest + "," + base + "\n";
    }
    // jump
    void j(const std::string &dest) {
        exec() << + "j " + dest + "\n";
    }
    // jump and link
    void jl(const std::string &store, const std::string &dest) {
        exec() << + "jl " + store + "," + dest + "\n";
    }
    // jump register
    void jr(const std::string &dest) {
        exec() << + "jr " + dest + "\n";
    }

    /* $END CONTROL INSTRUCTIONS */

    /* $BEGIN I/O INSTRUCTIONS */

    // putc
    void putc(const std::string &dest) {
        exec() << + "putc " + dest + "\n";
    }

    /* $END I/O INSTRUCTIONS */

    void push(const std::string &reg, int offset) {
        addi(SP, SP, offset);
        sw(0, SP, reg);
    }


    void printNewLine(const std::string &localRegister1, const std::string &localRegister2) {
        exec() << "% print newline\n";
        addi(localRegister1, ZR, 0);
        lb(localRegister2, "cr", localRegister1);
        putc(localRegister2);
        addi(localRegister1, localRegister1, 1);
        lb(localRegister2, "cr", localRegister1);
        putc(localRegister2);
    }

    std::string getTag() {
        return "tag" + std::to_string(tagCounter++);
    }



};

#endif //COMPILER_CODEGEN_HPP
