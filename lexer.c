#include <stdio.h>
#include <ctype.h>

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

void ignore(const char *source, size_t *cursor) {
    while(source[*cursor] != '\0') {
        char c = peek(source, *cursor);

        if(isspace((unsigned char)c)) {
            advance(source, cursor);
        } else if(c == '/' && peek(source, *cursor + 1) == '/') {
            while(c != '\n' && c != '\r' && c != '\0') {
                advance(source, cursor);
            }
        } else {
            break;
        }
    }
}