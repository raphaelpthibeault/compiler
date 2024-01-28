#ifndef LEXER_H
#define LEXER_H

#ifdef _cplusplus
// will use google test for unit testing so have to use C linkage
extern "C" {
#endif // _cplusplus

    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>

    typedef enum {
        TokenTypeId,
        TokenTypeInt,
        TokenTypeFloat,
        TokenTypeEquals,
        TokenTypeNotEquals,
        TokenTypeLessThan,
        TokenTypeGreaterThan,
        TokenTypeLessThanOrEquals,
        TokenTypeGreaterThanOrEquals,
        TokenTypePlus,
        TokenTypeMinus,
        TokenTypeMultiply,
        TokenTypeDivide,
        TokenTypeAssign,
        TokenTypeOr,
        TokenTypeAnd,
        TokenTypeNot,
        TokenTypeLeftParenthesis,
        TokenTypeRightParenthesis,
        TokenTypeLeftBrace,
        TokenTypeRightBrace,
        TokenTypeLeftBracket,
        TokenTypeRightBracket,
        TokenTypeSemicolon,
        TokenTypeComma,
        TokenTypePeriod,
        TokenTypeColon,
        TokenTypeArrow,
        TokenTypeIf,
        TokenTypeThen,
        TokenTypeElse,
        TokenTypeVoid,
        TokenTypePublic,
        TokenTypePrivate,
        TokenTypeFunc,
        TokenTypeVar,
        TokenTypeStruct,
        TokenTypeWhile,
        TokenTypeRead,
        TokenTypeWrite,
        TokenTypeReturn,
        TokenTypeSelf,
        TokenTypeInherits,
        TokenTypeLet,
        TokenTypeImplements,
        TokenTypeIllegal,
        TokenTypeInvalidId,
        TokenTypeInvalidInt,
        TokenTypeInvalidFloat,
        TokenTypeInvalidChar,
        TokenTypeInlineComment,
        TokenTypeBlockComment,
    } TokenType;

    typedef struct SToken {
        TokenType type;
        const char *value;
        size_t line;
    } *Token; // Token being a pointer to SToken

    typedef struct SLexer *Lexer; // Lexer being a pointer to SLexer
    // generally referenced as a double pointer so it becomes NULL after free
    // also to avoid use-after-free errors without having to rely on checking Valgrind.

    Lexer lexerNew(const char *input);
    void lexerFree(Lexer *lexer);
    Token lexerNextToken(Lexer lexer);

    Token tokenNew(TokenType type, const char *value, size_t line);
    void tokenFree(Token *token);

    // get all tokens from lexer
    Token *lexerGetAllTokens(Lexer lexer, size_t *length);
    void tokensFreeAll(Token **tokens, size_t *length);

#ifdef _cplusplus
};
#endif // _cplusplus


#endif //LEXER_H
