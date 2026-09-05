#include "std.h"

ASTNode *function_statement(Parser *p) {
    Token *t = expect(p, TYPE_ID);
    ASTNode *func_node = create_ast_node(AST_RETURN, TYPE_ID, t->value);
    expect(p, TYPE_LPAREN);

    if(match(p, TYPE_ID)) {
        Token *param = consume(p);
        func_node->left = create_ast_node(AST_RETURN, TYPE_ID, param->value);
    }

    expect(p, TYPE_RPAREN);
    expect(p, TYPE_LBRACE);

    ASTNode *body = create_ast_node(AST_CHILDREN, TYPE_EMPTY, NULL);
    while(!match(p, TYPE_RBRACE) && !match(p, TYPE_EOF)) {
        ASTNode *stmt = NULL;
        if (match(p, TYPE_RETURN)) stmt = return_statement(p);
        else if (match(p, TYPE_ID)) stmt = variable_statement(p);
        else {
            printf("Syntax Error: Unknown statement in function body '%s'\n", p->tokens[p->current]->value);
            exit(1);
        }

        ASTNode **new_children = (ASTNode **)realloc(
            body->children,
            sizeof(ASTNode *) * (body->node_count + 1)
        );

        if(!new_children) {
            printf("Memory Error: Failed to reallocate body children\n");
            exit(1);
        }

        body->children = new_children;
        body->children[body->node_count++] = stmt;
    }

    expect(p, TYPE_RBRACE);
    func_node->right = body;
    return func_node;
}