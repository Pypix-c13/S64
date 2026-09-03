# S64 Language — Story
S64 was born from a simple question:

> **"Can I create a programming language with just one syntax?"**

That question became the starting point of **S64**.
What started as a simple experiment eventually became a language built around minimalism, low-level operations, and a deliberately limited syntax.

---

# S64 Language — Design

S64 is a **Domain-Specific Language (DSL)** and is **not Turing Complete** by design.
The language intentionally has only two core syntax constructs:

```text
set
return
```

The available symbols are:

```text
{}
()
;
=
&
|
~
^
<<
>>
+
-
*
/
```

S64 is intentionally small.
Rather than adding more and more syntax, S64 focuses on making a very small set of operations useful and meaningful.

---

# S64 Language — Example Code

## Variables
Variables can be created using `set`:

```text
set on = 0x01;
set off = 0x00;
```

S64 currently focuses on integer and hexadecimal values.

---

## Looping
S64 does not provide traditional looping keywords such as `for` or `while`.
Instead, recursive function calls can be used:

```text
my_function() {
    my_function();
}
```

The function continuously calls itself, creating recursive execution.

---

## Looping with `return`
A `return` statement can be used to stop the recursive execution:

```text
my_function() {
    my_function();
    return 0x00;
}
```

Here, `0x00` is returned when the function reaches the `return` statement.

---

## Math Expression

S64 supports arithmetic expressions:

```text
set after_second = 30 + 0x0001;
```

S64 also provides bitwise and bit-shifting operators:

```text
&
|
~
^
<<
>>
```

along with the basic arithmetic operators:

```text
+
-
*
/
```

---

# The Idea Behind S64
S64 is intentionally limited.

It is not designed to be a replacement for a general-purpose programming language. Instead, it is an exploration of how far a language can go when its syntax is kept extremely small.

> **Less syntax. More meaning.**

S64 began with one question.
The project is an attempt to find the answer.
