#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

Token currentToken;
FILE *asmFile;
int labelCounter = 0;

void advance(FILE *f) {
    getNextToken(f, &currentToken);
}

void error(const char *msg) {
    printf("Parser Error: %s\n", msg);
    exit(1);
}

typedef enum {
    NODE_PROGRAM,
    NODE_DECL,
    NODE_ASSIGN,
    NODE_BINOP,
    NODE_IF,
    NODE_VAR,
    NODE_NUM
} NodeType;

typedef struct ASTNode {
    NodeType type;

    char name[32];
    int value;
    char op;

    struct ASTNode *left;
    struct ASTNode *right;

    struct ASTNode *condition;
    struct ASTNode *body;
    struct ASTNode *next;
} ASTNode;

#define MAX_SYMBOLS 100
#define MEMORY_START 0x10

typedef struct {
    char name[32];
    int memory_address;
} Symbol;

Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;
int nextMemoryAddress = MEMORY_START;


int lookupSymbol(const char *name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return symbolTable[i].memory_address;
        }
    }
    return -1;  // not found
}

void addSymbol(const char *name) {
    if (lookupSymbol(name) != -1) {
        error("Variable redeclared");
    }

    if (symbolCount >= MAX_SYMBOLS) {
        error("Symbol table overflow");
    }

    strcpy(symbolTable[symbolCount].name, name);
    symbolTable[symbolCount].memory_address = nextMemoryAddress;

    symbolCount++;
    nextMemoryAddress++;
}


ASTNode *newNode(NodeType type) {
    ASTNode *n = malloc(sizeof(ASTNode));
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    return n;
}


/* Forward declarations */
ASTNode *parseProgram(FILE *);
ASTNode *parseStatement(FILE *);
ASTNode *parseDeclaration(FILE *);
ASTNode *parseAssignment(FILE *);
ASTNode *parseIfStatement(FILE *);
ASTNode *parseExpression(FILE *);
ASTNode *parseTerm(FILE *);
ASTNode *parseCondition(FILE *);

void generateCode(ASTNode *node);
void generateStatement(ASTNode *node);
void generateExpression(ASTNode *node);


ASTNode *parseStatement(FILE *f) {

    if (currentToken.type == TOKEN_INT) {
        return parseDeclaration(f);
    }

    if (currentToken.type == TOKEN_IDENTIFIER) {
        return parseAssignment(f);
    }

    if (currentToken.type == TOKEN_IF) {
        return parseIfStatement(f);
    }

    error("Unknown statement");
    return NULL;
}
ASTNode *parseProgram(FILE *f) {
    ASTNode *head = NULL;
    ASTNode *tail = NULL;

    while (currentToken.type != TOKEN_EOF) {
        ASTNode *stmt = parseStatement(f);
        if (!head) head = tail = stmt;
        else {
            tail->next = stmt;
            tail = stmt;
        }
    }

    ASTNode *prog = newNode(NODE_PROGRAM);
    prog->body = head;
    return prog;
}

ASTNode *parseDeclaration(FILE *f) {
    advance(f); // consume 'int'

    if (currentToken.type != TOKEN_IDENTIFIER)
        error("Expected identifier after int");

    ASTNode *n = newNode(NODE_DECL);
    strcpy(n->name, currentToken.text);
    addSymbol(n->name);

    advance(f); // identifier

    if (currentToken.type != TOKEN_SEMICOLON)
        error("Missing semicolon");

    advance(f);
    return n;
}

ASTNode *parseAssignment(FILE *f) {
    if (lookupSymbol(currentToken.text) == -1) {
        error("Variable used before declaration");
    }

    ASTNode *n = newNode(NODE_ASSIGN);

    ASTNode *var = newNode(NODE_VAR);
    strcpy(var->name, currentToken.text);

    advance(f); // identifier

    if (currentToken.type != TOKEN_ASSIGN)
        error("Expected =");

    advance(f);

    ASTNode *expr = parseExpression(f);

    if (currentToken.type != TOKEN_SEMICOLON)
        error("Missing semicolon");

    advance(f);

    n->left = var;
    n->right = expr;
    return n;
}

ASTNode *parseIfStatement(FILE *f) {
    ASTNode *n = newNode(NODE_IF);

    advance(f); // if

    if (currentToken.type != TOKEN_LPAREN)
        error("Expected (");

    advance(f);

    n->condition = parseCondition(f);

    if (currentToken.type != TOKEN_RPAREN)
        error("Expected )");

    advance(f);

    if (currentToken.type != TOKEN_LBRACE)
        error("Expected {");

    advance(f);

    ASTNode *head = NULL;
    ASTNode *tail = NULL;

    while (currentToken.type != TOKEN_RBRACE) {
        ASTNode *stmt = parseStatement(f);
        if (!head) head = tail = stmt;
        else {
            tail->next = stmt;
            tail = stmt;
        }
    }

    n->body = head;
    advance(f); // }

    return n;
}

ASTNode *parseCondition(FILE *f) {
    ASTNode *left = newNode(NODE_VAR);
    strcpy(left->name, currentToken.text);

    advance(f);

    if (currentToken.type != TOKEN_EQUAL)
        error("Expected ==");

    advance(f);

    ASTNode *right = parseTerm(f);

    ASTNode *cond = newNode(NODE_BINOP);
    cond->op = '=';  // equality
    cond->left = left;
    cond->right = right;

    return cond;
}

ASTNode *parseExpression(FILE *f) {
    ASTNode *left = parseTerm(f);

    if (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        ASTNode *op = newNode(NODE_BINOP);
        op->op = currentToken.text[0];

        advance(f);

        op->left = left;
        op->right = parseTerm(f);
        return op;
    }

    return left;
}

ASTNode *parseTerm(FILE *f) {
    ASTNode *n;

    if (currentToken.type == TOKEN_IDENTIFIER) {
      if (lookupSymbol(currentToken.text) == -1) {
           error("Variable used before declaration");
        }
        n = newNode(NODE_VAR);
        strcpy(n->name, currentToken.text);
    } else if (currentToken.type == TOKEN_NUMBER) {
        n = newNode(NODE_NUM);
        n->value = atoi(currentToken.text);
    } else {
        error("Invalid term");
    }

    advance(f);
    return n;
}


void generateCode(ASTNode *node) {
    if (!node) return;

    if (node->type == NODE_PROGRAM) {
        ASTNode *stmt = node->body;
        while (stmt) {
            generateStatement(stmt);
            stmt = stmt->next;
        }
    }
}

void generateStatement(ASTNode *node) {

    if (node->type == NODE_DECL) {
        // No assembly needed (memory already allocated)
        return;
    }

    if (node->type == NODE_ASSIGN) {
        int addr = lookupSymbol(node->left->name);

        generateExpression(node->right);

        fprintf(asmFile, "STORE A, 0x%02X\n", addr);
        return;
    }

    if (node->type == NODE_IF) {
        int label = labelCounter++;

        int addr = lookupSymbol(node->condition->left->name);

        fprintf(asmFile, "LOAD A, 0x%02X\n", addr);

        if (node->condition->right->type == NODE_NUM) {
            fprintf(asmFile, "CMP A, %d\n", node->condition->right->value);
        } else {
            int raddr = lookupSymbol(node->condition->right->name);
            fprintf(asmFile, "CMP A, 0x%02X\n", raddr);
        }

        fprintf(asmFile, "JNZ if_end_%d\n", label);

        ASTNode *stmt = node->body;
        while (stmt) {
            generateStatement(stmt);
            stmt = stmt->next;
        }

        fprintf(asmFile, "if_end_%d:\n", label);
    }
}

void generateExpression(ASTNode *node) {

    if (node->type == NODE_NUM) {
        fprintf(asmFile, "LOADI A, %d\n", node->value);
        return;
    }

    if (node->type == NODE_VAR) {
        int addr = lookupSymbol(node->name);
        fprintf(asmFile, "LOAD A, 0x%02X\n", addr);
        return;
    }

    if (node->type == NODE_BINOP) {
        generateExpression(node->left);
        fprintf(asmFile, "PUSH A\n");

        generateExpression(node->right);
        fprintf(asmFile, "POP B\n");

        if (node->op == '+')
            fprintf(asmFile, "ADD A, B\n");
        else
            fprintf(asmFile, "SUB A, B\n");

        return;
    }
}


int main() {
    FILE *f = fopen("input.txt", "r");
    if (!f) {
        perror("File error");
        return 1;
    }

    advance(f);
    ASTNode *ast = parseProgram(f);

    asmFile = fopen("output.asm", "w");
    if (!asmFile) {
        perror("Assembly file error");
        return 1;
    }

    fprintf(asmFile, "; -------- SimpleLang Compiler Output --------\n");
    fprintf(asmFile, "; Variables:\n");
    for (int i = 0; i < symbolCount; i++) {
        fprintf(asmFile, "; %s -> 0x%02X\n",
                symbolTable[i].name,
                symbolTable[i].memory_address);
    }
    fprintf(asmFile, "; -------------------------------------------\n\n");

    generateCode(ast);

    printf("Parsing completed successfully.\n");
    fclose(asmFile);
    fclose(f);
    printf("Assembly code generated in output.asm\n");
    return 0;
}
