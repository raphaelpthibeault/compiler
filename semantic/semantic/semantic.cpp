#include <semantic.hpp>
#include <iomanip>

/*
 * function to recursively print symbol tables in a nice format
 * */
static void printSymbolTable(const SymbolTable* table, int indent = 0, std::ostream& out = std::cout) {
    const int maxWidth = 81;
    std::string indentStr;
    if (indent == 0)
        indentStr = "";
    else if (indent == 5)
        indentStr = "|    ";
    else if (indent == 10)
        indentStr = "|    |    ";
    else if (indent == 15)
        indentStr = "|    |    |    ";
    else if (indent == 20)
        indentStr = "|    |    |    |    ";


    std::string innerIndentStr = indentStr + "|    ";
    std::string border = indentStr + "| " + std::string(maxWidth - 2 - indent, '=') + " |";
    std::string entryBorder = innerIndentStr + std::string(maxWidth - 6 - indent, '=') + " |";

    out << border << "\n";
    out << indentStr << "| table: " << std::left << std::setw(maxWidth - 10 - indent) << table->name << "  |";
    out << "\n" << border << "\n";

    for (const auto& entry : table->symList) {
        if (entry->kind == "struct") {
            out << innerIndentStr << std::left << std::setw(11) << entry->kind
                << std::setw(maxWidth - 19 - indent) << entry->name << "    |";
        } else {
            out << innerIndentStr << std::left << std::setw(11) << entry->kind
                << std::setw(20) << entry->name
                << "| " << std::setw(maxWidth - 40 - indent - entry->visibility.length()) << entry->type
                << (entry->visibility.empty() ? "" : "| " + entry->visibility)
                << (entry->visibility.empty() ? "   |" : " |");

        }
        out << "\n";

        if (entry->link) {
            printSymbolTable(entry->link, indent + 5, out);
        }
    }

    out << border << "\n";
}

/*
 * function to start the cycle checking
 * */
void detectCyclicStructDependency(const std::map<std::string, std::vector<std::string>> &graph, std::ostream &symerrors, bool isDependencyGraph) {
    std::map<std::string, NodeState> state;
    std::set<std::string> visited;
    std::vector<std::string> currPath;

    for (const auto &node : graph) {
        if (state[node.first] == NOT_VISITED) {
            if (hasCycle(node.first, graph, state, currPath, visited, symerrors, isDependencyGraph)) {
                break;
            }
        }
    }
}

/*
 * function to perform semantic analysis on the AST intermediate representation
 * */
void semanticAnalysis(ASTNode &root, std::ostream &symfile, std::ostream &symerrors) {
    std::cout << "Starting semantic analysis." << std::endl;
    SymbolTableCreationVisitor visitor(symerrors);
    root.accept(visitor);

    ImplToStructAddingVisitor visitor2(symerrors);
    root.accept(visitor2);

    SemanticCheckingVisitor semanticChecker(symerrors);
    root.accept(semanticChecker);

    detectCyclicStructDependency(visitor2.inheritanceGraph, symerrors, false);
    detectCyclicStructDependency(visitor2.dependencyGraph, symerrors, true);

    std::cout << "Done semantic analysis." << std::endl;

    printSymbolTable(root.symbolTable, 0, symfile);
}


