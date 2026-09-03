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

Function *add_function(const char *label, char **parameter, uint32_t parameter_count, ASTNode *body) {
    Function *func = (Function *)malloc(sizeof(Function));
    if(func == NULL) return NULL;

    func->label = strdup(label);
    func->parameter = parameter;
    func->parameter_count = parameter_count;
    func->body = body;

    return func;
}

void register_function(FunctionTable *table, Function *function) {
    table->function = realloc(
        table->function,
        sizeof(Function *) * (table->count + 1)
    );
    return table->function[table->count++] = function;
}

Function *lookup_function(FunctionTable *table, const char *label) {
    for(size_t i = 0; i < table->count;i++) {
        if(strcmp(table->function[i]->label, label) == 0) return table->function[i];
    }
    return NULL;
}

uint32_t evaluate_function(Function *function, SymbolTable *table) {
    ASTNode *node = function->body;
    for(size_t i = 0;i < node->node_count;i++) {
        ASTNode *child = node->child[i];

        if(child->type == AST_SET) {
            uint32_t value = evaluate(child->right);
            add_variable(table, child->value, value);
        } else if(child->type == AST_RETURN) {
            if(child->left == NULL) return evaluate(child->left);
            SymbolNode *symbol = lookup(table, child->value);
            if(symbol != NULL) return symbol->value;
            return 0;
        }
    }

    return 0;
}

uint32_t call_function(FunctionTable *table, const char *label, uint32_t *arg) {
    Function *function = lookup_function(table, label);
    if(function == NULL) return 0;

    SymbolNode *node = add_new_table();
    for(size_t i = 0;i < function->parameter_count;i++) add_variable(node, function->parameter[i], arg[i]);
    return evaluate_function(function, node);
}