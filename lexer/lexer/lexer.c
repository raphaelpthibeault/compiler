#include <lexer.h>
#include <util.h>

/* private function declarations */
static void lexerReadChar_(Lexer lexer);
static void lexerSkipWhitespace_(Lexer lexer);

static const char *lexerReadIdentifier_(Lexer lexer);
static char *lexerReadDigits_(Lexer lexer);
static char *lexerReadInlineComment_(Lexer lexer);
static char *lexerReadBlockComment_(Lexer lexer);
static Token lexerReadPunctuationAndOperators_(Lexer lexer);
static Token lexerReadNumberStartingWithZero_(Lexer lexer);
static Token lexerReadNumberStartingWithNonZero_(Lexer lexer);

static uint8_t lexerIsLetter_(char c);
static uint8_t lexerIsAlphaNumeric_(char c);

static uint8_t lexerIsNonZeroDigit_(char c);
static uint8_t lexerIsDigit_(char c);

static TokenType getTokenType_(const char *id);


/**********************************************************************************************************************
                                           PUBLIC FUNCTIONS
 **********************************************************************************************************************/

Lexer lexerNew(const char *input) {
    size_t length = sizeof(struct SLexer);
    Lexer lexer = Malloc(length);
    memset(lexer, 0, length);

    lexer->input = input;
    lexer->inputLength = strlen(input);
    lexer->position = 1;
    lexer->readPosition = 0;

    lexerReadChar_(lexer);

    return lexer;
}

void lexerFree(Lexer *lexer) {
    if (lexer == NULL || *lexer == NULL) {
        return;
    }

    Free(*lexer);
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

    Free(*tokens);
    *tokens = NULL;
    *length = 0;
}

Token getNextToken(Lexer lexer) {
    // skip comment tokens
    Token token = lexerNextToken(lexer);
    while (token != NULL && (token->type == TokenTypeInlineComment || token->type == TokenTypeBlockComment || token->type == TokenTypeInvalidChar || token->type == TokenTypeInvalidId || token->type == TokenTypeInvalidInt || token->type == TokenTypeInvalidFloat || token->type == TokenTypeIllegal)) {
        tokenFree(&token);
        token = lexerNextToken(lexer);
    }

    if (token == NULL) {
        return tokenNew(TokenTypeEOF, "EOF", (int)lexer->position);
    }

    return token;

}


Token lexerNextToken(Lexer lexer) {
    Token token = NULL;
    lexerSkipWhitespace_(lexer);

    // start off with the punctuation and operators
    token = lexerReadPunctuationAndOperators_(lexer);
    if (token != NULL) {
        return token;
    }

    // reading identifiers and keywords
    if (lexerIsLetter_(lexer->character)) {
        const char *id = lexerReadIdentifier_(lexer);
        TokenType type = getTokenType_(id);
        return tokenNew(type, id, (int)lexer->position);

    } else if (lexer->character == '_') {
        // invalid identifier, but keep reading until whitespace and give error with full invalid identifier
        if (!lexerIsAlphaNumeric_(lexer->input[lexer->readPosition])) {
            lexerReadChar_(lexer);
            return tokenNew(TokenTypeInvalidId, "_", (int)lexer->position);
        }

        const char *id = lexerReadIdentifier_(lexer);
        return tokenNew(TokenTypeInvalidId, id, (int)lexer->position);

    } else if (lexer->character == '0') {
       token = lexerReadNumberStartingWithZero_(lexer);
       if (token != NULL) {
           return token;
       }

    } else if (lexerIsNonZeroDigit_(lexer->character)) {
        token = lexerReadNumberStartingWithNonZero_(lexer);
        if (token != NULL) {
            return token;
        }

    } else if (lexer->character == '/') {
        if (lexer->input[lexer->readPosition] == '/') {
            char const *comment = lexerReadInlineComment_(lexer);
            return tokenNew(TokenTypeInlineComment, comment, (int)lexer->position);
        } else if (lexer->input[lexer->readPosition] == '*') {
            char const *comment = lexerReadBlockComment_(lexer);
            return tokenNew(TokenTypeBlockComment, comment, (int)lexer->position);
        } else {
            lexerReadChar_(lexer);
            return tokenNew(TokenTypeDivide, "/", (int)lexer->position);
        }
    } else {
        // must be an invalid character or EOF
        if (lexer->character == '\0') {
            return NULL;
        }

        char *invalidChar = Malloc(sizeof(char) * 2);
        invalidChar[0] = lexer->character;
        invalidChar[1] = '\0';
        lexerReadChar_(lexer);
        return tokenNew(TokenTypeInvalidChar, invalidChar, (int)lexer->position);
    }

    // if we reach here, something has gone horribly wrong
    return tokenNew(TokenTypeIllegal, NULL, (int)lexer->position);
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

    Free(*token);
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

    // add null terminator to account for a 1 character identifier
    identifier = Realloc(identifier, sizeof(char) * (i + 1));
    identifier[i] = '\0';

    return identifier;
}

static TokenType getTokenType_(const char *id) {
    // compare to all reserved words
    if (strncmp(id, "if", 2) == 0) {
        return TokenTypeIf;
    } else if (strncmp(id, "then", 4) == 0) {
        return TokenTypeThen;
    } else if (strncmp(id, "else", 4) == 0) {
        return TokenTypeElse;
    } else if (strncmp(id, "integer", 7) == 0) {
        return TokenTypeIntType;
    } else if (strncmp(id, "float", 5) == 0) {
        return TokenTypeFloatType;
    } else if (strncmp(id, "void", 4) == 0) {
        return TokenTypeVoid;
    } else if (strncmp(id, "public", 6) == 0) {
        return TokenTypePublic;
    } else if (strncmp(id, "private", 7) == 0) {
        return TokenTypePrivate;
    } else if (strncmp(id, "func", 4) == 0) {
        return TokenTypeFunc;
    } else if (strncmp(id, "var", 3) == 0) {
        return TokenTypeVar;
    } else if (strncmp(id, "struct", 6) == 0) {
        return TokenTypeStruct;
    } else if (strncmp(id, "while", 5) == 0) {
        return TokenTypeWhile;
    } else if (strncmp(id, "read", 4) == 0) {
        return TokenTypeRead;
    } else if (strncmp(id, "write", 5) == 0) {
        return TokenTypeWrite;
    } else if (strncmp(id, "return", 6) == 0) {
        return TokenTypeReturn;
    } else if (strncmp(id, "self", 4) == 0) {
        return TokenTypeSelf;
    } else if (strncmp(id, "inherits", 8) == 0) {
        return TokenTypeInherits;
    } else if (strncmp(id, "let", 3) == 0) {
        return TokenTypeLet;
    } else if (strncmp(id, "impl", 4) == 0) {
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

    // add null terminator to account for a 1 character digit
    digits = Realloc(digits, sizeof(char) * (i + 1));
    digits[i] = '\0';

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
        if (lexer->character == '\n') {
            lexer->position++;  // Increment position for each new line
        }
        i++;

        lexerReadChar_(lexer);

        // Check for start of nested block comment
        if (lexer->character == '/' && lexer->input[lexer->readPosition] == '*') {
            commentDepth++;
            comment = Realloc(comment, sizeof(char) * (i + 1));
            if (comment == NULL) {
                return NULL;
            }
            comment[i] = lexer->character;
            if (lexer->character == '\n') {
                lexer->position++;  // Increment position for each new line
            }
            i++;
            lexerReadChar_(lexer);
        }
            // Check for end of the current block comment
        else if (lexer->character == '*' && lexer->input[lexer->readPosition] == '/') {
            commentDepth--;
            if (commentDepth > 0) {  // If still inside a nested comment
                comment = Realloc(comment, sizeof(char) * (i + 1));
                if (comment == NULL) {
                    return NULL;
                }
                comment[i] = lexer->character;
                if (lexer->character == '\n') {
                    lexer->position++;  // Increment position for each new line
                }
                i++;
            }
            lexerReadChar_(lexer);
        }

    } while (commentDepth > 0 && lexer->character != '\0');

    // Append the final "*/" and null terminator
    comment = Realloc(comment, sizeof(char) * (i + 3));
    if (comment == NULL) {
        return NULL;
    }

    comment[i++] = '*';
    lexerReadChar_(lexer);
    comment[i++] = '/';
    comment[i] = '\0';

    return comment;
}

static Token lexerReadPunctuationAndOperators_(Lexer lexer) {
    Token token = NULL;
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
        default:
            break;
    }

    if (token != NULL) {
        lexerReadChar_(lexer);
        return token;
    }

    return NULL;
}

static Token lexerReadNumberStartingWithZero_(Lexer lexer) {
    if (lexer->input[lexer->readPosition] != '.') {
        if (lexerIsDigit_(lexer->input[lexer->readPosition])) {
            char *invalidNumber = lexerReadDigits_(lexer);
            if (lexer->character != '.') {
                return tokenNew(TokenTypeInvalidInt, invalidNumber, (int)lexer->position);
            }

            invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
            strcat(invalidNumber, ".");
            lexerReadChar_(lexer); // point to the next char

            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
            }

            char *digits = lexerReadDigits_(lexer);
            invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + strlen(digits) + 1));
            strcat(invalidNumber, digits);
            Free(digits);
            digits = NULL;

            if (lexer->character != 'e') {
                return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
            }

            invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
            strcat(invalidNumber, "e");
            lexerReadChar_(lexer); // point to the next char

            if (lexer->character == '+' || lexer->character == '-') {
                invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + 1));
                strcat(invalidNumber, lexer->character == '+' ? "+" : "-");
                lexerReadChar_(lexer); // point to the next char
            }

            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);
            }

            char *exponent = lexerReadDigits_(lexer);
            invalidNumber = Realloc(invalidNumber, sizeof(char) * (strlen(invalidNumber) + strlen(exponent) + 1));
            strcat(invalidNumber, exponent);
            Free(exponent);
            exponent = NULL;

            return tokenNew(TokenTypeInvalidFloat, invalidNumber, (int)lexer->position);

        } else {
            lexerReadChar_(lexer);
            return tokenNew(TokenTypeInt, "0", (int)lexer->position);
        }

    } else {
        char *number = Malloc(sizeof(char) * 3);
        number[0] = '0';
        number[1] = '.';
        number[2] = '\0';

        lexerReadChar_(lexer); // read the .
        lexerReadChar_(lexer); // point to the next char

        // if that character is not a digit, then "0." is invalid float
        if (!lexerIsDigit_(lexer->character)) {
            return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
        }

        char *digits = lexerReadDigits_(lexer);
        number = Realloc(number, sizeof(char) * (strlen(number) + strlen(digits) + 1));
        strcat(number, digits);
        Free(digits);
        digits = NULL;

        // last digit has to be non-zero except in the case of 0.0
        if (number[strlen(number) - 1] == '0' && strncmp(number, "0.0", strlen(number)) != 0) {
            // means something like 0.00 or 0.10
            if (lexer->character != 'e') {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            number = Realloc(number, sizeof(char) * (strlen(number) + 1));
            strcat(number, "e");
            lexerReadChar_(lexer); // read the e

            if (lexer->character == '+' || lexer->character == '-') {
                number = Realloc(number, sizeof(char) * (strlen(number) + 1));
                strcat(number, lexer->character == '+' ? "+" : "-");
                lexerReadChar_(lexer); // read the sign
            }

            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
            }

            // read all the digits
            char *exponent = lexerReadDigits_(lexer);
            number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
            strcat(number, exponent);
            Free(exponent);
            exponent = NULL;

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

        if (lexer->character == '+' || lexer->character == '-') {
            number = Realloc(number, sizeof(char) * (strlen(number) + 1));
            strcat(number, lexer->character == '+' ? "+" : "-");
            lexerReadChar_(lexer); // read the sign
        }

        // now the next sequence should be an integer
        if (!lexerIsDigit_(lexer->character)) {
            return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
        }

        if (lexer->character == '0') {
            char *exponent = lexerReadDigits_(lexer);
            if (strcmp(exponent, "0") == 0) {
                number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
                strcat(number, exponent);
                Free(exponent);
                exponent = NULL;

                return tokenNew(TokenTypeFloat, number, (int)lexer->position);
            }

            number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
            strcat(number, exponent);
            Free(exponent);
            exponent = NULL;

            return tokenNew(TokenTypeInvalidFloat, number, (int)lexer->position);
        }

        char *exponent = lexerReadDigits_(lexer);
        number = Realloc(number, sizeof(char) * (strlen(number) + strlen(exponent) + 1));
        strcat(number, exponent);
        Free(exponent);
        exponent = NULL;

        return tokenNew(TokenTypeFloat, number, (int)lexer->position);
    }
}

static Token lexerReadNumberStartingWithNonZero_(Lexer lexer) {
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
    Free(fraction);
    fraction = NULL;

    if (digits[strlen(digits) - 1] == '0') {
        if (digits[strlen(digits) - 2] != '.') {
            // means something like 1.00
            if (lexer->character != 'e') {
                return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
            }

            digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
            strcat(digits, "e");
            lexerReadChar_(lexer); // read the e

            if (lexer->character == '+' || lexer->character == '-') {
                digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
                strcat(digits, lexer->character == '+' ? "+" : "-");
                lexerReadChar_(lexer); // read the sign
            }

            if (!lexerIsDigit_(lexer->character)) {
                return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
            }

            // read all the digits
            char *exponent = lexerReadDigits_(lexer);
            digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
            strcat(digits, exponent);
            Free(exponent);
            exponent = NULL;

            return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
        }
    }

    if (lexer->character != 'e') {
        return tokenNew(TokenTypeFloat, digits, (int)lexer->position);
    }

    digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
    strcat(digits, "e");
    lexerReadChar_(lexer); // read the e

    if (lexer->character == '+' || lexer->character == '-') {
        digits = Realloc(digits, sizeof(char) * (strlen(digits) + 1));
        strcat(digits, lexer->character == '+' ? "+" : "-");
        lexerReadChar_(lexer); // read the sign
    }

    if (!lexerIsDigit_(lexer->character)) {
        return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
    }

    if (lexer->character == '0') {
        char *exponent = lexerReadDigits_(lexer);
        if (strcmp(exponent, "0") == 0) {
            digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
            strcat(digits, exponent);
            Free(exponent);
            exponent = NULL;

            return tokenNew(TokenTypeFloat, digits, (int)lexer->position);
        }
        digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
        strcat(digits, exponent);
        Free(exponent);
        exponent = NULL;

        return tokenNew(TokenTypeInvalidFloat, digits, (int)lexer->position);
    }

    char *exponent = lexerReadDigits_(lexer);
    digits = Realloc(digits, sizeof(char) * (strlen(digits) + strlen(exponent) + 1));
    strcat(digits, exponent);
    Free(exponent);
    exponent = NULL;

    return tokenNew(TokenTypeFloat, digits, (int)lexer->position);
}


