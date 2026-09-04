# S64 — Upgrade

S64 is a minimal low-level programming language focused on integer values, hexadecimal values, bitwise operations, variables, functions, and explicit control over data flow.

## Syntax

S64 currently provides only one keyword:

* `return` — returns a value from a function.

## Symbols

| Symbol | Purpose                         |
| ------ | ------------------------------- |
| `{ }`  | Function/block boundary         |
| `( )`  | Function parameters / arguments |
| `,`    | Parameter separator             |
| `;`    | Statement terminator            |
| `=`    | Variable assignment             |

## Expression

S64 supports the following bitwise operators:

| Operator | Operation   |
| -------- | ----------- |
| `&`      | Bitwise AND |
| `\|`     | Bitwise OR  |
| `~`      | Bitwise NOT |
| `^`      | Bitwise XOR |
| `<<`     | Left shift  |
| `>>`     | Right shift |

Expressions can be combined to perform bitwise operations on supported values.

```s64
value = 0x10 << 2;
result = value & 0x3F;
```

## Supported Values

S64 currently supports two numeric formats:

### Integer

```s64
value = 64;
```

### Hexadecimal

```s64
value = 0x40;
```

Both formats represent integer values and can be used in expressions.

## Variables

Variables are declared through assignment:

```s64
<identifier> = <expression>;
```

Example:

```s64
value = 10;
mask = 0xFF;
result = value & mask;
```

A variable can also use another variable as part of an expression:

```s64
value = 0x10;
result = value << 2;
```

## Functions

Functions are defined using an identifier followed by a parameter list and a block:

```s64
<function_name>(<parameters>) {
    <statement>;
}
```

Example:

```s64
calculate(a, b) {
    result = a & b;
    return result;
}
```

The `{}` block defines the scope of the function.

## Return

A function can return either an expression or an identifier:

```s64
return <expression|identifier>;
```

Example:

```s64
calculate(a, b) {
    result = a ^ b;
    return result;
}
```

An expression can also be returned directly:

```s64
calculate(a, b) {
    return a | b;
}
```

## Complete Example

```s64
calculate(value, mask) {
    result = value & mask;
    result = result << 2;
    return result;
}
```

This example demonstrates the core features currently available in S64:

* Variables
* Integer values
* Hexadecimal values
* Bitwise expressions
* Function parameters
* Functions
* Return statements
* Block scope
