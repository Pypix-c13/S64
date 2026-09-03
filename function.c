#include "stable.c"

typedef struct Function {
    char *label;
    char **parameter;
    uint32_t parameter_count;
    ASTNode *body;
} Function;

typedef struct FunctionTable {
    Function **function;
    size_t count;
} FunctionTable;

Function *add_function(const char *label,char **parameter,uint32_t parameter_count,ASTNode *body) {
    Function *func = (Function *)malloc(sizeof(Function));
    if(func == NULL) return NULL;

    func->label = strdup(label);
    func->parameter = parameter;
    func->parameter_count = parameter_count;
    func->body = body;

    return func;
}

void register_function(FunctionTable *table,Function *function) {
    table->function = realloc(
        table->function,
        sizeof(Function *) * (table->count + 1)
    );

    table->function[table->count++] = function;
}

Function *lookup_function(FunctionTable *table,const char *label) {
    for(size_t i = 0;i < table->count;i++) {
        if(strcmp(table->function[i]->label,label) == 0)
            return table->function[i];
    }

    return NULL;
}

uint32_t execute_function(Function *function) {
    ASTNode *body = function->body;

    for(size_t i = 0;i < body->node_count;i++) {
        ASTNode *node = body->child[i];

        if(node->type == AST_SET)
            evaluate(node->right);

        if(node->type == AST_RETURN) {
            if(node->left != NULL)
                return evaluate(node->left);

            return 0;
        }
    }

    return 0;
}

uint32_t call_function(FunctionTable *table,const char *label) {
    Function *function = lookup_function(table,label);

    if(function == NULL)
        return 0;

    return execute_function(function);
}