#include <codegen.hpp>

/*
 * compute sizes and offsets and place them in the symbol tables and entries
 * */
void computeSizes(ASTNode &root) {
    ComputeMemSizeVisitor memSizeVisitor = ComputeMemSizeVisitor();
    root.accept(memSizeVisitor);
}


void generateCode(ASTNode &root, std::ostream &out) {
    std::ostringstream moonExecCode;
    std::ostringstream moonDataCode;
    CodeGenerationVisitor codeGenVisitor = CodeGenerationVisitor(moonExecCode, moonDataCode);
    root.accept(codeGenVisitor);

    out << moonExecCode.str() << moonDataCode.str() << std::endl;
}


/*
 * sizeof table
 * */
int sizeofTable(SymbolTable *table, bool isStruct) {
    /* structdecl, impl func or free func decl */
    if (table->size > 0) {
        return table->size;
    }

    if (isStruct) {
        std::vector<SymbolTableEntry*> memberVars = table->lookupAllOfKind("var");
        int structSize = 0;
        for (auto *entry : memberVars) {
            structSize += sizeofEntry(entry, table);
        }

        // recurse through inherited structs
        std::vector<std::string> inheritNames = table->lookupAllNamesOfKind("inherit");
        if (!inheritNames.empty()) {
            auto *globalScope = table;
            while (globalScope->upperScope) {
                globalScope = globalScope->upperScope;
            }

            for (const auto &name : inheritNames) {
                auto *structEntry = globalScope->lookup(name, "struct");
                structSize += sizeofTable(structEntry->link, true);
            }
        }

        //table->size = structSize;
        return structSize;
    } else  {
        // params, vars and tempvars ; i.e. all entries
        std::vector<SymbolTableEntry*> allEntries = table->symList;
        int funcSize = 0;
        for (auto *entry : allEntries) {
            funcSize += sizeofEntry(entry, table);
        }
        table->size = funcSize;
        return funcSize;
    }
}

/*
 * sizeof entry
 * */
int sizeofEntry(SymbolTableEntry *entry, SymbolTable *currentScope) {
    if (entry->size > 0) {
        return entry->size;
    }

    if (entry->type == "integer") {
        entry->size = INT_SIZE;
        return INT_SIZE;
    } else if (entry->type == "float") {
        entry->size = FLOAT_SIZE;
        return FLOAT_SIZE;
    } else if (entry->type.find('[') == std::string::npos) {
        auto *globalScope = currentScope;
        while (globalScope->upperScope) {
            globalScope = globalScope->upperScope;
        }

        auto *structEntry = globalScope->lookup(entry->type, "struct");
        entry->size = sizeofTable(structEntry->link, true);

        return entry->size;
    }

    // must be an array
    int dimSize = getDimsSize(entry->type);
    std::string trimmedType = trimVariableType(entry->type);

    if (trimmedType == "integer") {
        int size = INT_SIZE * dimSize;
        entry->size = size;
        return size;
    } else if (trimmedType == "float") {
        int size = FLOAT_SIZE * dimSize;
        entry->size = size;
        return size;
    } else {
        auto *globalScope = currentScope;
        while (globalScope->upperScope) {
            globalScope = globalScope->upperScope;
        }

        auto *structEntry = globalScope->lookup(trimmedType, "struct");
        int tableSize = sizeofTable(structEntry->link, true);
        entry->size = tableSize * dimSize;
        return tableSize * dimSize;
    }
}

/*
 * sizeof type
 * */
int sizeofType(std::string &type, SymbolTable *currentScope) {
    if (type == "void") {
        return 4; // idk
    } else if (type == "integer") {
        return INT_SIZE;
    } else if (type == "float") {
        return FLOAT_SIZE;
    } else if (type.find('[') == std::string::npos) {
        auto *globalScope = currentScope;
        while (globalScope->upperScope) {
            globalScope = globalScope->upperScope;
        }

        auto *structEntry = globalScope->lookup(type, "struct");
        return sizeofTable(structEntry->link, true);
    }

    // must be an array
    int dimSize = getDimsSize(type);
    std::string trimmedType = trimVariableType(type);
    if (trimmedType == "integer") {
        int size = INT_SIZE * dimSize;
        return size;
    } else if (trimmedType == "float") {
        int size = FLOAT_SIZE * dimSize;
        return size;
    } else {
        auto *globalScope = currentScope;
        while (globalScope->upperScope) {
            globalScope = globalScope->upperScope;
        }

        auto *structEntry = globalScope->lookup(trimmedType, "struct");
        int tableSize = sizeofTable(structEntry->link, true);
        return tableSize * dimSize;
    }
}
