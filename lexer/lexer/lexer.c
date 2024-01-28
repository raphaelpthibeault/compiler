#include <lexer.h>
#include <util.h>

struct SLexer {
    const char *input;
    size_t inputLength;
    size_t position; // line
    size_t readPosition; // position in input ; should always be 1 ahead of the character we're looking at
    char character;
};

/* private function declarations */
static void lexerReadChar_(Lexer lexer);
static void lexerSkipWhitespace_(Lexer lexer);

static const char *lexerReadIdentifier_(Lexer lexer);
static char *lexerReadDigits_(Lexer lexer);
static char *lexerReadInlineComment_(Lexer lexer);
static char *lexerReadBlockComment_(Lexer lexer);

static uint8_t lexerIsLetter_(char c);
static uint8_t lexerIsAlphaNumeric_(char c);

static uint8_t lexerIsNonZeroDigit_(char c);
static uint8_t lexerIsDigit_(char c);

static TokenType getTokenType_(const char *id, size_t idLength);


/**********************************************************************************************************************
                                           PUBLIC FUNCTIONS
 **********************************************************************************************************************/

Lexer lexerNew(const char *input) {
    size_t length = sizeof(struct SLexer);
    Lexer lexer = Malloc(length);
    memset(lexer, 0, length);

    lexer->input = input;
    lexer->inputLength = strlen(input);
    lexer->position = 0;
    lexer->readPosition = 0;

    lexerReadChar_(lexer);

    return lexer;
}

void lexerFree(Lexer *lexer) {
    if (lexer == NULL || *lexer == NULL) {
        return;
    }

    free(*lexer);
    *lexer = NULL;
}

Token* lexerGetAllTokens(Lexer lexer, size_t *length) {
    Token *tokens = NULL;
    Token token = NULL;

    size_t i = 0;
    while ((token = lexerNextToken(lexer)) != NULL) {
        tokens = Realloc(tokens, sizeof(Token) * (i + 1)); // inefficient
        tokens[i] = token;
        i++;
    }

    *length = i;
    return tokens;
}

void tokensFreeAll(Token **tokens, size_t *length) {
    if (tokens == NULL || *tokens == NULL) {
        return;
    }

    for (size_t i = 0; i < *length; i++) {
        tokenFree(&(*tokens)[i]);
    }

    free(*tokens);
    *tokens = NULL;
    *length = 0;
}

Token lexerNextToken(Lexer lexer) {
    Token token = NULL;
    lexerSkipWhitespace_(lexer);

    // start off with the punctuation and operators
    switch (lexer->character) {
        case '=':
            if (lexer->input[lexer->readPosition] == '=') {
                token = tokenNew(TokenTypeEquals, "==", (int)lexer->position);
                lexerReadChar_(lexer);
            } else {
                token = tokenNew(TokenTypeAssign, "=", (int)lexer->position);
            }
            break;
        case '<':
            if (lexer->input[lexer->readPosition] == '>') {
                token = tokenNew(TokenTypeNotEquals, "<>", (int)lexer->position);
                lexerReadChar_(lexer);
            } else if (lexer->input[lexer->readPosition] == '=') {
                token = tokenNew(TokenTypeLessThanOrEquals, "<=", (int)lexer->position);
                lexerReadChar_(lexer);
            } else {
                token = tokenNew(TokenTypeLessThan, "<", (int)lexer->position);
            }
            break;
        case '>':
            if (lexer->input[lexer->readPosition] == '=') {
                token = tokenNew(TokenTypeGreaterThanOrEquals, ">=", (int)lexer->position);
                lexerReadChar_(lexer);
            } else {
                token = tokenNew(TokenTypeGreaterThan, ">", (int)lexer->position);
            }
            break;
        case '+':
            token = tokenNew(TokenTypePlus, "+", (int)lexer->position);
            break;
        case '-':
            if (lexer->input[lexer->readPosition] == '>') {
                token = tokenNew(TokenTypeArrow, "->", (int)lexer->position);
                lexerReadChar_(lexer);
            } else {
                token = tokenNew(TokenTypeMinus, "-", (int)lexer->position);
            }
            break;
        case '*':
            token = tokenNew(TokenTypeMultiply, "*", (int)lexer->position);
            break;
        case '|':
            token = tokenNew(TokenTypeOr, "|", (int)lexer->position);
            break;
        case '&':
            token = tokenNew(TokenTypeAnd, "&", (int)lexer->position);

            break;
        case '!':
            token = tokenNew(TokenTypeNot, "!", (int)lexer->position);
            break;
        case '(':
            token = tokenNew(TokenTypeLeftParenthesis, "(", (int)lexer->position);
            break;
        case ')':
            token = tokenNew(TokenTypeRightParenthesis, ")", (int)lexer->position);
            break;
        case '{':
            token = tokenNew(TokenTypeLeftBrace, "{", (int)lexer->position);
            break;
        case '}':
            token = tokenNew(TokenTypeRightBrace, "}", (int)lexer->position);
            break;
        case '[':
            token = tokenNew(TokenTypeLeftBracket, "[", (int)lexer->position);
            break;
        case ']':
            token = tokenNew(TokenTypeRightBracket, "]", (int)lexer->position);
            break;
        case ';':
            token = tokenNew(TokenTypeSemicolon, ";", (int)lexer->position);
            break;
        case ',':
            token = tokenNew(TokenTypeComma, ",", (int)lexer->position);
            break;
        case '.':
            token = tokenNew(TokenTypePeriod, ".", (int)lexer->position);
            break;
        case ':':
            token = tokenNew(TokenTypeColon, ":", (int)lexer->position);
            break;
    }

    lexerReadChar_(lexer);

    if (token != NULL) {
        return token;
    }

    // reading identifiers and keywords
    if (lexerIsLetter_(lexer->character)) {
        const char *id = lexerReadIdentifier_(lexer);
        TokenType type = getTokenType_(id, strlen(id));
        return tokenNew(type, id, (int)lexer->position);

    } else if (lexer->character == '_') {
        // invalid identifier, but keep reading until whitespace and give error with full invalid identifier
        const char *id = lexerReadIdentifier_(lexer);
        return tokenNew(TokenTypeInvalidId, id, (int)lexer->position);

    } else if (lexer->character == '0') {
        if (lexer->input[lexer->readPosition] != '.') {
            if (lexerIsDigit_(lexer->input[lexer->readPosition])) {
                char *invalidNumber = lexerReadDigits_(lexer);
                if (lexer->input[lexer->readPosition] != '.') {
                    return tokenNew(TokenTypeInvalidInt, invalidNumber, (int)lexer->position);
                }

                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
                strcat(invalidNumber, ".");
                lexerReadChar_(lexer); // point to the next char

                char *digits = lexerReadDigits_(lexer);
                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + strlen(digits) + 1));
                strcat(invalidNumber, digits);
                free(digits);

                if (lexer->input[lexer->readPosition] != 'e') {
                    return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
                }


                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
                strcat(invalidNumber, "e");
                lexerReadChar_(lexer); // point to the next char

                if (lexer->character != '+' && lexer->character != '-') {
                    return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
                }

                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
                strcat(invalidNumber, lexer->character == '+' ? "+" : "-");
                lexerReadChar_(lexer); // point to the next char

                if (!lexerIsDigit_(lexer->character)) {
                    return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
                }

                if (lexer->character == '0') {
                    // read the following digits if any
                    char *exponent = lexerReadDigits_(lexer);
                    invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + strlen(exponent) + 1));
                    strcat(invalidNumber, exponent);
                    free(exponent);
                    return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
                }

                char *exponent = lexerReadDigits_(lexer);
                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + strlen(exponent) + 1));
                strcat(invalidNumber, exponent);
                free(exponent);
                return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);

            } else {
                return tokenNew(TokenTypeInt, "0", (int)lexer->position);
            }

        } else {
            char *number = Malloc(sizeof(char) * 2);
            number[0] = '0';
            number[1] = '.';
            lexerReadChar_(lexer); // read the .
            lexerReadChar_(lexer); // point to the next char

            // if that character is not a digit, then "0." is invalid float
            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            char *digits = lexerReadDigits_(lexer);
            number = Realloc(number, sizeof(char) * (strlen(number) + strlen(digits) + 1));
            strcat(number, digits);
            free(digits);
            // last digit has to be non-zero
            if (number[strlen(number) - 1] == '0') {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            // if the character is not an e, then we're done
            if (lexer->character != 'e') {
                return tokenNew(TokenTypeFloat, number, (int)lexer->position);
            }

            // read exponent
            number = Realloc(number, sizeof(char) * (strlen(number) + 1));
            strcat(number, "e");
            lexerReadChar_(lexer); // read the e

            // if the character is not a sign, invalid float
            if (lexer->character != '+' && lexer->character != '-') {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            number = Realloc(number, sizeof(char) * (strlen(number) + 1));
            strcat(number, lexer->character == '+' ? "+" : "-");
            lexerReadChar_(lexer); // read the sign

            // now the next sequence should be an integer
            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            if (lexer->character == '0') {
                char *exponent = lexerReadDigits_(lexer);
                number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
                strcat(number, exponent);
                free(exponent);
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            char *exponent = lexerReadDigits_(lexer);
            number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
            strcat(number, exponent);
            free(exponent);
            return tokenNew(TokenTypeFloat, number, (int)lexer->position);
        }


    } else if (lexerIsNonZeroDigit_(lexer->character)) {
        char *digits = lexerReadDigits_(lexer);
        if (lexer->character != '.') {
            return tokenNew(TokenTypeInt, digits, (int)lexer->position);
        }

        digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
        strcat(digits, ".");
        lexerReadChar_(lexer); // point to the next char

        if (!lexerIsDigit_(lexer->character)) {
            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }

        char *fraction = lexerReadDigits_(lexer);
        digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(fraction) + 1));
        strcat(digits, fraction);
        free(fraction);

        if (digits[strlen(digits) - 1] == '0') {
            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }

        if (lexer->character != 'e') {
            return tokenNew(TokenTypeFloat, digits, (int)lexer->position);
        }

        digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
        strcat(digits, "e");
        lexerReadChar_(lexer); // read the e

        if (lexer->character != '+' && lexer->character != '-') {
            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }

        digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
        strcat(digits, lexer->character == '+' ? "+" : "-");
        lexerReadChar_(lexer); // read the sign

        if (!lexerIsDigit_(lexer->character)) {
            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }

        if (lexer->character == '0') {
            char *exponent = lexerReadDigits_(lexer);
            digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
            strcat(digits, exponent);
            free(exponent);
            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }

        char *exponent = lexerReadDigits_(lexer);
        digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
        strcat(digits, exponent);
        free(exponent);
        return tokenNew(TokenTypeFloat, digits, (int)lexer->position);

    } else if (lexer->character == '/') {
        if (lexer->input[lexer->readPosition] == '/') {
            char *comment = lexerReadInlineComment_(lexer);
            return tokenNew(TokenTypeInlineComment, comment, (int)lexer->position);
        } else if (lexer->input[lexer->readPosition] == '*') {
            char *comment = lexerReadBlockComment_(lexer);
            return tokenNew(TokenTypeBlockComment, comment, (int)lexer->position);
        } else {
            return tokenNew(TokenTypeDivide, "/", (int)lexer->position);
        }
    } else {
        // must be an invalid character
        char *invalidChar = Malloc(sizeof(char) * 2);
        invalidChar[0] = lexer->character;
        invalidChar[1] = '\0';
        lexerReadChar_(lexer);
        return tokenNew(TokenTypeInvalidChar, invalidChar, (int)lexer->position);
    }

}

Token tokenNew(TokenType type, const char *value, const size_t line) {
    size_t length = sizeof(struct SToken);
    Token token = Malloc(length);
    if (token == NULL) {
        return NULL;
    }
    memset(token, 0, length);

    token->type = type;
    token->value = value;
    token->line = line;

    return token;
}

void tokenFree(Token *token) {
    if (token == NULL || *token == NULL) {
        return;
    }

    free(*token);
    *token = NULL;
}

/**********************************************************************************************************************
                                           PRIVATE FUNCTIONS
 **********************************************************************************************************************/

static uint8_t lexerIsLetter_(char c) {
    return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z';
}

static uint8_t lexerIsNonZeroDigit_(char c) {
    return '1' <= c && c <= '9';
}

static uint8_t lexerIsDigit_(char c) {
    return '0' <= c && c <= '9';
}

static uint8_t lexerIsAlphaNumeric_(char c) {
    return lexerIsLetter_(c) || lexerIsDigit_(c) || c == '_';
}

static void lexerReadChar_(Lexer lexer) {
    if (lexer->readPosition >= lexer->inputLength) {
        lexer->character = '\0';
    } else {
        lexer->character = lexer->input[lexer->readPosition];
    }

    lexer->position = lexer->readPosition;
    lexer->readPosition++;
}

static void lexerSkipWhitespace_(Lexer lexer) {
   // if newline observed, increment line number
    while (lexer->character == ' ' || lexer->character == '\t' || lexer->character == '\n' || lexer->character == '\r') {
         if (lexer->character == '\n') {
              lexer->position++;
         }

        lexerReadChar_(lexer);
    }
}

static const char *lexerReadIdentifier_(Lexer lexer) {
    char* identifier = NULL;
    size_t i = 0;
    do {
        identifier = Realloc(identifier, sizeof(char) * (i + 1));
        identifier[i] = lexer->character;
        i++;
        lexerReadChar_(lexer);
    } while (lexerIsAlphaNumeric_(lexer->character));

    return identifier;
}

static TokenType getTokenType_(const char *id, size_t idLength) {
    if (idLength == 0) {
        return TokenTypeIllegal; // for testing purposes, should never happen
    }
    // compare to all reserved words
    if (strncmp(id, "if", idLength) == 0) {
        return TokenTypeIf;
    } else if (strncmp(id, "then", idLength) == 0) {
        return TokenTypeThen;
    } else if (strncmp(id, "else", idLength) == 0) {
        return TokenTypeElse;
    } else if (strncmp(id, "void", idLength) == 0) {
        return TokenTypeVoid;
    } else if (strncmp(id, "public", idLength) == 0) {
        return TokenTypePublic;
    } else if (strncmp(id, "private", idLength) == 0) {
        return TokenTypePrivate;
    } else if (strncmp(id, "func", idLength) == 0) {
        return TokenTypeFunc;
    } else if (strncmp(id, "var", idLength) == 0) {
        return TokenTypeVar;
    } else if (strncmp(id, "struct", idLength) == 0) {
        return TokenTypeStruct;
    } else if (strncmp(id, "while", idLength) == 0) {
        return TokenTypeWhile;
    } else if (strncmp(id, "read", idLength) == 0) {
        return TokenTypeRead;
    } else if (strncmp(id, "write", idLength) == 0) {
        return TokenTypeWrite;
    } else if (strncmp(id, "return", idLength) == 0) {
        return TokenTypeReturn;
    } else if (strncmp(id, "self", idLength) == 0) {
        return TokenTypeSelf;
    } else if (strncmp(id, "inherits", idLength) == 0) {
        return TokenTypeInherits;
    } else if (strncmp(id, "let", idLength) == 0) {
        return TokenTypeLet;
    } else if (strncmp(id, "impl", idLength) == 0) {
        return TokenTypeImplements;
    } else {
        return TokenTypeId;
    }
}

static char *lexerReadDigits_(Lexer lexer) {
    char *digits = NULL;
    size_t i = 0;
    do {
        digits = Realloc(digits, sizeof(char) * (i + 1));
        digits[i] = lexer->character;
        i++;
        lexerReadChar_(lexer);
    } while (lexerIsDigit_(lexer->character));

    return digits;
}

static char *lexerReadInlineComment_(Lexer lexer) {
    char *comment = NULL;
    size_t i = 0;
    do {
        comment = Realloc(comment, sizeof(char) * (i + 1));
        comment[i] = lexer->character;
        i++;
        lexerReadChar_(lexer);
    } while (lexer->character != '\n' && lexer->character != '\0');

    comment = Realloc(comment, sizeof(char) * (i + 1));
    comment[i] = '\0';

    return comment;
}

static char *lexerReadBlockComment_(Lexer lexer) {
    char *comment = NULL;
    size_t i = 0;
    int commentDepth = 1;

    do {
        comment = Realloc(comment, sizeof(char) * (i + 1));
        if (comment == NULL) {
            return NULL;
        }

        comment[i] = lexer->character;
        i++;

        lexerReadChar_(lexer);

        if (lexer->character == '/' && lexer->input[lexer->readPosition] == '*') {
            commentDepth++;
            lexerReadChar_(lexer);
        } else if (lexer->character == '*' && lexer->input[lexer->readPosition] == '/') {
            commentDepth--;
            lexerReadChar_(lexer);
        }

    } while (commentDepth > 0 && lexer->character != '\0');

    comment = Realloc(comment, sizeof(char) * (i + 1));
    if (comment == NULL) {
        return NULL;
    }

    comment[i] = '\0';

    return comment;
}