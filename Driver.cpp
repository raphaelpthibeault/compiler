#include <iostream>
#include <util.h>
#include <lexer.h>

static const char *tokenTypeToString_(TokenType type) {
    // should return a string representation of the token type
    switch (type) {
        case TokenTypeId:
            return "ID";
        case TokenTypeInt:
            return "INT";
        case TokenTypeFloat:
            return "FLOAT";
        case TokenTypeEquals:
            return "EQUALS";
        case TokenTypeNotEquals:
            return "NOT_EQUALS";
        case TokenTypeLessThan:
            return "LESS_THAN";
        case TokenTypeGreaterThan:
            return "GREATER_THAN";
        case TokenTypeLessThanOrEquals:
            return "LESS_THAN_OR_EQUALS";
        case TokenTypeGreaterThanOrEquals:
            return "GREATER_THAN_OR_EQUALS";
        case TokenTypePlus:
            return "PLUS";
        case TokenTypeMinus:
            return "MINUS";
        case TokenTypeMultiply:
            return "MULTIPLY";
        case TokenTypeDivide:
            return "DIVIDE";
        case TokenTypeAssign:
            return "ASSIGN";
        case TokenTypeOr:
            return "OR";
        case TokenTypeAnd:
            return "AND";
        case TokenTypeNot:
            return "NOT";
        case TokenTypeLeftParenthesis:
            return "LEFT_PARENTHESIS";
        case TokenTypeRightParenthesis:
            return "RIGHT_PARENTHESIS";
        case TokenTypeLeftBrace:
            return "LEFT_BRACE";
        case TokenTypeRightBrace:
            return "RIGHT_BRACE";
        case TokenTypeLeftBracket:
            return "LEFT_BRACKET";
        case TokenTypeRightBracket:
            return "RIGHT_BRACKET";
        case TokenTypeSemicolon:
            return "SEMICOLON";
        case TokenTypeComma:
            return "COMMA";
        case TokenTypePeriod:
            return "PERIOD";
        case TokenTypeColon:
            return "COLON";
        case TokenTypeArrow:
            return "ARROW";
        case TokenTypeIf:
            return "IF";
        case TokenTypeThen:
            return "THEN";
        case TokenTypeElse:
            return "ELSE";
        case TokenTypeVoid:
            return "VOID";
        case TokenTypePublic:
            return "PUBLIC";
        case TokenTypePrivate:
            return "PRIVATE";
        case TokenTypeFunc:
            return "FUNC";
        case TokenTypeVar:
            return "VAR";
        case TokenTypeStruct:
            return "STRUCT";
        case TokenTypeWhile:
            return "WHILE";
        case TokenTypeRead:
            return "READ";
        case TokenTypeWrite:
            return "WRITE";
        case TokenTypeReturn:
            return "RETURN";
        case TokenTypeSelf:
            return "SELF";
        case TokenTypeInherits:
            return "INHERITS";
        case TokenTypeInvalidId:
            return "INVALID_ID";
        case TokenTypeInvalidInt:
            return "INVALID_INT";
        case TokenTypeInvalidFloat:
            return "INVALID_FLOAT";
        case TokenTypeInvalidChar:
            return "INVALID_CHAR";
        case TokenTypeInlineComment:
            return "INLINE_COMMENT";
        case TokenTypeBlockComment:
            return "BLOCK_COMMENT";
        default:
            return "INVALID"; // should never happen
    }
}

/*
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Error: no input file given\n");
        exit(EXIT_FAILURE);
    }

    printf("Scanning file: %s\n", argv[1]);
    FILE *fp = Fopen(argv[1], "r");

    char *originalFileName = argv[1];

    const char *originalFileNameWithoutExtension = strtok((char*)originalFileName, ".");
    // copy originalFileNameWithoutExtension to outputFileName and errorFileName
    // to output file name append .outlextokens
    // to error file name append .outlexerrors
    char outputFileName[strlen(originalFileNameWithoutExtension) + 12];
    char errorFileName[strlen(originalFileNameWithoutExtension) + 12];
    strcpy(outputFileName, originalFileNameWithoutExtension);
    strcpy(errorFileName, originalFileNameWithoutExtension);
    strcat(outputFileName, ".outlextokens");
    strcat(errorFileName, ".outlexerrors");


    char *input = (char *) Malloc(sizeof(char));
    size_t input_len = 0;
    size_t input_size = 1;
    char c;
    while ((c = fgetc(fp)) != EOF) {
        input[input_len++] = c;
        if (input_len == input_size) {
            input_size *= 2;
            input = (char *) Realloc(input, input_size * sizeof(char));
        }
    }
    input[input_len] = '\0';
    fclose(fp);

    Lexer lexer = lexerNew(input);
    Token *tokens = lexerGetAllTokens(lexer, &input_len);
    size_t tokens_len = input_len;
    Token token;
    fp = Fopen(outputFileName, "w");
    FILE *fp_err = Fopen(errorFileName, "w");
    for (size_t i = 0; i < tokens_len; i++) {
        token = tokens[i];
        if (token->type >= TokenTypeInvalidId && token->type <= TokenTypeInvalidChar) {
            const char *type = tokenTypeToString_(token->type);
            Fwrite("[", sizeof(char), 1, fp_err);
            Fwrite(type, sizeof(char), strlen(type), fp_err);
            Fwrite(", ", sizeof(char), 2, fp_err);
            Fwrite(token->value, sizeof(char), strlen(token->value), fp_err);
            Fwrite(", ", sizeof(char), 2, fp_err);
            Fwrite(std::to_string(token->line).c_str(), sizeof(char), strlen(std::to_string(token->line).c_str()), fp_err);
            Fwrite("]\n", sizeof(char), 2, fp_err);
        } else {
            const char *type = tokenTypeToString_(token->type);
            Fwrite("[", sizeof(char), 1, fp);
            Fwrite(type, sizeof(char), strlen(type), fp);
            Fwrite(", ", sizeof(char), 2, fp);
            Fwrite(token->value, sizeof(char), strlen(token->value), fp);
            Fwrite(", ", sizeof(char), 2, fp);
            Fwrite(std::to_string(token->line).c_str(), sizeof(char), strlen(std::to_string(token->line).c_str()), fp);
            Fwrite("]\n", sizeof(char), 2, fp);
        }
    }
    Fclose(fp);
    Fclose(fp_err);
    lexerFree(&lexer);
    tokensFreeAll(&tokens, &tokens_len);
    Free(input);

    return 0;
}
*/
