#include <iostream>
#include <stack>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "Parser.hpp"



using ProductionRule = std::vector<std::string>;
using TableKey = std::pair<std::string, std::string>;
using Set = std::unordered_set<std::string>;

void skipError(Lexer& lexer, std::stack<std::string>& parseStack, Token& lookahead);
void inverseRHSMultiplePush(std::stack<std::string>& parseStack, const std::vector<std::string>& rule);
bool isTerminal(const std::string& symbol);
std::string tokenTypeToString(TokenType type);

bool parse2(Lexer lexer, std::map<TableKey, ProductionRule>& TT, std::ofstream& outfile, std::ofstream& errorfile);


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


void parseCSVIntoTT(const std::string& filePath, std::map<TableKey, ProductionRule>& TT) {
    std::ifstream file(filePath);
    std::string line;
    std::vector<std::string> terminals;

    // Read the first line to get terminals
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string terminal;
        std::getline(ss, terminal, ','); // Skip the first empty cell
        while (std::getline(ss, terminal, ',')) {
            terminals.push_back(terminal); // Store terminals
        }
    }

    // Read the rest of the file
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string nonTerminal, cell;
        std::getline(ss, nonTerminal, ','); // Get the non-terminal

        int terminalIndex = 0;
        while (std::getline(ss, cell, ',')) {
            if (!cell.empty() && cell != " " && cell != "\xC2\xA0") { // Check for non-empty rule, handling non-breaking space
                TableKey key = {nonTerminal, terminals[terminalIndex]};
                ProductionRule production;

                // Split cell into rule symbols
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

    // add EOF to REPTPROG0
    TableKey key = {"REPTPROG0", "EOF"};
    ProductionRule production;
    // epsilon
    production.push_back("REBTPROG0");
    production.push_back("\xC2\xA0");
    production.push_back("EOF");
    TT[key] = production;
}


int main() {
    std::map<TableKey, ProductionRule> TT;
    const std::string csvFilePath = "GRAMMAR_TABLE.csv";
    std::cout << "Parsing CSV into table" << std::endl;
    parseCSVIntoTT(csvFilePath, TT);

    std::string baseName = "statement_pos";
    std::ifstream file(baseName + ".src");
    std::ofstream outfile(baseName + ".outderivation");
    std::ofstream errorfile(baseName + ".outsyntaxerrors");


    std::string input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Lexer lexer = lexerNew(input.c_str());

    std::cout << "Parsing the .src file " << std::endl;
    bool parse_accepted = parse2(lexer, TT, outfile, errorfile);
    outfile << "\nPARSE " << (parse_accepted ? "ACCEPTED" : "REJECTED") << std::endl;

    std::cout << "Done" << std::endl;
    return 0;
}


bool parse2(Lexer lexer, std::map<TableKey, ProductionRule>& TT, std::ofstream& outfile, std::ofstream& errorfile) {
    bool accepted = true;
    std::stack<std::string> parseStack;

    parseStack.emplace("$");
    parseStack.emplace("START");
    Token a = getNextToken(lexer);
    while (parseStack.top() != "$") {
        printStack(parseStack, outfile);

        std::string x = parseStack.top();

        if (isTerminal(x)) {
            if (x == tokenTypeToString(a->type)) {
                parseStack.pop();
                a = getNextToken(lexer);
            } else {

                errorfile << "ERROR - stack symbol " << x << "  has unexpected token: " << tokenTypeToString(a->type) << " " << a->value << " " << a->line << std::endl;

                skipError(lexer, parseStack, a);
                accepted = false;
            }
        } else {

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
    return accepted;

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
            lookahead = getNextToken(lexer);
        }
    }
}

bool isTerminal(const std::string& symbol) {
    return symbol == "&epsilon" || symbol == "id" || symbol == "intlit" || symbol == "floatlit" || symbol == "integer" || symbol == "float" || symbol == "eq" || symbol == "neq" || symbol == "lt" || symbol == "gt" || symbol == "leq" || symbol == "geq" || symbol == "plus" || symbol == "minus" || symbol == "mult" || symbol == "div" || symbol == "equal" || symbol == "or" || symbol == "and" || symbol == "not" || symbol == "lpar" || symbol == "rpar" || symbol == "lcurbr" || symbol == "rcurbr" || symbol == "lsqbr" || symbol == "rsqbr" || symbol == "semi" || symbol == "comma" || symbol == "dot" || symbol == "colon" || symbol == "arrow" || symbol == "if" || symbol == "then" || symbol == "else" || symbol == "void" || symbol == "public" || symbol == "private" || symbol == "func" || symbol == "var" || symbol == "struct" || symbol == "while" || symbol == "read" || symbol == "write" || symbol == "return" || symbol == "self" || symbol == "inherits" || symbol == "let" || symbol == "impl";
}

