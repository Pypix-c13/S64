#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum TokenType {
    TYPE_RETURN, TYPE_LBRACE, TYPE_RBRACE, TYPE_COMMA, TYPE_SEMICOLON, TYPE_EQUAL,
    TYPE_LPAREN, TYPE_RPAREN, TYPE_BITWISE_AND, TYPE_BITWISE_OR, TYPE_BITWISE_NOT,
    TYPE_BITWISE_XOR, TYPE_LSHIFT, TYPE_RSHIFT, TYPE_ID, TYPE_INT_LIT, TYPE_HEX_LIT,
    TYPE_EMPTY, TYPE_EOF
} TokenType;

typedef struct Token {
    TokenType type;
    char *value;
} Token;

typedef struct Vector {
    const char *key;
    TokenType type;
} Vector;

static const Vector KEYWORDS[] = {
    {"return", TYPE_RETURN},
    {"<<",     TYPE_LSHIFT},
    {">>",     TYPE_RSHIFT},
    {"{",      TYPE_LBRACE},
    {"}",      TYPE_RBRACE},
    {"(",      TYPE_LPAREN},
    {")",      TYPE_RPAREN},
    {",",      TYPE_COMMA},
    {";",      TYPE_SEMICOLON},
    {"=",      TYPE_EQUAL},
    {"&",      TYPE_BITWISE_AND},
    {"|",      TYPE_BITWISE_OR},
    {"~",      TYPE_BITWISE_NOT},
    {"^",      TYPE_BITWISE_XOR},
    {NULL,     TYPE_EMPTY}
};

Token *create_token(TokenType type, const char *value) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->value = strdup(value);
    return token;
}

void free_token(Token *token) {
    if (!token) return;
    free(token->value);
    free(token);
}

Token *next_token(const char *source, size_t *cursor) {
    while (source[*cursor] != '\0') {
        char c = source[*cursor];

        if (isspace((unsigned char)c)) {
            (*cursor)++;
            continue;
        }

        if (c == '/' && source[*cursor + 1] == '/') {
            *cursor += 2;
            while (source[*cursor] != '\n' && source[*cursor] != '\r' && source[*cursor] != '\0') {
                (*cursor)++;
            }
            continue;
        }

        for (int i = 0; KEYWORDS[i].key != NULL; i++) {
            size_t len = strlen(KEYWORDS[i].key);
            if (strncmp(&source[*cursor], KEYWORDS[i].key, len) == 0) {
                if (isalpha((unsigned char)KEYWORDS[i].key[0])) {
                    char next_char = source[*cursor + len];
                    if (isalnum((unsigned char)next_char) || next_char == '_') {
                        break;
                    }
                }
                *cursor += len;
                return create_token(KEYWORDS[i].type, KEYWORDS[i].key);
            }
        }

        if (isdigit((unsigned char)c)) {
            size_t start = *cursor;
            if (c == '0' && (source[*cursor + 1] == 'x' || source[*cursor + 1] == 'X')) {
                *cursor += 2;
                while (isxdigit((unsigned char)source[*cursor])) (*cursor)++;
                
                size_t length = *cursor - start;
                char val_buf[64];
                strncpy(val_buf, &source[start], length);
                val_buf[length] = '\0';
                return create_token(TYPE_HEX_LIT, val_buf);
            }

            while (isdigit((unsigned char)source[*cursor])) (*cursor)++;
            
            size_t length = *cursor - start;
            char val_buf[64];
            strncpy(val_buf, &source[start], length);
            val_buf[length] = '\0';
            return create_token(TYPE_INT_LIT, val_buf);
        }

        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = *cursor;
            while (isalnum((unsigned char)source[*cursor]) || source[*cursor] == '_') (*cursor)++;
            
            size_t length = *cursor - start;
            char val_buf[64];
            strncpy(val_buf, &source[start], length);
            val_buf[length] = '\0';
            return create_token(TYPE_ID, val_buf);
        }

        (*cursor)++;
    }
    return create_token(TYPE_EOF, "EOF");
}

typedef enum ASTNodeType {
    AST_ROOT, AST_NODE, AST_PARENT, AST_CHILDREN,
    AST_ID, AST_RETURN, AST_FUNC, AST_EXPRESSION,
    AST_LITERAL, AST_BINARY_EXPRESSION, AST_UNARY_EXPRESSION
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;
    TokenType operators;
    struct ASTNode *parent;
    struct ASTNode **children;
    size_t node_count;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

ASTNode *create_ast_node(ASTNodeType type, TokenType op, const char *val) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = type;
    node->operators = op;
    node->value = val ? strdup(val) : NULL;
    node->parent = NULL;
    node->children = NULL;
    node->node_count = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    for (size_t i = 0; i < node->node_count; i++) {
        free_ast(node->children[i]);
    }
    free(node->children);
    free_ast(node->left);
    free_ast(node->right);
    free(node->value);
    free(node);
}

typedef struct Parser {
    Token **tokens;
    size_t current;
    size_t count;
} Parser;

int get_level(TokenType type) {
    switch (type) {
        case TYPE_BITWISE_OR:  return 1;
        case TYPE_BITWISE_XOR: return 2;
        case TYPE_BITWISE_AND: return 3;
        case TYPE_LSHIFT:
        case TYPE_RSHIFT:      return 4;
        case TYPE_BITWISE_NOT: return 5;
        default: return 0;
    }
}

ASTNode *statement_level(Parser *p, int level) {
    Token *t = p->tokens[p->current];
    ASTNode *left = NULL;

    if (t->type == TYPE_BITWISE_NOT) {
        p->current++;
        ASTNode *node = create_ast_node(AST_UNARY_EXPRESSION, t->type, t->value);
        node->left = statement_level(p, 5);
        left = node;
    }
    else if (t->type == TYPE_INT_LIT || t->type == TYPE_HEX_LIT) {
        p->current++;
        left = create_ast_node(AST_LITERAL, t->type, t->value);
    }
    else if (t->type == TYPE_ID) {
        p->current++;
        left = create_ast_node(AST_ID, t->type, t->value);
    }
    else {
        printf("Syntax Error: Unexpected token '%s'\n", t->value);
        exit(1);
    }

    while (p->current < p->count) {
        Token *op_token = p->tokens[p->current];
        int c_level = get_level(op_token->type);
        if (c_level < level) break;

        p->current++;
        ASTNode *node = create_ast_node(AST_BINARY_EXPRESSION, op_token->type, op_token->value);
        node->left = left;
        node->right = statement_level(p, c_level + 1);
        left = node;
    }

    return left;
}

bool match(Parser *p, TokenType type) {
    if (p->current >= p->count) return false;
    return p->tokens[p->current]->type == type;
}

Token *consume(Parser *p) {
    if (p->current < p->count) {
        return p->tokens[p->current++];
    }
    return p->tokens[p->count - 1];
}

Token *expect(Parser *p, TokenType type) {
    if (match(p, type)) {
        return consume(p);
    }
    printf("Syntax Error: Unexpected token '%s'\n", p->tokens[p->current]->value);
    exit(1);
}

typedef struct SymbolNode {
    char *name;
    uint8_t value;
    struct SymbolNode *next;
} SymbolNode;

typedef struct SymbolTable {
    SymbolNode *head;
} SymbolTable;

SymbolTable *root_table() {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    table->head = NULL;
    return table;
}

SymbolNode *lookup(SymbolTable *table, const char *label) {
    SymbolNode *current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, label) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_variable(SymbolTable *table, const char *label, uint8_t value) {
    SymbolNode *node = (SymbolNode *)malloc(sizeof(SymbolNode));
    node->name = strdup(label);
    node->value = value;
    node->next = table->head;
    table->head = node;
}

bool re_assignment(SymbolTable *table, const char *label, uint8_t new_value) {
    SymbolNode *look = lookup(table, label);
    if (look == NULL) return false;
    look->value = new_value;
    return true;
}

uint8_t evaluate(ASTNode *node, SymbolTable *table) {
    if (!node) return 0;

    if (node->type == AST_LITERAL) {
        if (node->operators == TYPE_HEX_LIT) {
            return (uint8_t)strtol(node->value, NULL, 16);
        }
        return (uint8_t)strtol(node->value, NULL, 10);
    }

    if (node->type == AST_ID) {
        SymbolNode *sym = lookup(table, node->value);
        if (sym == NULL) {
            printf("Runtime Error: Variable '%s' not found!\n", node->value);
            exit(1);
        }
        return sym->value;
    }

    if (node->type == AST_UNARY_EXPRESSION) {
        uint8_t value = evaluate(node->left, table);
        if (node->operators == TYPE_BITWISE_NOT) return ~value;
    }

    if (node->type == AST_BINARY_EXPRESSION) {
        uint8_t left = evaluate(node->left, table);
        uint8_t right = evaluate(node->right, table);

        switch (node->operators) {
            case TYPE_BITWISE_AND: return left & right;
            case TYPE_BITWISE_OR:  return left | right;
            case TYPE_BITWISE_XOR: return left ^ right;
            case TYPE_LSHIFT:      return left << right;
            case TYPE_RSHIFT:      return left >> right;
            default: break;
        }
    }

    return 0;
}

ASTNode *variable_statement(Parser *p) {
    Token *id = expect(p, TYPE_ID);
    ASTNode *id_node = create_ast_node(AST_ID, TYPE_ID, id->value);

    ASTNode *assign_node = create_ast_node(AST_EXPRESSION, TYPE_EQUAL, NULL);
    assign_node->left = id_node;

    if (match(p, TYPE_EQUAL)) {
        consume(p);
        assign_node->right = statement_level(p, 1);
    } else {
        assign_node->right = create_ast_node(AST_LITERAL, TYPE_INT_LIT, "0");
    }

    expect(p, TYPE_SEMICOLON);
    return assign_node;
}

ASTNode *return_statement(Parser *p) {
    expect(p, TYPE_RETURN);

    if (match(p, TYPE_SEMICOLON)) {
        printf("Syntax Error: cannot find identifier or expression after 'return' keyword!\n");
        exit(1);
    }

    ASTNode *return_node = create_ast_node(AST_RETURN, TYPE_RETURN, NULL);
    return_node->right = statement_level(p, 1);

    expect(p, TYPE_SEMICOLON);
    return return_node;
}

void add_into_symboltable(ASTNode *node, SymbolTable *table) {
    if (!node || node->operators != TYPE_EQUAL) return;
    const char *var_name = node->left->value;
    uint8_t value = evaluate(node->right, table);

    if (!re_assignment(table, var_name, value)) {
        add_variable(table, var_name, value);
    }
}