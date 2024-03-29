#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <lexer.h>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>

/* parsing */
using ProductionRule = std::vector<std::string>;
using TableKey = std::pair<std::string, std::string>;

bool parse(Lexer lexer, std::map<TableKey, ProductionRule>& TT, std::ofstream& outfile, std::ofstream& errorfile, std::ofstream& astfile);
void parseCSVIntoTT(const std::string& filePath, std::map<TableKey, ProductionRule>& TT);

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
};

class EpsilonNode : public ASTNode {
public:
    EpsilonNode() : ASTNode(Epsilon, "") {}
};

class StructDeclNode : public ASTNode {
public:
    explicit StructDeclNode() : ASTNode(StructDecl, "") {}
};

class FuncDefNode : public ASTNode {
public:
    explicit FuncDefNode() : ASTNode(FuncDef, "") {}
};

class ImplDefNode : public ASTNode {
public:
    explicit ImplDefNode() : ASTNode(ImplDef, "") {}
};

class AddOpNode : public ASTNode {
public:
    explicit AddOpNode(const std::string& op) : ASTNode(AddOp, op) {}
};

class AParamsListNode : public ASTNode {
public:
    AParamsListNode() : ASTNode(AParamsList, "") {}
};

class ArraySizeListNode : public ASTNode {
public:
    ArraySizeListNode() : ASTNode(ArraySizeList, "") {}
};

class AssignOpNode : public ASTNode {
public:
    explicit AssignOpNode(const std::string& op) : ASTNode(AssignOp, op) {}
};

class VarDeclOrStatBlockNode : public ASTNode {
public:
    VarDeclOrStatBlockNode() : ASTNode(VarDeclOrStatBlock, "") {}
};

class StatBlockNode : public ASTNode {
public:
    StatBlockNode() : ASTNode(StatBlock, "") {}
};

class DotNode : public ASTNode {
public:
    DotNode() : ASTNode(Dot, "") {}
};

class IntlitNode : public ASTNode {
public:
    explicit IntlitNode(const std::string& value) : ASTNode(Intlit, value) {}
};

class FloatlitNode : public ASTNode {
public:
    explicit FloatlitNode(const std::string& value) : ASTNode(Floatlit, value) {}
};

class NotNode : public ASTNode {
public:
    explicit NotNode(const std::string& value) : ASTNode(Not, value) {}
};

class SignNode : public ASTNode {
public:
    explicit SignNode(const std::string& sign) : ASTNode(Sign, sign) {}
};

class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode() : ASTNode(FunctionCall, "") {}
};

class VariableNode : public ASTNode {
public:
    VariableNode() : ASTNode(Variable, "") {}
};

class FuncDeclNode : public ASTNode {
public:
    FuncDeclNode() : ASTNode(FuncDecl, "") {}
};

class FParamNode : public ASTNode {
public:
    FParamNode() : ASTNode(FParam, "") {}
};

class FParamListNode : public ASTNode {
public:
    FParamListNode() : ASTNode(FParamList, "") {}
};

class IdNode : public ASTNode {
public:
    explicit IdNode(const std::string& id) : ASTNode(Id, id) {}
};

class IndiceListNode : public ASTNode {
public:
    IndiceListNode() : ASTNode(IndiceList, "") {}
};

class ImplFuncListNode : public ASTNode {
public:
    ImplFuncListNode() : ASTNode(ImplFuncList, "") {}
};

class MultOpNode : public ASTNode {
public:
    explicit MultOpNode(const std::string& op) : ASTNode(MultOp, op) {}
};

class MemberNode : public ASTNode {
public:
    MemberNode() : ASTNode(Member, "") {}
};

class RelOpNode : public ASTNode {
public:
    explicit RelOpNode(const std::string& op) : ASTNode(RelOp, op) {}
};

class RelExprNode : public ASTNode {
public:
    RelExprNode() : ASTNode(RelExpr, "") {}
};

class MemberListNode : public ASTNode {
public:
    MemberListNode() : ASTNode(MemberList, "") {}
};

class IfStatNode : public ASTNode {
public:
    IfStatNode() : ASTNode(IfStat, "") {}
};

class WhileStatNode : public ASTNode {
public:
    WhileStatNode() : ASTNode(WhileStat, "") {}
};

class ReadStatNode : public ASTNode {
public:
    ReadStatNode() : ASTNode(ReadStat, "") {}
};

class WriteStatNode : public ASTNode {
public:
    WriteStatNode() : ASTNode(WriteStat, "") {}
};

class ReturnStatNode : public ASTNode {
public:
    ReturnStatNode() : ASTNode(ReturnStat, "") {}
};

class AssignStatNode : public ASTNode {
public:
    AssignStatNode() : ASTNode(AssignStat, "") {}
};

class TypeNode : public ASTNode {
public:
    explicit TypeNode(const std::string& typeVal) : ASTNode(Type, typeVal) {}
};

class VisibilityNode : public ASTNode {
public:
    explicit VisibilityNode(const std::string& visibility) : ASTNode(Visibility, visibility) {}
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode() : ASTNode(VarDecl, "") {}
};

class InheritListNode : public ASTNode {
public:
    InheritListNode() : ASTNode(InheritList, "") {}
};

class ProgNode : public ASTNode {
public:
    ProgNode() : ASTNode(Prog, "") {}
};


/* factories for creating AST nodes */
/*
class ASTNodeFactory {
public:
    static ASTNode* createASTNode(ASTNodeType type, const std::vector<ASTNode*>& children, Token a) {
        switch (type) {
            case Epsilon:
                return new EpsilonNode();
            case Prog:
                return ProgNodeFactory::createProgNode(children);
            case StructDecl:
                return StructDeclNodeFactory::createStructDeclNode(children);
            case FuncDef:
                return FuncDefNodeFactory::createFuncDefNode(children);
            case ImplDef:
                return ImplDefNodeFactory::createImplDefNode(children);
            case InheritList:
                return InheritListNodeFactory::createInheritListNode(children);
            case AddOp:
                return AddOpNodeFactory::createAddOpNode(children);
            case AParamsList:
                return AParamsListNodeFactory::createAParamsListNode(children);
            case ArraySizeList:
                return ArraySizeListNodeFactory::createArraySizeListNode(children);
            case AssignOp:
                return new AssignOpNode(a->value);
            case VarDeclOrStatBlock:
                return VarDeclOrStatBlockNodeFactory::createVarDeclOrStatBlockNode(children);
            case StatBlock:
                return StatBlockNodeFactory::createStatBlockNode(children);
            case Dot:
                return DotNodeFactory::createDotNode(children);
            case Intlit:
                return new IntlitNode(a->value);
            case Floatlit:
                return new FloatlitNode(a->value);
            case Not:
                return NotNodeFactory::createNotNode(children);
            case Sign:
                return SignNodeFactory::createSignNode(children);
            case FunctionCall:
                return FunctionCallNodeFactory::createFunctionCallNode(children);
            case Variable:
                return VariableNodeFactory::createVariableNode(children);
            case FuncDecl:
                return FuncDeclNodeFactory::createFuncDeclNode(children);
            case FParam:
                return FParamNodeFactory::createFParamNode(children);
            case FParamList:
                return FParamListNodeFactory::createFParamListNode(children);
            case Id:
                return new IdNode(a->value);
            case IndiceList:
                return IndiceListNodeFactory::createIndiceListNode(children);
            case ImpFuncList:
                return ImpFuncListNodeFactory::createImpFuncListNode(children);
            case MultOp:
                return MultOpNodeFactory::createMultOpNode(children);
            case Member:
                return MemberNodeFactory::createMemberNode(children);
            case RelOp:
                return new RelOpNode(a->value);
            case Term:
                return TermNodeFactory::createTermNode(children);
            case RelExpr:
                return RelExprNodeFactory::createRelExprNode(children);
            case MemberList:
                return MemberListNodeFactory::createMemberListNode(children);
            case IfStat:
                return IfStatNodeFactory::createIfStatNode(children);
            case WhileStat:
                return WhileStatNodeFactory::createWhileStatNode(children);
            case ReadStat:
                return ReadStatNodeFactory::createReadStatNode(children);
            case WriteStat:
                return WriteStatNodeFactory::createWriteStatNode(children);
            case ReturnStat:
                return ReturnStatNodeFactory::createReturnStatNode(children);
            case AssignStat:
                return AssignStatNodeFactory::createAssignStatNode(children);
            case FuncCallStat:
                return FuncCallStatNodeFactory::createFuncCallStatNode(children);
            case Type:
                return new TypeNode(a->value);
            case Visibility:
                return new VisibilityNode(a->value);
            case VarDecl:
                return VarDeclNodeFactory::createVarDeclNode(children);
        }
};
*/




#endif //COMPILER_PARSER_HPP
