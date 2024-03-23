#ifndef COMPILER_AST_HPP
#define COMPILER_AST_HPP

#include <string>
#include <vector>

class ASTNodeVisitor;
class SymbolTable;
class SymbolTableEntry;
class SymbolTableGenerator;
class SemanticAnalyzer;

class StructEntry;
class FuncEntry;
class VarEntry;

class ProgNode;
class StructDeclNode;
class FuncDefNode;
class ImplDefNode;
class InheritListNode;
class AddOpNode;
class AParamsListNode;
class ArraySizeListNode;
class AssignOpNode;
class VarDeclOrStatBlockNode;
class StatBlockNode;
class DotNode;
class IntlitNode;
class FloatlitNode;
class NotNode;
class SignNode;
class FunctionCallNode;
class VariableNode;
class FuncDeclNode;
class FParamNode;
class FParamListNode;
class IdNode;
class IndiceListNode;
class ImplFuncListNode;
class MultOpNode;
class MemberNode;
class RelOpNode;
class RelExprNode;
class MemberListNode;
class IfStatNode;
class WhileStatNode;
class ReadStatNode;
class WriteStatNode;
class ReturnStatNode;
class AssignStatNode;
class TypeNode;
class VisibilityNode;
class VarDeclNode;

/* abstract syntax tree generation */
enum ASTNodeType {
    Epsilon,
    Prog,
    StructDecl,
    FuncDef,
    ImplDef,
    InheritList,
    AddOp,
    AParamsList,
    ArraySizeList,
    AssignOp,
    VarDeclOrStatBlock,
    StatBlock,
    Dot,
    Intlit,
    Floatlit,
    Not,
    Sign,
    FunctionCall,
    Variable,
    FuncDecl,
    FParam,
    FParamList,
    Id,
    IndiceList,
    ImplFuncList,
    MultOp,
    Member,
    RelOp,
    RelExpr,
    MemberList,
    IfStat,
    WhileStat,
    ReadStat,
    WriteStat,
    ReturnStat,
    AssignStat,
    Type,
    Visibility,
    VarDecl
};


class ASTNode {
public:
    ASTNodeType type;
    std::string value;
    std::vector<ASTNode*> children;

    explicit ASTNode(ASTNodeType type, std::string value) : type(type), value(std::move(value)) {}

    virtual ~ASTNode() {
        for (auto child : children) {
            delete child;
        }
    }

    virtual void accept(ASTNodeVisitor &visitor) = 0;
};


class ASTNodeVisitor {
public:
    virtual void visit(ProgNode& node) = 0;
    virtual void visit(StructDeclNode& node) = 0;
    virtual void visit(FuncDefNode& node) = 0;
    virtual void visit(ImplDefNode& node) = 0;
    virtual void visit(InheritListNode& node) = 0;
    virtual void visit(AddOpNode& node) = 0;
    virtual void visit(AParamsListNode& node) = 0;
    virtual void visit(ArraySizeListNode& node) = 0;
    virtual void visit(AssignOpNode& node) = 0;
    virtual void visit(VarDeclOrStatBlockNode& node) = 0;
    virtual void visit(StatBlockNode& node) = 0;
    virtual void visit(DotNode& node) = 0;
    virtual void visit(IntlitNode& node) = 0;
    virtual void visit(FloatlitNode& node) = 0;
    virtual void visit(NotNode& node) = 0;
    virtual void visit(SignNode& node) = 0;
    virtual void visit(FunctionCallNode& node) = 0;
    virtual void visit(VariableNode& node) = 0;
    virtual void visit(FuncDeclNode& node) = 0;
    virtual void visit(FParamNode& node) = 0;
    virtual void visit(FParamListNode& node) = 0;
    virtual void visit(IdNode& node) = 0;
    virtual void visit(IndiceListNode& node) = 0;
    virtual void visit(ImplFuncListNode& node) = 0;
    virtual void visit(MultOpNode& node) = 0;
    virtual void visit(MemberNode& node) = 0;
    virtual void visit(RelOpNode& node) = 0;
    virtual void visit(RelExprNode& node) = 0;
    virtual void visit(MemberListNode& node) = 0;
    virtual void visit(IfStatNode& node) = 0;
    virtual void visit(WhileStatNode& node) = 0;
    virtual void visit(ReadStatNode& node) = 0;
    virtual void visit(WriteStatNode& node) = 0;
    virtual void visit(ReturnStatNode& node) = 0;
    virtual void visit(AssignStatNode& node) = 0;
    virtual void visit(TypeNode& node) = 0;
    virtual void visit(VisibilityNode& node) = 0;
    virtual void visit(VarDeclNode& node) = 0;

    virtual ~ASTNodeVisitor() = default;
};


class EpsilonNode : public ASTNode {
public:
    EpsilonNode() : ASTNode(Epsilon, "") {}
};

class StructDeclNode : public ASTNode {
public:
    explicit StructDeclNode() : ASTNode(StructDecl, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FuncDefNode : public ASTNode {
public:
    explicit FuncDefNode() : ASTNode(FuncDef, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ImplDefNode : public ASTNode {
public:
    explicit ImplDefNode() : ASTNode(ImplDef, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class AddOpNode : public ASTNode {
public:
    explicit AddOpNode(const std::string& op) : ASTNode(AddOp, op) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class AParamsListNode : public ASTNode {
public:
    AParamsListNode() : ASTNode(AParamsList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ArraySizeListNode : public ASTNode {
public:
    ArraySizeListNode() : ASTNode(ArraySizeList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class AssignOpNode : public ASTNode {
public:
    explicit AssignOpNode(const std::string& op) : ASTNode(AssignOp, op) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class VarDeclOrStatBlockNode : public ASTNode {
public:
    VarDeclOrStatBlockNode() : ASTNode(VarDeclOrStatBlock, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class StatBlockNode : public ASTNode {
public:
    StatBlockNode() : ASTNode(StatBlock, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class DotNode : public ASTNode {
public:
    DotNode() : ASTNode(Dot, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class IntlitNode : public ASTNode {
public:
    explicit IntlitNode(const std::string& value) : ASTNode(Intlit, value) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FloatlitNode : public ASTNode {
public:
    explicit FloatlitNode(const std::string& value) : ASTNode(Floatlit, value) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class NotNode : public ASTNode {
public:
    explicit NotNode(const std::string& value) : ASTNode(Not, value) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class SignNode : public ASTNode {
public:
    explicit SignNode(const std::string& sign) : ASTNode(Sign, sign) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode() : ASTNode(FunctionCall, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class VariableNode : public ASTNode {
public:
    VariableNode() : ASTNode(Variable, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FuncDeclNode : public ASTNode {
public:
    FuncDeclNode() : ASTNode(FuncDecl, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FParamNode : public ASTNode {
public:
    FParamNode() : ASTNode(FParam, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FParamListNode : public ASTNode {
public:
    FParamListNode() : ASTNode(FParamList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class IdNode : public ASTNode {
public:
    explicit IdNode(const std::string& id) : ASTNode(Id, id) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class IndiceListNode : public ASTNode {
public:
    IndiceListNode() : ASTNode(IndiceList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ImplFuncListNode : public ASTNode {
public:
    ImplFuncListNode() : ASTNode(ImplFuncList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class MultOpNode : public ASTNode {
public:
    explicit MultOpNode(const std::string& op) : ASTNode(MultOp, op) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class MemberNode : public ASTNode {
public:
    MemberNode() : ASTNode(Member, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class RelOpNode : public ASTNode {
public:
    explicit RelOpNode(const std::string& op) : ASTNode(RelOp, op) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class RelExprNode : public ASTNode {
public:
    RelExprNode() : ASTNode(RelExpr, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class MemberListNode : public ASTNode {
public:
    MemberListNode() : ASTNode(MemberList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class IfStatNode : public ASTNode {
public:
    IfStatNode() : ASTNode(IfStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class WhileStatNode : public ASTNode {
public:
    WhileStatNode() : ASTNode(WhileStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ReadStatNode : public ASTNode {
public:
    ReadStatNode() : ASTNode(ReadStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class WriteStatNode : public ASTNode {
public:
    WriteStatNode() : ASTNode(WriteStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ReturnStatNode : public ASTNode {
public:
    ReturnStatNode() : ASTNode(ReturnStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class AssignStatNode : public ASTNode {
public:
    AssignStatNode() : ASTNode(AssignStat, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class TypeNode : public ASTNode {
public:
    explicit TypeNode(const std::string& typeVal) : ASTNode(Type, typeVal) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class VisibilityNode : public ASTNode {
public:
    explicit VisibilityNode(const std::string& visibility) : ASTNode(Visibility, visibility) {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode() : ASTNode(VarDecl, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class InheritListNode : public ASTNode {
public:
    InheritListNode() : ASTNode(InheritList, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class ProgNode : public ASTNode {
public:
    ProgNode() : ASTNode(Prog, "") {}

    void accept(ASTNodeVisitor &visitor) override {
        visitor.visit(*this);
    }
};


class SymbolTableEntry {
public:
    std::string name;
    std::string kind;
    std::string type;
    SymbolTable *link;

    int offset;
    std::vector<int> dims;

    SymbolTableEntry(std::string name, std::string kind, std::string type, SymbolTable *link) : name(std::move(name)), kind(std::move(kind)), type(std::move(type)), link(link) {
        offset = 0;
        dims = std::vector<int>();
    }
};

class StructEntry : public SymbolTableEntry {
public:
    StructEntry(std::string name, std::string type, SymbolTable *link) : SymbolTableEntry(std::move(name), "struct", std::move(type), link) {}
};

class FuncEntry : public SymbolTableEntry {
public:
    FuncEntry(std::string name, std::string type, SymbolTable *link) : SymbolTableEntry(std::move(name), "func", std::move(type), link) {}
};

class VarEntry : public SymbolTableEntry {
public:
    VarEntry(std::string name, std::string type, SymbolTable *link) : SymbolTableEntry(std::move(name), "var", std::move(type), link) {}
};



class SymbolTable {
public:
    std::string name;
    std::vector<SymbolTableEntry*> symList;
    SymbolTable* upperScope;
    int size;
    int level;

    SymbolTable(std::string name, SymbolTable* upperScope, int level) : name(std::move(name)), upperScope(upperScope), level(level) {
        symList = std::vector<SymbolTableEntry*>();
        size = 0;
    }

    ~SymbolTable() {
        for (auto entry : symList) {
             entry = nullptr;
        }
    }

    void insert(SymbolTableEntry* entry) {
        symList.push_back(entry);
        size++;
    }

    SymbolTableEntry* lookup(const std::string& lookup) {
        for (auto entry : symList) {
            if (entry->name == lookup) {
                return entry;
            }
        }
        return nullptr;
    }
};





#endif //COMPILER_AST_HPP
