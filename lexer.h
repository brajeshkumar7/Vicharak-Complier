#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_LEN 100

typedef enum {
    TOKEN_INT,
    TOKEN_IF,

    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_EQUAL,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_SEMICOLON,

    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char text[MAX_TOKEN_LEN];
} Token;

void getNextToken(FILE *file, Token *token);

#endif
