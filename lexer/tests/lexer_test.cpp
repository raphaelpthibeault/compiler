#include<gtest/gtest.h>
#include<lexer.h>

TEST(SanityCheck, BasicAssertions) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}

TEST(LEXER, LexerNew) {
    Lexer lexer = lexerNew("test");
    ASSERT_STREQ(lexer->input, "test");
    ASSERT_EQ(lexer->inputLength, 4);
    ASSERT_EQ(lexer->position, 1);
    ASSERT_EQ(lexer->readPosition, 1);
    ASSERT_EQ(lexer->character, 't');
    lexerFree(&lexer);
    lexer = nullptr;
}

TEST(LEXER, TokenNew) {
    Token token = tokenNew(TokenTypeIllegal, "test", 1);
    ASSERT_EQ(token->type, TokenTypeIllegal);
    ASSERT_STREQ(token->value, "test");
    ASSERT_EQ(token->line, 1);
    tokenFree(&token);
    token = nullptr;
}

TEST(lexerNextToken, returnsNULL) {
    Lexer lexer = lexerNew("");
    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token, nullptr);

    lexerFree(&lexer);
    tokenFree(&token);
}

// now we test lexerNextToken with
// (1) all operators
// (2) all punctuation
// (3) all keywords
// (4) some identifiers and invalid identifiers
// (5) integers and invalid integers
// (6) floats and invalid floats
// (7) invalid chars
// (8) inline comments
// (9) block comments
// (10) odd cases e.g. 0a as int and id, 0.0.0 as "0.0, period and 0", 00. as invalid int and a period
//                      00.0 00.00,
TEST(lexerNextToken, operators) {
    Lexer lexer = lexerNew(
            "==<> < ><=>= \n + - * / \n=\n|&!"
    );
    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeEquals);
    ASSERT_STREQ(token->value, "==");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeNotEquals);
    ASSERT_STREQ(token->value, "<>");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLessThan);
    ASSERT_STREQ(token->value, "<");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeGreaterThan);
    ASSERT_STREQ(token->value, ">");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLessThanOrEquals);
    ASSERT_STREQ(token->value, "<=");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeGreaterThanOrEquals);
    ASSERT_STREQ(token->value, ">=");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypePlus);
    ASSERT_STREQ(token->value, "+");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeMinus);
    ASSERT_STREQ(token->value, "-");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeMultiply);
    ASSERT_STREQ(token->value, "*");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeDivide);
    ASSERT_STREQ(token->value, "/");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeAssign);
    ASSERT_STREQ(token->value, "=");
    ASSERT_EQ(token->line, 3);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeOr);
    ASSERT_STREQ(token->value, "|");
    ASSERT_EQ(token->line, 4);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeAnd);
    ASSERT_STREQ(token->value, "&");
    ASSERT_EQ(token->line, 4);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeNot);
    ASSERT_STREQ(token->value, "!");
    ASSERT_EQ(token->line, 4);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, punctuation) {
    Lexer lexer = lexerNew(
            "(){}[];,.:->"
    );
    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLeftParenthesis);
    ASSERT_STREQ(token->value, "(");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeRightParenthesis);
    ASSERT_STREQ(token->value, ")");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLeftBrace);
    ASSERT_STREQ(token->value, "{");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeRightBrace);
    ASSERT_STREQ(token->value, "}");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLeftBracket);
    ASSERT_STREQ(token->value, "[");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeRightBracket);
    ASSERT_STREQ(token->value, "]");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeSemicolon);
    ASSERT_STREQ(token->value, ";");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeComma);
    ASSERT_STREQ(token->value, ",");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypePeriod);
    ASSERT_STREQ(token->value, ".");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeColon);
    ASSERT_STREQ(token->value, ":");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeArrow);
    ASSERT_STREQ(token->value, "->");
    ASSERT_EQ(token->line, 1);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, keywords) {
    Lexer lexer = lexerNew(
            "if then else integer float void public private func var struct while read write return self inherits let impl"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeIf);
    ASSERT_STREQ(token->value, "if");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeThen);
    ASSERT_STREQ(token->value, "then");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeElse);
    ASSERT_STREQ(token->value, "else");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "integer");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "float");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeVoid);
    ASSERT_STREQ(token->value, "void");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypePublic);
    ASSERT_STREQ(token->value, "public");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypePrivate);
    ASSERT_STREQ(token->value, "private");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFunc);
    ASSERT_STREQ(token->value, "func");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeVar);
    ASSERT_STREQ(token->value, "var");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeStruct);
    ASSERT_STREQ(token->value, "struct");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeWhile);
    ASSERT_STREQ(token->value, "while");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeRead);
    ASSERT_STREQ(token->value, "read");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeWrite);
    ASSERT_STREQ(token->value, "write");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeReturn);
    ASSERT_STREQ(token->value, "return");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeSelf);
    ASSERT_STREQ(token->value, "self");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInherits);
    ASSERT_STREQ(token->value, "inherits");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeLet);
    ASSERT_STREQ(token->value, "let");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeImplements);
    ASSERT_STREQ(token->value, "impl");
    ASSERT_EQ(token->line, 1);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, identifiers) {
    Lexer lexer = lexerNew(
            "abc _abc _abc123 abc123 _abc123 abc_123 _abc_123 abc_123_abc__ a _ b"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "abc");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidId);
    ASSERT_STREQ(token->value, "_abc");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidId);
    ASSERT_STREQ(token->value, "_abc123");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "abc123");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidId);
    ASSERT_STREQ(token->value, "_abc123");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "abc_123");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidId);
    ASSERT_STREQ(token->value, "_abc_123");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "abc_123_abc__");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "a");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidId);
    ASSERT_STREQ(token->value, "_");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeId);
    ASSERT_STREQ(token->value, "b");
    ASSERT_EQ(token->line, 1);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, integers) {
    Lexer lexer = lexerNew(
            "0 1 10 101 00 01 010 0101\n1230"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "0");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "1");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "10");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "101");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidInt);
    ASSERT_STREQ(token->value, "00");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidInt);
    ASSERT_STREQ(token->value, "01");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidInt);
    ASSERT_STREQ(token->value, "010");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidInt);
    ASSERT_STREQ(token->value, "0101");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInt);
    ASSERT_STREQ(token->value, "1230");
    ASSERT_EQ(token->line, 2);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, floatsNoExponent) {
    Lexer lexer = lexerNew(
            "0. 00. 0.0 00.0 00.00   0.010 0.0101 0.10101 1. 10. 10.0 10.00 10.010 10.0101 101.101"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "0.0");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.0");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.00");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.010");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "0.0101");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "0.10101");

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "10.0");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.010");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "10.0101");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "101.101");
    ASSERT_EQ(token->line, 1);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, floatWithExponent) {
    Lexer lexer = lexerNew(
            "0.0e 0.00e 0.0e+ 0.00e+ 00.0e 00.00e 00.0e+ 00.00e+ 00.00e+1 00.00e+01 0.01e+0 0.01e+01 0.01e+10\n"
            "1.0e 1.00e 1.0e+ 1.00e+ 10.0e 10.00e 10.0e+ 10.00e+ 10.00e+1 10.00e+10 10.00e+01 01.00e+01 1.01e+0 1.01e+01 1.01e+10"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.0e");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.00e");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.0e+");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.00e+");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.0e");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.00e");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.0e+");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.00e+");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.00e+1");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "00.00e+01");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "0.01e+0");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "0.01e+01");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "0.01e+10");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.0e");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.00e");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.0e+");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.00e+");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.0e");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00e");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.0e+");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00e+");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00e+1");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00e+10");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "10.00e+01");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "01.00e+01");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "1.01e+0");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidFloat);
    ASSERT_STREQ(token->value, "1.01e+01");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeFloat);
    ASSERT_STREQ(token->value, "1.01e+10");
    ASSERT_EQ(token->line, 2);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, invalidChars) {
    Lexer lexer = lexerNew(
            "@ # $ ' \\ ~ % ? # $ % ^"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "@");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "#");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "$");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "'");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "\\");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "~");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "%");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "?");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "#");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "$");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "%");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInvalidChar);
    ASSERT_STREQ(token->value, "^");
    ASSERT_EQ(token->line, 1);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, inlineComments) {
    Lexer lexer = lexerNew(
            "// this is an inline comment\n"
            "// this is another inline comment\n"
            "// this is a third inline comment but this time with an end of file\0"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInlineComment);
    ASSERT_STREQ(token->value, "// this is an inline comment");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInlineComment);
    ASSERT_STREQ(token->value, "// this is another inline comment");
    ASSERT_EQ(token->line, 2);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeInlineComment);
    ASSERT_STREQ(token->value, "// this is a third inline comment but this time with an end of file");
    ASSERT_EQ(token->line, 3);

    lexerFree(&lexer);
    tokenFree(&token);
}

TEST(lexerNextToken, blockComments) {
    Lexer lexer = lexerNew(
            "/* this is single-line block comment */"
            "/* this is a\nmultiple line\nblock comment \n*/"
            "/* this is a multiline imbricated \n/* block comment\n*/\n*/"
            "/* /* this is a single-line imbricated comment  */ */"
    );

    Token token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeBlockComment);
    ASSERT_STREQ(token->value, "/* this is single-line block comment */");
    ASSERT_EQ(token->line, 1);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeBlockComment);
    ASSERT_STREQ(token->value, "/* this is a\nmultiple line\nblock comment \n*/");
    ASSERT_EQ(token->line, 4);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeBlockComment);
    ASSERT_STREQ(token->value, "/* this is a multiline imbricated \n/* block comment\n*/\n*/");
    ASSERT_EQ(token->line, 7);

    token = lexerNextToken(lexer);
    ASSERT_EQ(token->type, TokenTypeBlockComment);
    ASSERT_STREQ(token->value, "/* /* this is a single-line imbricated comment  */ */");
    ASSERT_EQ(token->line, 7);

    lexerFree(&lexer);
    tokenFree(&token);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}