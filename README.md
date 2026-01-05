# SIL
***SIL referes to Structured Integer Language***; is a prototype interpreted language implemented in C, designed to explore lexer, parser, and recursive expression evaluation techniques.

## Motivation to start the Project

SIL was built as an experimental project to understand how interpreters work internally — including reading input, lexing, parsing, control-flow dispatch, and recursive expression evaluation. 
The project serves as a proof-of-concept for a weight-based recursive descent parsing model that is later intended to be used in a larger DSL called LAMDA.

## Language Features

- Integer-only language
- Operator precedence and parentheses support
- Variables and assignment
- Conditional statements (`if`)
- Loops (`for`, `while` with guard`)
- Interactive shell and file execution mode
- Native integer input and output, and string output statements

## Example Program

```sil
put "welcome to this Program (fully written in Structured Integer Language)\n"
let x
put "enter number of terms of fibonacci sequence you want to print on the screem: "
get x

if x < 1 {
	put "INVALID INPUT\n\a"
	stop
}

let firstNum @any variable is always initialised to 0 automatically at decleration@
let secondNum
let thirdNum
secondNum = 1; thirdNum = 1;

for x {
	put firstNum, ", "
	firstNum = secondNum;
	secondNum = thirdNum;
	thirdNum = firstNum + secondNum
}

put "\b\b  \n"
```
## Limitations

- Integer-only arithmetic
- No floating-point or character support
- No array, functions, structres support
- Minimal error handling
- Prototype-level code (not production hardened)

## Relation to LAMDA

SIL is a prototype language created to validate parsing and execution strategies that will later be used in LAMDA — a domain-specific language for large-scale linear algebra computations.

link to LAMDA repository - <u>https://github.com/AKR-code/LAMDA</u>
