# Structured Integer Language

iThis project is done by a sem 1 Btech student so obviously I dont know the conventional ways of building an interpreter or compiler, and also this is not a General Purpose Language. Hence I am going to opt unconventional easy to iplement simple algorithms for lexing and parsing. Certainly this will not be the most optimal approach. The aim of this project is more to learn rather than surving a real purpose or solving a problem

## Lexer and Parser Logic

### High level solution approach

We will be defining all the tokens and syntax in a token table with specific weight for each tokens. The Lexer scans through the preprocessed string given by the reader and searches for the matching pattern in token table. and generates a tokenArray of datatype tokens.

We can manage this interpreter because of the shallow grammar I designed earlier

all the statements in this SI Language can be classified into two types
1. Key word driven 
These are the statments which start with a keyword like
let x
put "hello world!"
get x
for 5 {
	x = x + 1
}
til x < 5 : 1000 {
	x = x + 1
}
con x < 5 {
	put x
} 

2. Non key word driven
These are assignment operations and }
x = 1
(y + 2) / (x * z + y) / z = c

Lexer progressively searches for the matching pattern accross the names from the collection of tokenTable and varTable. The first word must match with a pattern in token table or var table if it doesnt match it is definately a typo and error will be displayed and returned back, if there is any matching pair then we can predict next words or characters

for instance
let x

tokenTable.pattern = {'let ', 'put ', 'get ', 'for ', 'con ', 'til ', '}', '(', '['}
consider some random varTable.name = {'m', 'n', 'l', 'bad', 'good'}

lexer first sees the first letter l and it understands that it could be either let from tokenTable or variable name l
it reads further more and it gets e. Here clearly the only matching pair is let from tokenTable, but still it reads further more to confirm.
it reads, t, and space confirming it is token let in tokenTable. and adds the first token in token array LET.

it records all the characters upto next white space and puts it into the string field of LET token.
if string ends then lexer fills o in value field of token type, if there is = symbol then it reads further more and genrates assignment tokens

for instance
c = (y + 2) / (x * z + y) / z

and so on ... like this
```text
Token		VAR		OPR
weight		120		80 + 20
strng		"\0"
value		12
```

so that based on the weight of the token parser will do this called as Abstract Syntax Tree, where heaviest elements go down and lightest elements float on top

```text
		 =
	    / \
 	   c   A
		   |
	   __ /__
	  /		 \
	  B		 z
  	  |
   __ / __
  /		  \
  C	 	  D
  |		  |
  +		  +
 / \     / \
y  	2   D   y
  	    |
  	    *
 	   / \
	  x   z
```

i.e ASN(c, DVD(DVD(ADD(y, 2), ADD(MUL(x, z), y)), z))

for 
put "hello world!, "x" this is the number you entered

```text
		put
		 |
	   merge
	 /		 \
  merge		str2
/	   \
str1	x

	```
i.e PUT(MRG(MRG(str1, x), str2))

for the string given by the reader 
con condshn0 { stmt0 } condshn1 { stmt1 } { stmt 2 }
after tokenisation by lexer

conditions, and loops are implemented by one to one correspondance with C
i.e

```text
if (evaluate(condshn0)) {
	execute AST of stmt0
} else {
	call if-else executer again ----------> if (evaluate(condshn1)) {
}												execute AST of stmt1
											} else {
												execute AST of stmt2
											}
```
