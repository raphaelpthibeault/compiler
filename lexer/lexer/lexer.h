#ifndef LEXER_H
#define LEXER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
        TokenTypeDollarSign,
        TokenTypeIntType,
        TokenTypeFloatType,
        TokenTypeEOF,
    } TokenType;

    typedef struct SToken {
        TokenType type;
        const char *value;
        size_t line;
    } *Token; // Token being a pointer to SToken

    typedef struct SLexer {
        const char *input;
        size_t inputLength;
        size_t position; // line
        size_t readPosition; // position in input ; should always be 1 ahead of the character we're looking at
        char character;
    } *Lexer;

    // Lexer being a pointer to SLexer

    Lexer lexerNew(const char *input);
    void lexerFree(Lexer *lexer);
    Token lexerNextToken(Lexer lexer);

    Token tokenNew(TokenType type, const char *value, size_t line);
    void tokenFree(Token *token);

    // get all tokens from lexer
    Token *lexerGetAllTokens(Lexer lexer, size_t *length);
    void tokensFreeAll(Token **tokens, size_t *length);

    Token getNextToken(Lexer lexer);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif //LEXER_H
