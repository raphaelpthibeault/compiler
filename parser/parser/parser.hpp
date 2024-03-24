#ifndef COMPILER_PARSER_HPP
#define COMPILER_PARSER_HPP

#include <lexer.h>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <sstream>
#include <ast.hpp>

/* parsing */
using ProductionRule = std::vector<std::string>;
using TableKey = std::pair<std::string, std::string>;

ASTNode *parse(Lexer lexer, std::map<TableKey, ProductionRule>& TT, std::ofstream& outfile, std::ofstream& errorfile, std::ofstream& astfile);
void parseCSVIntoTT(const std::string& filePath, std::map<TableKey, ProductionRule>& TT);



#endif //COMPILER_PARSER_HPP