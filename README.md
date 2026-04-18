Craft-Shell

Craft-Shell is a UNIX-based shell implementation written in C.
It uses a Recursive Descent Parser (RDP) to parse input commands and construct an Abstract Syntax Tree (AST) for execution.

[Recursive Descent Parser](https://en.wikipedia.org/wiki/Recursive_descent_parser) (RDP):

A top-down parsing technique built from a set of mutually-recursive functions.

[Abstract Syntax Tree](https://en.wikipedia.org/wiki/Abstract_syntax_tree) (AST):

A tree-based representation of the parsed command structure.

## Features

- Command execution
- Arguments support
- Input redirection: `<`
- Output redirection: `>`
- Append redirection: `>>`
- Pipes: `|`
- Logical connectors: `&&` and `||`
- Command separator: `;`

## How it works

The shell is split into two main parts:

1. **Parser**
   - Reads the input command line
   - Breaks it into tokens
   - Builds an AST (Abstract Syntax Tree)

2. **Execution Engine**
   - Walks the AST
   - Runs commands
   - Handles pipes, redirections, and connectors
   - Executes built-in commands like `cd` without starting a new process

---

## BNF Grammar

The shell uses a simple BNF-style grammar to describe how commands are formed.

### General idea

A command line can be made from:

- one command
- commands connected with `&&`, `||`, `|`, or `;`
- optional input/output redirection
- one or more arguments

### Grammar

```bnf
<cmd_line>   ::= <cmd> (<connector> <cmd>)*
<connector>   ::= "&&" | "||" | "|" | ";"

<cmd>        ::= <command> <args> <redirect>?
<args>       ::= <arg>*
<redirect>   ::= "<" <file> | ">" <file> | ">>" <file>

<command>    ::= <string>
<arg>        ::= <string>
<file>       ::= <string>
