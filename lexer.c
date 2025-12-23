#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

#define MAX_TOKEN_LEN 100


void getNextToken(FILE *file, Token *token) {
    int c;

    while ((c = fgetc(file)) != EOF) {

        if (isspace(c))
            continue;

        // Identifiers and keywords
        if (isalpha(c)) {
            int len = 0;
            token->text[len++] = c;

            while (isalnum(c = fgetc(file))) {
                if (len < MAX_TOKEN_LEN - 1)
                    token->text[len++] = c;
            }
            ungetc(c, file);
            token->text[len] = '\0';

            if (strcmp(token->text, "int") == 0)
                token->type = TOKEN_INT;
            else if (strcmp(token->text, "if") == 0)
                token->type = TOKEN_IF;
            else
                token->type = TOKEN_IDENTIFIER;

            return;
        }

        // Numbers
        if (isdigit(c)) {
            int len = 0;
            token->text[len++] = c;

            while (isdigit(c = fgetc(file))) {
                if (len < MAX_TOKEN_LEN - 1)
                    token->text[len++] = c;
            }
            ungetc(c, file);
            token->text[len] = '\0';

            token->type = TOKEN_NUMBER;
            return;
        }

        // Operators and symbols
        switch (c) {

            case '=':
                if ((c = fgetc(file)) == '=') {
                    token->type = TOKEN_EQUAL;
                    strcpy(token->text, "==");
                } else {
                    ungetc(c, file);
                    token->type = TOKEN_ASSIGN;
                    strcpy(token->text, "=");
                }
                return;

            case '+':
                token->type = TOKEN_PLUS;
                strcpy(token->text, "+");
                return;

            case '-':
                token->type = TOKEN_MINUS;
                strcpy(token->text, "-");
                return;

            case ';':
                token->type = TOKEN_SEMICOLON;
                strcpy(token->text, ";");
                return;

            case '(':
                token->type = TOKEN_LPAREN;
                strcpy(token->text, "(");
                return;

            case ')':
                token->type = TOKEN_RPAREN;
                strcpy(token->text, ")");
                return;

            case '{':
                token->type = TOKEN_LBRACE;
                strcpy(token->text, "{");
                return;

            case '}':
                token->type = TOKEN_RBRACE;
                strcpy(token->text, "}");
                return;

            default:
                token->type = TOKEN_UNKNOWN;
                token->text[0] = c;
                token->text[1] = '\0';
                return;
        }
    }

    token->type = TOKEN_EOF;
    token->text[0] = '\0';
}