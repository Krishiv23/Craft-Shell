# Craft-Shell

> A UNIX-based shell implementation in C, powered by a Recursive Descent Parser and an AST-driven execution engine.

---

## Overview

Craft-Shell does not execute input strings directly. Instead, it parses every command into an **Abstract Syntax Tree (AST)** first, then walks that tree to run each node. This makes it straightforward to handle complex inputs with pipes, redirections, logical operators, and chained commands.

**Key concepts:**

- **Recursive Descent Parser (RDP)** — A top-down parsing technique built from a set of mutually recursive functions.
- **Abstract Syntax Tree (AST)** — A tree that represents the structure of a command, not just its tokens.

---

## Features

| Feature | Syntax |
|---|---|
| Command execution | `ls`, `grep`, `echo`, … |
| Arguments | `ls -l /home` |
| Pipe | `\|` |
| Logical AND | `&&` |
| Logical OR | `\|\|` |
| Command separator | `;` |
| Input redirection | `<` |
| Output redirection | `>` |
| Append redirection | `>>` |
| Built-ins | `cd` |

---

## How It Works

### 1. Parsing

The parser reads the command line, splits it into tokens, and constructs the AST bottom-up:

```
Read input → Tokenize → Build command node → Attach args → Attach redirections → Wrap in connectors → Return AST
```

### 2. Execution

The execution engine walks the finished AST and handles each node by type:

- **Command node** — collects the name and arguments, runs built-ins directly or dispatches to `execvp`.
- **Redirection node** — sets up file descriptors before the command runs (`<`, `>`, `>>`).
- **Pipe node** — forks two processes; left writes, right reads.
- **Connector node** — evaluates `&&`, `||`, `;` according to exit status.

---

## AST Node Types

### Command nodes
Leaf nodes holding a command name, its argument list (linked list), and any redirection info.

### Argument nodes
Attached to a command node as a linked list, keeping the command and its parameters together.

```sh
ls -l /home
# ls       → command node
# -l /home → argument nodes
```

### Redirection nodes
Store file-based I/O handling — `< input.txt`, `> output.txt`, `>> log.txt`.

### Connector nodes
Parent nodes that sit above two sub-trees and link them with `|`, `&&`, `||`, or `;`.

---

## AST Example

```sh
cat file.txt | grep hello
```

```
        |  (connector)
       / \
    cat   grep
   file.txt  hello
```

The pipe node is the root. `cat file.txt` is its left child; `grep hello` is its right child. The engine runs them in separate processes and wires the pipe between them.

---

## Example Commands

```sh
ls -l
cat input.txt
echo hello > out.txt
cat input.txt | grep main
make && ./app
false || echo failed
echo first ; echo second
```

---

## Build & Run

```sh
gcc src/*.c -o craft-shell
./craft-shell
```

---

## Project Structure

```
craft-shell/
├── src/
│   ├── parser.c            # builds the AST
│   ├── execution_engine.c  # traverses and runs the AST
│   └── Grammer.txt         # grammar reference
└── README.md
```

> `Grammer.txt` is a reference file describing the shell's formal grammar. The real logic lives in the parser and execution engine.
