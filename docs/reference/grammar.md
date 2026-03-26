# Grammar Reference

This page summarizes the formal grammar of the Lux language, derived from the ANTLR4 grammar files (`LuxLexer.g4` and `LuxParser.g4`).

---

## Program Structure

```
program       → namespaceDecl? useDecl* includeDecl* topLevelDecl* EOF
namespaceDecl → 'namespace' IDENTIFIER ';'
```

### Imports

```
useDecl → 'use' modulePath '::' IDENTIFIER ';'
        | 'use' modulePath '::' '{' IDENTIFIER (',' IDENTIFIER)* '}' ';'

modulePath → IDENTIFIER ('::' IDENTIFIER)*
```

### Includes

```
includeDecl → '#include' '<' header '>'
            | '#include' '"' header '"'
```

---

## Top-Level Declarations

```
topLevelDecl → typeAliasDecl
             | structDecl
             | unionDecl
             | enumDecl
             | extendDecl
             | externDecl
             | functionDecl
```

### Type Alias

```
typeAliasDecl → 'type' IDENTIFIER '=' typeSpec ';'
```

```tm
type BinOp = fn(int32, int32) -> int32;
type Byte = uint8;
```

### Struct

```
structDecl → 'struct' IDENTIFIER '{' structField* '}'
structField → typeSpec IDENTIFIER ';'
```

```tm
struct Point {
    float64 x;
    float64 y;
}
```

### Union

```
unionDecl → 'union' IDENTIFIER '{' unionField* '}'
unionField → typeSpec IDENTIFIER ';'
```

```tm
union Value {
    int32 i;
    float32 f;
    *void ptr;
}
```

### Enum

```
enumDecl → 'enum' IDENTIFIER '{' IDENTIFIER (',' IDENTIFIER)* ','? '}'
```

```tm
enum Color { Red, Green, Blue }
```

### Extern

```
externDecl → 'extern' typeSpec IDENTIFIER '(' externParamList? (',' '...')? ')' ';'
externParamList → externParam (',' externParam)*
externParam → typeSpec IDENTIFIER?
```

```tm
extern int32 printf(*char fmt, ...);
extern void exit(int32 code);
```

### Function

```
functionDecl → typeSpec IDENTIFIER '(' paramList? ')' block
paramList → param (',' param)*
param → typeSpec '...' IDENTIFIER    // variadic
      | typeSpec IDENTIFIER          // normal
```

```tm
int32 main() {
    ret 0;
}

int32 add(int32 a, int32 b) {
    ret a + b;
}
```

### Extend

```
extendDecl → 'extend' IDENTIFIER '{' extendMethod* '}'
extendMethod → typeSpec IDENTIFIER '(' '&' IDENTIFIER (',' param)* ')' block
             | typeSpec IDENTIFIER '(' paramList? ')' block
```

```tm
extend Point {
    float64 distance(&self) {
        ret sqrt(self.x * self.x + self.y * self.y);
    }
}
```

---

## Statements

```
statement → varDeclStmt | assignStmt | compoundAssignStmt
          | derefAssignStmt | fieldAssignStmt
          | arrowAssignStmt | arrowCompoundAssignStmt
          | callStmt | exprStmt | returnStmt
          | ifStmt | forStmt | loopStmt | whileStmt | doWhileStmt
          | breakStmt | continueStmt | switchStmt
          | lockStmt | tryCatchStmt | throwStmt | deferStmt
```

### Variable Declaration

```
varDeclStmt → typeSpec IDENTIFIER '=' expression ';'
            | typeSpec IDENTIFIER ';'
```

```tm
int32 x = 42;
string name;
```

### Assignment

```
assignStmt → IDENTIFIER ('[' expression ']')* '=' expression ';'
compoundAssignStmt → IDENTIFIER op expression ';'
fieldAssignStmt → IDENTIFIER ('.' IDENTIFIER)+ '=' expression ';'
derefAssignStmt → '*' IDENTIFIER '=' expression ';'
                | '*' '(' expression ')' '=' expression ';'
arrowAssignStmt → IDENTIFIER '->' IDENTIFIER '=' expression ';'
arrowCompoundAssignStmt → IDENTIFIER '->' IDENTIFIER op expression ';'
```

Compound operators: `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `<<=` `>>=`

```tm
x = 10;
arr[0] = 5;
x += 1;
p.x = 3.0;
*ptr = 42;
node->value = 10;
```

### Return

```
returnStmt → 'ret' expression ';'
```

### If / Else

```
ifStmt → 'if' ('(' expression ')' | expression) block elseIfClause* elseClause?
elseIfClause → 'else' 'if' ('(' expression ')' | expression) block
elseClause → 'else' block
```

Parentheses around the condition are optional.

```tm
if x > 0 {
    println("positive");
} else if (x == 0) {
    println("zero");
} else {
    println("negative");
}
```

### For

```
forStmt → 'for' typeSpec IDENTIFIER 'in' expression block
        | 'for' typeSpec IDENTIFIER '=' expression ';' expression ';' expression block
```

```tm
for int32 x in arr { println(x); }
for int32 i in 0..10 { println(i); }
for int32 i = 0; i < 10; i++ { println(i); }
```

### While / Do-While / Loop

```
whileStmt → 'while' ('(' expression ')' | expression) block
doWhileStmt → 'do' block 'while' ('(' expression ')' | expression) ';'
loopStmt → 'loop' block
```

```tm
while x > 0 { x -= 1; }
do { x += 1; } while x < 10;
loop { if done { break; } }
```

### Switch

```
switchStmt → 'switch' ('(' expression ')' | expression) '{' caseClause* defaultClause? '}'
caseClause → 'case' expression (',' expression)* block
defaultClause → 'default' block
```

```tm
switch x {
    case 1 { println("one"); }
    case 2, 3 { println("two or three"); }
    default { println("other"); }
}
```

### Try / Catch / Finally

```
tryCatchStmt → 'try' block catchClause+ finallyClause?
             | 'try' block finallyClause
catchClause → 'catch' '(' typeSpec IDENTIFIER ')' block
finallyClause → 'finally' block
```

```tm
try {
    riskyOperation();
} catch (Error e) {
    println(e.message);
} finally {
    cleanup();
}
```

### Other Statements

```
throwStmt → 'throw' expression ';'
deferStmt → 'defer' callStmt | 'defer' exprStmt
breakStmt → 'break' ';'
continueStmt → 'continue' ';'
lockStmt → 'lock' '(' expression ')' block
callStmt → IDENTIFIER '(' argList? ')' ';'
exprStmt → expression ';'
```

---

## Expressions

Expressions listed from highest to lowest precedence. See [Operator Precedence](operator-precedence.md) for a full table.

### Postfix (highest)

```
expression '.' IDENTIFIER '(' argList? ')'     // method call
expression '(' argList? ')'                     // function call
expression '.' IDENTIFIER                       // field access
expression '->' IDENTIFIER                      // arrow access
expression '[' expression ']'                   // index
expression 'as' typeSpec                        // cast
expression 'is' typeSpec                        // type check
expression '++'                                 // post-increment
expression '--'                                 // post-decrement
```

### Special Syntax

```
IDENTIFIER '{' (IDENTIFIER ':' expression (',' IDENTIFIER ':' expression)*)? '}'  // struct literal
IDENTIFIER '::' IDENTIFIER '(' argList? ')'                                        // static method
IDENTIFIER '::' IDENTIFIER                                                         // enum access
```

### Unary Prefix

```
'*' expression              // dereference
'&' IDENTIFIER              // address-of
'-' expression              // negation
'spawn' expression           // spawn task
'await' expression           // await task
'!' expression              // logical NOT
'~' expression              // bitwise NOT
'++' expression             // pre-increment
'--' expression             // pre-decrement
'sizeof' '(' typeSpec ')'   // size in bytes
'typeof' '(' expression ')' // type name
```

### Binary (highest to lowest)

```
* / %                        // multiplicative
+ -                          // additive
<< >>                        // shift
< > <= >=                    // relational
== !=                        // equality
&                            // bitwise AND
^                            // bitwise XOR
|                            // bitwise OR
&&                           // logical AND
||                           // logical OR
??                           // null coalescing
.. ..=                       // range
? :                          // ternary
```

### Try Expression

```
'try' expression             // lowest precedence before atoms
```

### Atoms

```
'(' expression ')'                                          // grouping
'...' expression                                             // spread
'[' expression '|' 'for' typeSpec IDENTIFIER 'in' expression ('if' expression)? ']'  // list comprehension
'[' (expression (',' expression)*)? ']'                      // array literal
'null'                                                       // null literal
INT_LIT | FLOAT_LIT | BOOL_LIT | CHAR_LIT | STR_LIT | C_STR_LIT | IDENTIFIER
```

---

## Type Specifiers

```
typeSpec → '*' typeSpec                                        // pointer: *int32
         | '[' INT_LIT ']' typeSpec                            // fixed array: [10]int32
         | '[]' typeSpec                                       // dynamic array: []int32
         | fnTypeSpec                                          // function type
         | IDENTIFIER '<' typeSpec (',' typeSpec)* '>'         // generic: Vec<int32>
         | primitiveType                                       // int32, bool, etc.
         | IDENTIFIER                                          // user-defined: Point

fnTypeSpec → 'fn' '(' (typeSpec (',' typeSpec)*)? ')' '->' typeSpec
```

---

## Lexical Elements

### Literals

| Literal | Pattern | Examples |
|---------|---------|----------|
| Integer | `[0-9]+` | `42`, `0`, `1000000` |
| Float | `[0-9]+.[0-9]+` or scientific | `3.14`, `.5`, `1e10`, `2.5e-3` |
| Bool | `true` \| `false` | `true`, `false` |
| Char | `'c'` with escapes | `'a'`, `'\n'`, `'\x41'` |
| String | `"..."` with escapes | `"hello"`, `"line\n"` |
| C String | `c"..."` with escapes | `c"hello %s\n"` |
| Null | `null` | `null` |

### Character Escapes

| Escape | Meaning |
|--------|---------|
| `\n` | Newline |
| `\r` | Carriage return |
| `\t` | Tab |
| `\\` | Backslash |
| `\'` | Single quote |
| `\"` | Double quote |
| `\0` | Null byte |
| `\a` | Bell |
| `\b` | Backspace |
| `\f` | Form feed |
| `\v` | Vertical tab |
| `\xHH` | Hex byte (e.g., `\x41` = `'A'`) |

### Comments

```tm
// Line comment

/* Block
   comment */
```

### Identifiers

```
IDENTIFIER → [a-zA-Z_][a-zA-Z0-9_]*
```
