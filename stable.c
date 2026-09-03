#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum TokenType {
    TYPE_SET, TYPE_RETURN, TYPE_LBRACE, TYPE_RBRACE, TYPE_LPAREN,
    TYPE_RPAREN, TYPE_COLON, TYPE_SEMICOLON, TYPE_EQUAL, TYPE_AND_BIT,
    TYPE_OR_BIT, TYPE_NOT_BIT, TYPE_XOR_BIT, TYPE_LEFT_SHIFT, TYPE_RIGHT_SHIFT,
    TYPE_PLUS, TYPE_MIN, TYPE_ASTERISK, TYPE_DIV, TYPE_ID,
    TYPE_INT_LITERAL, TYPE_HEX_LITERAL, TYPE_UNKNOWN, TYPE_EOF
} TokenType;

typedef struct Token {
    TokenType type;
    char *value;
} Token;

typedef struct Lexer {
    const char *source;
    size_t cursor;
} Lexer;

typedef struct Vector2i {
    const char *key;
    TokenType type;
} Vector2i;

typedef enum ASTNodeType {
    AST_ROOT, AST_NODE, AST_PARENT, AST_CHILD,
    AST_SET, AST_ID, AST_RETURN, AST_FUNCTION,
    AST_LITERAL, AST_EXPRESSION, AST_BINARY_LITERAL, AST_UNARY_LITERAL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *value;

    struct ASTNode *parent;
    struct ASTNode **child;
    size_t node_count;

    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

typedef struct Parser {
    Token *token;
    size_t current;
    size_t count;
} Parser;

typedef struct SymbolNode {
    char *label;
    uint32_t value;
    struct SymbolNode *next;
} SymbolNode;

typedef struct SymbolTable {
    SymbolNode *head;
} SymbolTable;

const Vector2i keyword[] = {
    {"set", TYPE_SET}, {"return", TYPE_RETURN}, {"{", TYPE_LBRACE}, {"}", TYPE_RBRACE},
    {"(", TYPE_LPAREN}, {")", TYPE_RPAREN}, {";", TYPE_SEMICOLON}, {":", TYPE_COLON},
    {"=", TYPE_EQUAL}, {"&", TYPE_AND_BIT}, {"|", TYPE_OR_BIT}, {"~", TYPE_NOT_BIT},
    {"^", TYPE_XOR_BIT}, {">>", TYPE_RIGHT_SHIFT}, {"<<", TYPE_LEFT_SHIFT}, {"+", TYPE_PLUS},
    {"-", TYPE_MIN}, {"*", TYPE_ASTERISK}, {"/", TYPE_DIV}, {NULL, TYPE_UNKNOWN}
};

char peek(Lexer *lexer) {
    return lexer->source[lexer->cursor];
}

char advance(Lexer *lexer) {
    char c = peek(lexer);
    if(c != '\0') lexer->cursor++;
    return c;
}

void ignore(Lexer *lexer) {
    while(peek(lexer) != '\0') {
        if(isspace(peek(lexer))) {
            advance(lexer);
            continue;
        }

        if(peek(lexer) == '/' && lexer->source[lexer->cursor + 1] == '/') {
            while(peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0') {
                advance(lexer);
            }
            continue;
        }

        break;
    }
}

Token *next_token(Lexer *lexer) {
    ignore(lexer);

    size_t start = lexer->cursor;
    char c = peek(lexer);

    Token *token = (Token *)malloc(sizeof(Token));
    if(token == NULL) return NULL;

    token->type = TYPE_UNKNOWN;
    token->value = NULL;

    if(c == '\0') {
        token->type = TYPE_EOF;
        token->value = strdup("EOF");
        return token;
    }

    if(c == '0' && (lexer->source[lexer->cursor + 1] == 'x' ||
                    lexer->source[lexer->cursor + 1] == 'X')) {
        advance(lexer);
        advance(lexer);

        while(isxdigit(peek(lexer))) {
            advance(lexer);
        }

        size_t len = lexer->cursor - start;
        token->type = TYPE_HEX_LITERAL;
        token->value = (char *)malloc(len + 1);

        if(token->value == NULL) {
            free(token);
            return NULL;
        }

        strncpy(token->value, lexer->source + start, len);
        token->value[len] = '\0';
        return token;
    }

    if(isdigit(c)) {
        while(isdigit(peek(lexer))) {
            advance(lexer);
        }

        size_t len = lexer->cursor - start;
        token->type = TYPE_INT_LITERAL;
        token->value = (char *)malloc(len + 1);

        if(token->value == NULL) {
            free(token);
            return NULL;
        }

        strncpy(token->value, lexer->source + start, len);
        token->value[len] = '\0';
        return token;
    }

    if(isalpha(c) || c == '_') {
        while(isalnum(peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }

        size_t len = lexer->cursor - start;
        token->type = TYPE_ID;
        token->value = (char *)malloc(len + 1);

        if(token->value == NULL) {
            free(token);
            return NULL;
        }

        strncpy(token->value, lexer->source + start, len);
        token->value[len] = '\0';

        for(size_t i = 0; keyword[i].key != NULL; i++) {
            if(strcmp(token->value, keyword[i].key) == 0) {
                token->type = keyword[i].type;
                break;
            }
        }

        return token;
    }

    for(size_t i = 0; keyword[i].key != NULL; i++) {
        const char *key = keyword[i].key;
        size_t len = strlen(key);

        if(strncmp(lexer->source + start, key, len) == 0) {
            token->type = keyword[i].type;
            token->value = (char *)malloc(len + 1);

            if(token->value == NULL) {
                free(token);
                return NULL;
            }

            strncpy(token->value, key, len);
            token->value[len] = '\0';
            lexer->cursor += len;
            return token;
        }
    }

    advance(lexer);
    token->value = (char *)malloc(2);

    if(token->value == NULL) {
        free(token);
        return NULL;
    }

    token->value[0] = c;
    token->value[1] = '\0';
    return token;
}

Token *current_token(Parser *parser) {
    return &parser->token[parser->current];
}

void consume(Parser *p, TokenType type) {
    if(current_token(p)->type != type) return;
    p->current++;
}

int match(Parser *p, TokenType type) {
    if(current_token(p)->type == type) {
        p->current++;
        return 1;
    }
    return 0;
}

ASTNode *root_init() {
    ASTNode *root = (ASTNode *)malloc(sizeof(ASTNode));
    if(root == NULL) return NULL;

    root->type = AST_ROOT;
    root->value = NULL;
    root->parent = NULL;
    root->child = NULL;
    root->node_count = 0;
    root->left = NULL;
    root->right = NULL;

    return root;
}

ASTNode *add_node(ASTNode *parent) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if(node == NULL) return NULL;

    node->type = AST_NODE;
    node->value = NULL;
    node->parent = parent;
    node->child = NULL;
    node->node_count = 0;
    node->left = NULL;
    node->right = NULL;

    if(parent == NULL) return node;

    ASTNode **new_child = (ASTNode **)realloc(
        parent->child,
        sizeof(ASTNode *) * (parent->node_count + 1)
    );

    if(new_child == NULL) {
        free(node);
        return NULL;
    }

    parent->child = new_child;
    parent->child[parent->node_count] = node;
    parent->node_count++;

    return node;
}

ASTNode *add_parent(ASTNode *parent) {
    return add_node(parent);
}

ASTNode *add_child(ASTNode *parent) {
    return add_node(parent);
}

void free_ast(ASTNode *node) {
    if(node == NULL) return;

    for(size_t i = 0; i < node->node_count; i++) {
        free_ast(node->child[i]);
    }

    free(node->value);
    free(node->child);
    free(node);
}

ASTNode *factor(Parser *p) {
    Token *token = current_token(p);

    if(token->type != TYPE_INT_LITERAL &&
       token->type != TYPE_HEX_LITERAL) return NULL;

    ASTNode *node = add_node(NULL);
    if(node == NULL) return NULL;

    node->type = AST_LITERAL;
    node->value = strdup(token->value);

    consume(p, token->type);
    return node;
}

ASTNode *unary(Parser *p) {
    if(current_token(p)->type == TYPE_NOT_BIT) {
        Token *op = current_token(p);
        consume(p, TYPE_NOT_BIT);

        ASTNode *right = unary(p);
        if(right == NULL) return NULL;

        ASTNode *node = add_node(NULL);
        if(node == NULL) return NULL;

        node->type = AST_UNARY_LITERAL;
        node->value = strdup(op->value);
        node->right = right;

        return node;
    }

    return factor(p);
}

ASTNode *term(Parser *p) {
    ASTNode *left = unary(p);
    if(left == NULL) return NULL;

    while(current_token(p)->type == TYPE_ASTERISK ||
          current_token(p)->type == TYPE_DIV) {

        Token *op = current_token(p);
        consume(p, op->type);

        ASTNode *right = unary(p);
        if(right == NULL) return NULL;

        ASTNode *node = add_node(NULL);
        if(node == NULL) return NULL;

        node->type = AST_BINARY_LITERAL;
        node->value = strdup(op->value);
        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

ASTNode *expression(Parser *p) {
    ASTNode *left = term(p);
    if(left == NULL) return NULL;

    while(current_token(p)->type == TYPE_PLUS ||
          current_token(p)->type == TYPE_MIN) {

        Token *op = current_token(p);
        consume(p, op->type);

        ASTNode *right = term(p);
        if(right == NULL) return NULL;

        ASTNode *node = add_node(NULL);
        if(node == NULL) return NULL;

        node->type = AST_BINARY_LITERAL;
        node->value = strdup(op->value);
        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

int evaluate(ASTNode *node) {
    if(node == NULL) return 0;

    if(node->type == AST_LITERAL) {
        if(strncmp(node->value, "0x", 2) == 0 ||
           strncmp(node->value, "0X", 2) == 0)
            return (int)strtoul(node->value, NULL, 16);
        return atoi(node->value);
    }

    if(node->type == AST_UNARY_LITERAL) {
        int right = evaluate(node->right);
        if(strcmp(node->value, "~") == 0) return ~right;
        return 0;
    }

    int left = evaluate(node->left);
    int right = evaluate(node->right);

    if(strcmp(node->value, "+") == 0) return left + right;
    if(strcmp(node->value, "-") == 0) return left - right;
    if(strcmp(node->value, "*") == 0) return left * right;
    if(strcmp(node->value, "/") == 0) return right != 0 ? left / right : 0;
    if(strcmp(node->value, "&") == 0) return left & right;
    if(strcmp(node->value, "|") == 0) return left | right;
    if(strcmp(node->value, "^") == 0) return left ^ right;
    if(strcmp(node->value, ">>") == 0) return left >> right;
    if(strcmp(node->value, "<<") == 0) return left << right;

    return 0;
}

SymbolTable *add_new_table() {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if(table == NULL) return NULL;

    table->head = NULL;
    return table;
}

void add_variable(SymbolTable *table, const char *label, uint32_t value) {
    SymbolNode *node = (SymbolNode *)malloc(sizeof(SymbolNode));
    if(node == NULL) return;

    node->label = strdup(label);
    node->value = value;
    node->next = table->head;
    table->head = node;
}

void add_label(SymbolTable *table, const char *label) {
    SymbolNode *node = (SymbolNode *)malloc(sizeof(SymbolNode));
    if(node == NULL) return;

    node->label = strdup(label);
    node->value = 0;
    node->next = table->head;
    table->head = node;
}

SymbolNode *lookup(SymbolTable *table, const char *label) {
    SymbolNode *current = table->head;

    while(current != NULL) {
        if(strcmp(current->label, label) == 0) return current;
        current = current->next;
    }
    return NULL;
}

bool update_variable(SymbolTable *table, const char *label, uint32_t value) {
    SymbolNode *symbol = lookup(table, label);
    if(symbol == NULL) return false;
    symbol->value = value;
    return true;
}

ASTNode *set_statement(Parser *p) {
    if(!match(p, TYPE_SET)) return NULL;
    Token *t = current_token(p);

    if(t->type != TYPE_ID) return NULL;
    consume(p, TYPE_ID);

    if(current_token(p)->type != TYPE_EQUAL) return NULL;
    consume(p, TYPE_EQUAL);

    ASTNode *exp = expression(p);
    if(exp == NULL) return NULL;

    ASTNode *node = add_node(NULL);
    if(node == NULL) return NULL;

    node->type = AST_SET;
    node->value = strdup(t->value);
    node->right = exp;

    consume(p, TYPE_SEMICOLON);
    return node;
}

ASTNode *return_statement(Parser *p) {
    if(!match(p, TYPE_RETURN)) return NULL;

    ASTNode *node = add_node(NULL);
    if(node == NULL) return NULL;

    node->type = AST_RETURN;
    node->value = NULL;
    node->left = NULL;
    node->right = NULL;

    if(current_token(p)->type == TYPE_ID) {
        node->value = strdup(current_token(p)->value);
        consume(p, TYPE_ID);
    } else {
        ASTNode *exp = expression(p);
        if(exp == NULL) return NULL;
        node->left = exp;
    }

    consume(p, TYPE_SEMICOLON);
    return node;
}

ASTNode *function_statement(Parser *p) {
    if(current_token(p)->type != TYPE_ID) return NULL;

    Token *t = current_token(p);
    consume(p, TYPE_ID);

    if(current_token(p)->type != TYPE_LPAREN) return NULL;
    consume(p, TYPE_LPAREN);

    ASTNode *node = add_node(NULL);
    if(node == NULL) return NULL;

    node->type = AST_FUNCTION;
    node->value = strdup(t->value);

    while(current_token(p)->type != TYPE_RPAREN) {
        if(current_token(p)->type != TYPE_ID) return NULL;

        ASTNode *param = add_child(node);
        if(param == NULL) return NULL;

        param->type = AST_ID;
        param->value = strdup(current_token(p)->value);

        consume(p, TYPE_ID);
    }

    consume(p, TYPE_RPAREN);
    if(current_token(p)->type != TYPE_LBRACE) return NULL;
    consume(p, TYPE_LBRACE);

    while(current_token(p)->type != TYPE_RBRACE && current_token(p)->type != TYPE_EOF) {
        ASTNode *stmt = set_statement(p);
        if(stmt == NULL) stmt = return_statement(p);
        if(stmt == NULL) return NULL;
        stmt->parent = node;

        ASTNode **new_child = (ASTNode **)realloc(
            node->child,
            sizeof(ASTNode *) * (node->node_count + 1)
        );

        if(new_child == NULL) return NULL;
        node->child = new_child;
        node->child[node->node_count] = stmt;
        node->node_count++;
    }
    consume(p, TYPE_RBRACE);
    return node;
}