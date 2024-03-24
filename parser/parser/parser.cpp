#include <stack>
#include <string>
#include <parser.hpp>
#include <algorithm>
#include <iostream>


/* parse functions */
void skipError(Lexer& lexer, std::stack<std::string>& parseStack, Token& lookahead);
void inverseRHSMultiplePush(std::stack<std::string>& parseStack, const std::vector<std::string>& rule);
bool isTerminal(const std::string& symbol);
std::string tokenTypeToString(TokenType type);
void printStack(const std::stack<std::string>& stack, std::ofstream& outfile);

/* print AST */
std::string getASTNodeTypeToString(ASTNodeType type) {
    switch (type) {
        case Epsilon: return "Epsilon";
        case Prog: return "Prog";
        case StructDecl: return "StructDecl";
        case FuncDef: return "FuncDef";
        case ImplDef: return "ImplDef";
        case InheritList: return "InheritList";
        case AddOp: return "AddOp";
        case AParamsList: return "AParamsList";
        case ArraySizeList: return "ArraySizeList";
        case AssignOp: return "AssignOp";
        case VarDeclOrStatBlock: return "VarDeclOrStatBlock";
        case StatBlock: return "StatBlock";
        case Dot: return "Dot";
        case Intlit: return "Intlit";
        case Floatlit: return "Floatlit";
        case Not: return "Not";
        case Sign: return "Sign";
        case FunctionCall: return "FunctionCall";
        case Variable: return "Variable";
        case FuncDecl: return "FuncDecl";
        case FParam: return "FParam";
        case FParamList: return "FParamList";
        case Id: return "Id";
        case IndiceList: return "IndiceList";
        case ImplFuncList: return "ImplFuncList";
        case MultOp: return "MultOp";
        case Member: return "Member";
        case RelOp: return "RelOp";
        case RelExpr: return "RelExpr";
        case MemberList: return "MemberList";
        case IfStat: return "IfStat";
        case WhileStat: return "WhileStat";
        case ReadStat: return "ReadStat";
        case WriteStat: return "WriteStat";
        case ReturnStat: return "ReturnStat";
        case AssignStat: return "AssignStat";
        case Type: return "Type";
        case Visibility: return "Visibility";
        case VarDecl: return "VarDecl";
        default: return "Unknown";
    }
}

void printAST(std::ofstream& astfile, const ASTNode* node, int depth = 0) {
    if (!node) return;

    // Print leading spaces to indicate depth
    std::string indent(depth * 2, ' ');
    astfile << indent << getASTNodeTypeToString(node->type) << " : " << node->value << std::endl;

    // Recursively print children
    for (ASTNode* child : node->children) {
        printAST(astfile, child, depth + 1);
    }
}

// hard-coded first and follow sets for use in error handling and recovery
std::unordered_map<std::string, std::unordered_set<std::string>> FirstSets = {
        {"ADDOP", {"plus", "minus", "or"}},
        {"ARRAYSIZE2", {"intlit", "rsqbr"}},
        {"ASSIGNSTAT", {"id"}},
        {"EXPR2", {"eq", "neq", "lt", "gt", "leq", "geq"}},
        {"FACTOR2", {"lpar", "lsqbr"}},
        {"REPTVARIABLEORFUNCTIONCALL", {"dot"}},
        {"IDNEST", {"id"}},
        {"IDNEST3", {"dot"}},
        {"IDNEST2", {"lpar", "lsqbr"}},
        {"FUNCBODY", {"lcurbr"}},
        {"FUNCHEAD", {"func"}},
        {"FPARAMS", {"id"}},
        {"FUNCDECL", {"func"}},
        {"ARITHEXPR", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"RELOP", {"eq", "neq", "lt", "gt", "leq", "geq"}},
        {"APARAMSTAIL", {"comma"}},
        {"REPTAPARAMS1", {"comma"}},
        {"REPTFPARAMS3", {"lsqbr"}},
        {"FPARAMSTAIL", {"comma"}},
        {"REPTFPARAMS4", {"comma"}},
        {"REPTFPARAMSTAIL4", {"lsqbr"}},
        {"REPTFUNCBODY1", {"let", "if", "while", "read", "write", "return", "id"}},
        {"INDICE", {"lsqbr"}},
        {"REPTIMPLDEF3", {"func"}},
        {"REPTOPTSTRUCTDECL22", {"comma"}},
        {"REPTPROG0", {"struct", "impl", "func"}},
        {"MEMBERDECL", {"let", "func"}},
        {"ARRAYSIZE", {"lsqbr"}},
        {"RETURNTYPE", {"void", "integer", "float", "id"}},
        {"RIGHTRECARITHEXPR", {"plus", "minus", "or"}},
        {"MULTOP", {"mult", "div", "and"}},
        {"SIGN", {"plus", "minus"}},
        {"START", {"struct", "impl", "func"}},
        {"PROG", {"struct", "impl", "func"}},
        {"REPTSTATBLOCK1", {"if", "while", "read", "write", "return", "id"}},
        {"RELEXPR", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"STATBLOCK", {"lcurbr", "if", "while", "read", "write", "return", "id"}},
        {"STATEMENT2", {"lpar", "dot", "lsqbr", "equal"}},
        {"OPTSTRUCTDECL2", {"inherits"}},
        {"REPTSTRUCTDECL4", {"public", "private"}},
        {"STRUCTORIMPLORFUNC", {"struct", "impl", "func"}},
        {"STRUCTDECL", {"struct"}},
        {"IMPLDEF", {"impl"}},
        {"FUNCDEF", {"func"}},
        {"TERM", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"FACTOR", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"RIGHTRECTERM", {"mult", "div", "and"}},
        {"TYPE", {"integer", "float", "id"}},
        {"REPTVARDECL4", {"lsqbr"}},
        {"VARDECLORSTAT", {"let", "if", "while", "read", "write", "return", "id"}},
        {"VARDECL", {"let"}},
        {"STATEMENT", {"if", "while", "read", "write", "return", "id"}},
        {"VARIABLE", {"id"}},
        {"VARIABLE2", {"lpar", "lsqbr", "dot"}},
        {"REPTVARIABLE", {"dot"}},
        {"VARIDNEST2", {"lpar", "lsqbr"}},
        {"APARAMS", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"VARIDNEST", {"dot"}},
        {"REPTIDNEST1", {"lsqbr"}},
        {"VISIBILITY", {"public", "private"}},
        {"STATEMENT3", {"dot"}},
        {"STATEMENT4", {"dot", "equal"}},
        {"ASSIGNOP", {"equal"}},
        {"EXPR", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}}
};

std::unordered_map<std::string, std::unordered_set<std::string>> FollowSets = {
        {"ADDOP", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"ARRAYSIZE2", {"semi", "lsqbr", "rpar", "comma"}},
        {"ASSIGNSTAT", {""}},
        {"EXPR2", {"semi", "comma", "rpar"}},
        {"FACTOR2", {"semi", "mult", "div", "and", "dot", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"REPTVARIABLEORFUNCTIONCALL", {"semi", "mult", "div", "and", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"IDNEST", {""}},
        {"IDNEST3", {"semi", "mult", "div", "and", "dot", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"IDNEST2", {"semi", "mult", "div", "and", "dot", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"FUNCBODY", {"struct", "impl", "func", "rcurbr"}},
        {"FUNCHEAD", {"semi", "lcurbr"}},
        {"FPARAMS", {"rpar"}},
        {"FUNCDECL", {"rcurbr", "public", "private"}},
        {"ARITHEXPR", {"semi", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "comma", "rpar"}},
        {"RELOP", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"APARAMSTAIL", {"comma", "rpar"}},
        {"REPTAPARAMS1", {"rpar"}},
        {"REPTFPARAMS3", {"rpar", "comma"}},
        {"FPARAMSTAIL", {"comma", "rpar"}},
        {"REPTFPARAMS4", {"rpar"}},
        {"REPTFPARAMSTAIL4", {"comma", "rpar"}},
        {"REPTFUNCBODY1", {"rcurbr"}},
        {"INDICE", {"semi", "mult", "div", "and", "lsqbr", "dot", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "equal", "plus", "minus", "or", "comma", "rpar"}},
        {"REPTIMPLDEF3", {"rcurbr"}},
        {"REPTOPTSTRUCTDECL22", {"lcurbr"}},
        {"REPTPROG0", {""}},
        {"MEMBERDECL", {"rcurbr", "public", "private"}},
        {"ARRAYSIZE", {"semi", "lsqbr", "rpar", "comma"}},
        {"RETURNTYPE", {"semi", "lcurbr"}},
        {"RIGHTRECARITHEXPR", {"semi", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "comma", "rpar"}},
        {"MULTOP", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"SIGN", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"START", {""}},
        {"PROG", {""}},
        {"REPTSTATBLOCK1", {"rcurbr"}},
        {"RELEXPR", {"rpar"}},
        {"STATBLOCK", {"else", "semi"}},
        {"STATEMENT2", {"semi"}},
        {"STATEMENT3", {"semi"}},
        {"STATEMENT4", {"semi"}},
        {"ASSIGNOP", {"id", "intlit", "floatlit", "lpar", "not", "plus", "minus"}},
        {"EXPR", {"semi", "comma", "rpar"}},
        {"OPTSTRUCTDECL2", {"lcurbr"}},
        {"REPTSTRUCTDECL4", {"rcurbr"}},
        {"STRUCTORIMPLORFUNC", {"struct", "impl", "func"}},
        {"STRUCTDECL", {"struct", "impl", "func"}},
        {"IMPLDEF", {"struct", "impl", "func"}},
        {"FUNCDEF", {"struct", "impl", "func", "rcurbr"}},
        {"TERM", {"semi", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"FACTOR", {"semi", "mult", "div", "and", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"RIGHTRECTERM", {"semi", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "plus", "minus", "or", "comma", "rpar"}},
        {"TYPE", {"rpar", "lcurbr", "comma", "lsqbr", "semi"}},
        {"REPTVARDECL4", {"semi"}},
        {"VARDECLORSTAT", {"let", "if", "while", "read", "write", "return", "id", "rcurbr"}},
        {"VARDECL", {"public", "private", "let", "if", "while", "read", "write", "return", "id", "rcurbr"}},
        {"STATEMENT", {"else", "semi", "let", "if", "while", "read", "write", "return", "id", "rcurbr"}},
        {"VARIABLE", {"equal", "rpar"}},
        {"VARIABLE2", {"equal", "rpar"}},
        {"REPTVARIABLE", {"equal", "rpar"}},
        {"VARIDNEST2", {"dot", "semi", "equal", "rpar"}},
        {"APARAMS", {"rpar"}},
        {"VARIDNEST", {"dot", "semi", "equal", "rpar"}},
        {"REPTIDNEST1", {"semi", "mult", "div", "and", "dot", "rsqbr", "eq", "neq", "lt", "gt", "leq", "geq", "equal", "plus", "minus", "or", "comma", "rpar"}},
        {"VISIBILITY", {"let", "func"}}
};


/* switch on semantic action, call the appropriate function, return true if it was a semantic action */
bool callSemanticAction(std::stack<ASTNode*>& semanticStack, const std::string& action, Token &a) {
    if (action == "AA") {
        semanticStack.push(new EpsilonNode());
        return true;
    } else if (action == "A1") {
        if (a->type == TokenTypePlus || a->type == TokenTypeMinus || a->type == TokenTypeOr) {
            semanticStack.push(new AddOpNode(a->value));
            return true;
        } else {
            return false;
        }
    } else if (action == "A2") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode* aparamslist = new AParamsListNode();
        for (auto child : children) {
            aparamslist->children.push_back(child);
        }
        semanticStack.push(aparamslist);
        return true;
    } else if (action == "A3") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *arraysizelist = new ArraySizeListNode();
        for (auto child : children) {
            arraysizelist->children.push_back(child);
        }
        semanticStack.push(arraysizelist);
        return true;
    } else if (action == "A4") {
        ASTNode *term1 = semanticStack.top();
        semanticStack.pop();
        ASTNode *addop = semanticStack.top();
        semanticStack.pop();
        ASTNode *term2 = semanticStack.top();
        semanticStack.pop();

        addop->children.push_back(term2);
        addop->children.push_back(term1);

        semanticStack.push(addop);
        return true;
    } else if (action == "A5") {
        if (a->type == TokenTypeAssign) {
            semanticStack.push(new AssignOpNode(a->value));
            return true;
        } else {
            return false;
        }
    } else if (action == "B1") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *vardecorstatblock = new VarDeclOrStatBlockNode();
        for (auto child : children) {
            vardecorstatblock->children.push_back(child);
        }
        semanticStack.push(vardecorstatblock);
        return true;
    } else if (action == "B2") {
        ASTNode *statBlock = new StatBlockNode();
        semanticStack.push(statBlock);
        return true;
    } else if (action == "B3") {
        ASTNode *statblock = semanticStack.top();
        semanticStack.pop();
        ASTNode *statement = semanticStack.top();
        semanticStack.pop();

        statblock->children.push_back(statement);
        semanticStack.push(statblock);

        return true;
    } else if (action == "B4") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *statblock = new StatBlockNode();
        for (auto child : children) {
            statblock->children.push_back(child);
        }
        semanticStack.push(statblock);
        return true;
    } else if (action == "D1") {
        ASTNode *dot = new DotNode();
        semanticStack.push(dot);
        return true;
    } else if (action == "D2") {
        ASTNode *dotParam2 = semanticStack.top();
        semanticStack.pop();
        ASTNode *dot = semanticStack.top();
        semanticStack.pop();
        ASTNode *dotParam1 = semanticStack.top();
        semanticStack.pop();

        dot->children.push_back(dotParam1);
        dot->children.push_back(dotParam2);

        semanticStack.push(dot);
        return true;
    } else if (action == "F1") {
        ASTNode *intlit = new IntlitNode(a->value);
        semanticStack.push(intlit);
        return true;
    } else if (action == "F2") {
        ASTNode *floatlit = new FloatlitNode(a->value);
        semanticStack.push(floatlit);
        return true;
    } else if (action == "F3") {
        ASTNode *not_ = new NotNode("!");
        semanticStack.push(not_);
        return true;
    } else if (action == "F4") {
        ASTNode *factor = semanticStack.top();
        semanticStack.pop();
        ASTNode *not_ = semanticStack.top();
        semanticStack.pop();

        not_->children.push_back(factor);
        semanticStack.push(not_);
        return true;
    } else if (action == "F5") {
        ASTNode *sign = semanticStack.top();
        semanticStack.pop();
        ASTNode *factor = semanticStack.top();
        semanticStack.pop();

        sign->children.push_back(factor);
        semanticStack.push(sign);
        return true;
    } else if (action == "F7") {
        ASTNode *functionCall = new FunctionCallNode();
        ASTNode *aparams = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        functionCall->children.push_back(id);
        functionCall->children.push_back(aparams);
        semanticStack.push(functionCall);
        return true;
    } else if (action == "F8") {
        ASTNode *variable = new VariableNode();
        ASTNode *indiceList = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        variable->children.push_back(id);
        variable->children.push_back(indiceList);

        semanticStack.push(variable);
        return true;
    } else if (action == "F10") {
        ASTNode *funcdecl = new FuncDeclNode();
        ASTNode *rettype = semanticStack.top();
        semanticStack.pop();
        ASTNode *fparamlist = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        funcdecl->children.push_back(id);
        funcdecl->children.push_back(fparamlist);
        funcdecl->children.push_back(rettype);

        semanticStack.push(funcdecl);
        return true;
    } else if (action == "F11") {
        ASTNode *fparam = new FParamNode();
        ASTNode *arraysizelist = semanticStack.top();
        semanticStack.pop();
        ASTNode *type = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        fparam->children.push_back(id);
        fparam->children.push_back(type);
        fparam->children.push_back(arraysizelist);

        semanticStack.push(fparam);
        return true;
    } else if (action == "F12") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *fparamlist = new FParamListNode();
        for (auto child : children) {
            fparamlist->children.push_back(child);
        }

        semanticStack.push(fparamlist);
        return true;
    } else if (action == "F13") {
        ASTNode *funcdef = new FuncDefNode();
        ASTNode *vardeclorstatblock = semanticStack.top();
        semanticStack.pop();
        ASTNode *rettype = semanticStack.top();
        semanticStack.pop();
        ASTNode *fparamlist = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        funcdef->children.push_back(id);
        funcdef->children.push_back(fparamlist);
        funcdef->children.push_back(rettype);
        funcdef->children.push_back(vardeclorstatblock);

        semanticStack.push(funcdef);
        return true;
    } else if (action == "I1") {
        ASTNode *id = new IdNode(a->value);
        semanticStack.push(id);
        return true;
    } else if (action == "I2") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *indiceList = new IndiceListNode();
        for (auto child : children) {
            indiceList->children.push_back(child);
        }

        semanticStack.push(indiceList);
        return true;
    } else if (action == "I3") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *implFuncList = new ImplFuncListNode();
        for (auto child : children) {
            implFuncList->children.push_back(child);
        }

        semanticStack.push(implFuncList);
        return true;
    } else if (action == "M1") {
        ASTNode *multop = new MultOpNode(a->value);
        semanticStack.push(multop);
        return true;
    } else if (action == "M2") {
        ASTNode *member = new MemberNode();
        ASTNode *memberdecl = semanticStack.top();
        semanticStack.pop();
        ASTNode *visibility = semanticStack.top();
        semanticStack.pop();

        member->children.push_back(visibility);
        member->children.push_back(memberdecl);

        semanticStack.push(member);
        return true;
    } else if (action == "P1") {
        ASTNode *structdecl = new StructDeclNode();
        ASTNode *memberlist = semanticStack.top();
        semanticStack.pop();
        ASTNode *inheritlist = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        structdecl->children.push_back(id);
        structdecl->children.push_back(inheritlist);
        structdecl->children.push_back(memberlist);

        semanticStack.push(structdecl);
        return true;
    } else if (action == "P2") {
        ASTNode *impldef = new ImplDefNode();
        ASTNode *implfuncList = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        impldef->children.push_back(id);
        impldef->children.push_back(implfuncList);

        semanticStack.push(impldef);
        return true;
    } else if (action == "R1") {
        ASTNode *relop = new RelOpNode(a->value);
        semanticStack.push(relop);
        return true;
    } else if (action == "R2") {
        ASTNode *factor2 = semanticStack.top();
        semanticStack.pop();
        ASTNode *multop = semanticStack.top();
        semanticStack.pop();
        ASTNode *factor1 = semanticStack.top();
        semanticStack.pop();

        multop->children.push_back(factor1);
        multop->children.push_back(factor2);

        semanticStack.push(multop);
        return true;
    } else if (action == "R3") {
        ASTNode *relexpr = new RelExprNode();
        ASTNode *arithExpr2 = semanticStack.top();
        semanticStack.pop();
        ASTNode *relop = semanticStack.top();
        semanticStack.pop();
        ASTNode *arithExpr1 = semanticStack.top();
        semanticStack.pop();

        relexpr->children.push_back(arithExpr1);
        relexpr->children.push_back(relop);
        relexpr->children.push_back(arithExpr2);

        semanticStack.push(relexpr);
        return true;
    } else if (action == "S1") {
        ASTNode *sign = new SignNode(a->value);
        semanticStack.push(sign);
        return true;
    } else if (action == "S2") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *inheritlist = new InheritListNode();
        for (auto child : children) {
            inheritlist->children.push_back(child);
        }

        semanticStack.push(inheritlist);
        return true;
    } else if (action == "S3") {
        std::vector<ASTNode*> children;
        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *memberlist = new MemberListNode();
        for (auto child : children) {
            memberlist->children.push_back(child);
        }

        semanticStack.push(memberlist);
        return true;
    } else if (action == "S10") {
        ASTNode *ifStat = new IfStatNode();
        ASTNode *statblock2 = semanticStack.top();
        semanticStack.pop();
        ASTNode *statblock1 = semanticStack.top();
        semanticStack.pop();
        ASTNode *relexpr = semanticStack.top();
        semanticStack.pop();

        ifStat->children.push_back(relexpr);
        ifStat->children.push_back(statblock1);
        ifStat->children.push_back(statblock2);

        semanticStack.push(ifStat);
        return true;
    } else if (action == "S11") {
        ASTNode *whileStat = new WhileStatNode();
        ASTNode *statblock = semanticStack.top();
        semanticStack.pop();
        ASTNode *relexpr = semanticStack.top();
        semanticStack.pop();

        whileStat->children.push_back(relexpr);
        whileStat->children.push_back(statblock);

        semanticStack.push(whileStat);
        return true;
    } else if (action == "S12") {
        ASTNode *readStat = new ReadStatNode();
        ASTNode *variable = semanticStack.top();
        semanticStack.pop();

        readStat->children.push_back(variable);

        semanticStack.push(readStat);
        return true;
    } else if (action == "S13") {
        ASTNode *writeStat = new WriteStatNode();
        ASTNode *expression = semanticStack.top();
        semanticStack.pop();

        writeStat->children.push_back(expression);

        semanticStack.push(writeStat);
        return true;
    } else if (action == "S14") {
        ASTNode *returnStat = new ReturnStatNode();
        ASTNode *expression = semanticStack.top();
        semanticStack.pop();

        returnStat->children.push_back(expression);

        semanticStack.push(returnStat);
        return true;
    } else if (action == "S15") {
        ASTNode *assignStat = new AssignStatNode();
        ASTNode *expression = semanticStack.top();
        semanticStack.pop();
        ASTNode *assignop = semanticStack.top();
        semanticStack.pop();
        ASTNode *variable = semanticStack.top();
        semanticStack.pop();

        assignStat->children.push_back(variable);
        assignStat->children.push_back(assignop);
        assignStat->children.push_back(expression);

        semanticStack.push(assignStat);
        return true;
    } else if (action == "T1") {
        ASTNode *type = new TypeNode(a->value);
        semanticStack.push(type);
        return true;
    } else if (action == "V1") {
        ASTNode *visibility = new VisibilityNode(a->value);
        semanticStack.push(visibility);
        return true;
    } else if (action == "V2") {
        ASTNode *vardecl = new VarDeclNode();
        ASTNode *arraysizelist = semanticStack.top();
        semanticStack.pop();
        ASTNode *type = semanticStack.top();
        semanticStack.pop();
        ASTNode *id = semanticStack.top();
        semanticStack.pop();

        vardecl->children.push_back(id);
        vardecl->children.push_back(type);
        vardecl->children.push_back(arraysizelist);

        semanticStack.push(vardecl);
        return true;
    } else if (action == "ZZ") {
        std::vector<ASTNode*> children;

        while (semanticStack.top()->type != Epsilon) {
            children.push_back(semanticStack.top());
            semanticStack.pop();
        }

        semanticStack.pop();
        std::reverse(children.begin(), children.end());

        ASTNode *prog = new ProgNode();
        for (auto child : children) {
            prog->children.push_back(child);
        }

        semanticStack.push(prog);
        return true;
    }

    return false;
}

void parseCSVIntoTT(const std::string& filePath, std::map<TableKey, ProductionRule>& TT) {
    std::ifstream file(filePath);
    std::string line;
    std::vector<std::string> terminals;

    // first line is terminals
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string terminal;
        std::getline(ss, terminal, ',');
        while (std::getline(ss, terminal, ',')) {
            terminals.push_back(terminal);
        }
    }

    // Read the rest of the file
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string nonTerminal, cell;
        std::getline(ss, nonTerminal, ',');

        int terminalIndex = 0;
        while (std::getline(ss, cell, ',')) {
            if (!cell.empty() && cell != " " && cell != "\xC2\xA0") {
                TableKey key = {nonTerminal, terminals[terminalIndex]};
                ProductionRule production;

                std::string symbol;
                std::istringstream cellStream(cell);
                while (cellStream >> symbol) {
                    if (symbol != "&epsilon") {
                        production.push_back(symbol);
                    }
                }

                TT[key] = production;
            }
            terminalIndex++;
        }
    }

    // add EOF to REPTPROG0 and ZZ
    TableKey key = {"REPTPROG0", "EOF"};
    ProductionRule production;
    production.emplace_back("REBTPROG0");
    production.emplace_back("\xC2\xA0");
    production.emplace_back("EOF");
    TT[key] = production;

    TableKey key2 = {"REPTPROG0", "ZZ"};
    ProductionRule production2;
    production.emplace_back("ZZ");
    production.emplace_back("\xC2\xA0");
    production.emplace_back("EOF");
    TT[key2] = production2;
}

ASTNode *parse(Lexer lexer, std::map<TableKey, ProductionRule> &TT, std::ofstream &outfile, std::ofstream &errorfile, std::ofstream &astfile) {
    bool accepted = true;
    std::stack<std::string> parseStack;
    std::stack<ASTNode*> semanticStack;

    parseStack.emplace("$");
    parseStack.emplace("START");
    Token a = getNextToken(lexer);
    Token prev = a;

    while (parseStack.top() != "$") {
        printStack(parseStack, outfile);
        std::string x = parseStack.top();

        if (callSemanticAction(semanticStack, x, prev)) {
            parseStack.pop();
            continue;
        }
        if (isTerminal(x)) {
            if (x == tokenTypeToString(a->type)) {
                parseStack.pop();
                prev = a;
                a = getNextToken(lexer);
            } else {

                errorfile << "ERROR - stack symbol " << x << "  has unexpected token: " << tokenTypeToString(a->type) << " " << a->value << " " << a->line << std::endl;

                skipError(lexer, parseStack, a);
                accepted = false;
            }
        } else {
            if (a->type == TokenTypeEOF) {
                if (TT.find({x, "EOF"}) != TT.end()) { // REPTPROG0 and ZZ
                    parseStack.pop();
                    continue;
                }
                return nullptr; // unexpected EOF
            }

            TableKey key = {x, tokenTypeToString(a->type)};

            if (TT.find(key) != TT.end()) {
                parseStack.pop();
                inverseRHSMultiplePush(parseStack, TT[key]);

            } else {
                errorfile << "ERROR - stack symbol " << x << "  has unexpected token: " << tokenTypeToString(a->type) << " " << a->value << " " << a->line << std::endl;

                skipError(lexer, parseStack, a);
                accepted = false;
            }

        }
    }

    printStack(parseStack, outfile);

    // print AST
    std::stack<ASTNode*> tempStack = semanticStack;
    printAST(astfile, tempStack.top(), 0);

    if (accepted) {
        return semanticStack.top();
    } else {
        return nullptr;
    }
}

void inverseRHSMultiplePush(std::stack<std::string>& parseStack, const std::vector<std::string>& rule) {
    for (auto it = rule.rbegin(); it != rule.rend()-2; ++it) { // should skip the first element and the arrow
        if (*it != "&epsilon" && *it != "EOF") {
            parseStack.push(*it);
        }
    }
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenTypeId: return "id";
        case TokenTypeInt: return "intlit";
        case TokenTypeFloat: return "floatlit";
        case TokenTypeEquals: return "eq";
        case TokenTypeNotEquals: return "neq";
        case TokenTypeLessThan: return "lt";
        case TokenTypeGreaterThan: return "gt";
        case TokenTypeLessThanOrEquals: return "leq";
        case TokenTypeGreaterThanOrEquals: return "geq";
        case TokenTypePlus: return "plus";
        case TokenTypeMinus: return "minus";
        case TokenTypeMultiply: return "mult";
        case TokenTypeDivide: return "div";
        case TokenTypeAssign: return "equal";
        case TokenTypeOr: return "or";
        case TokenTypeAnd: return "and";
        case TokenTypeNot: return "not";
        case TokenTypeLeftParenthesis: return "lpar";
        case TokenTypeRightParenthesis: return "rpar";
        case TokenTypeLeftBrace: return "lcurbr";
        case TokenTypeRightBrace: return "rcurbr";
        case TokenTypeLeftBracket: return "lsqbr";
        case TokenTypeRightBracket: return "rsqbr";
        case TokenTypeSemicolon: return "semi";
        case TokenTypeComma: return "comma";
        case TokenTypePeriod: return "dot";
        case TokenTypeColon: return "colon";
        case TokenTypeArrow: return "arrow";
        case TokenTypeIf: return "if";
        case TokenTypeThen: return "then";
        case TokenTypeElse: return "else";
        case TokenTypeVoid: return "void";
        case TokenTypePublic: return "public";
        case TokenTypePrivate: return "private";
        case TokenTypeFunc: return "func";
        case TokenTypeVar: return "var";
        case TokenTypeStruct: return "struct";
        case TokenTypeWhile: return "while";
        case TokenTypeRead: return "read";
        case TokenTypeWrite: return "write";
        case TokenTypeReturn: return "return";
        case TokenTypeSelf: return "self";
        case TokenTypeInherits: return "inherits";
        case TokenTypeLet: return "let";
        case TokenTypeImplements: return "impl";
        case TokenTypeIllegal: return "illegal";
        case TokenTypeInvalidId: return "invalidid";
        case TokenTypeInvalidInt: return "invalidint";
        case TokenTypeInvalidFloat: return "invalidfloat";
        case TokenTypeInvalidChar: return "invalidchar";
        case TokenTypeInlineComment: return "inlinecomment";
        case TokenTypeBlockComment: return "blockcomment";
        case TokenTypeDollarSign: return "$";
        case TokenTypeIntType: return "integer";
        case TokenTypeFloatType: return "float";
        case TokenTypeEOF: return "EOF";
        default: return "unknown";
    }
}

void skipError(Lexer& lexer, std::stack<std::string>& parseStack, Token& lookahead) {
    std::string x = parseStack.top();
    if (FollowSets[x].find(tokenTypeToString(lookahead->type)) != FollowSets[x].end()) {
        parseStack.pop();
    } else {
        while (FirstSets[x].find(tokenTypeToString(lookahead->type)) == FirstSets[x].end()
                && FollowSets[x].find(tokenTypeToString(lookahead->type)) == FollowSets[x].end()) {
            if (x == "semi" || x == "$" || lookahead->type == TokenTypeEOF) {
                parseStack.pop();
                return;
            }

            lookahead = getNextToken(lexer);
        }
    }
}

bool isTerminal(const std::string& symbol) {
    return symbol == "&epsilon" || symbol == "id" || symbol == "intlit" || symbol == "floatlit" || symbol == "integer" || symbol == "float" || symbol == "eq" || symbol == "neq" || symbol == "lt" || symbol == "gt" || symbol == "leq" || symbol == "geq" || symbol == "plus" || symbol == "minus" || symbol == "mult" || symbol == "div" || symbol == "equal" || symbol == "or" || symbol == "and" || symbol == "not" || symbol == "lpar" || symbol == "rpar" || symbol == "lcurbr" || symbol == "rcurbr" || symbol == "lsqbr" || symbol == "rsqbr" || symbol == "semi" || symbol == "comma" || symbol == "dot" || symbol == "colon" || symbol == "arrow" || symbol == "if" || symbol == "then" || symbol == "else" || symbol == "void" || symbol == "public" || symbol == "private" || symbol == "func" || symbol == "var" || symbol == "struct" || symbol == "while" || symbol == "read" || symbol == "write" || symbol == "return" || symbol == "self" || symbol == "inherits" || symbol == "let" || symbol == "impl";
}

void printStack(const std::stack<std::string>& stack, std::ofstream& outfile) {
    std::stack<std::string> tempStack = stack; // Copy because original stack is LIFO
    std::vector<std::string> elements;
    while (!tempStack.empty()) {
        elements.push_back(tempStack.top());
        tempStack.pop();
    }
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        outfile << *it << " ";
    }
    outfile << std::endl;
}
