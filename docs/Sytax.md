# SIL Syntax Documentation

## Key Words

in this language any statement starts with a key word, the possible key words are let, get, put, kill, stop, if, } elif, for, while, }, and <variable name>. It is highly important to know that in the SI Language atleast a single whitespace is mandatory after the key word.

- Below is the correct usage

```sil
let x
get		 x
put   x
x = 10
kill x
```
-Below is the incorrect usage

```sil
letx
getx
x= 10
```

In the SI Language new line character '\n' and semicolon ';' are both delimitters, but semicolon ';' should only be used while chaining two or more statements in a single line. This language supports chaining of multiple statements with semicolons and also commas with specific key wrords only

## let

-This starts with the key word 'let', atleast a single whitespace and then a string as per programmar's wish. 
-This statement creates a new variable of integer datatype where programmar can work with it. 
-This string will be used as the name for the variable newly created. 
-Name string of the variable cannot start with numbers, cannot have any special characters in any position of the string. Only lower and upper case alphabets and numbers at any position other than first char can be used
-Number of characters in nae string cannot exceed 14 
-you cannot chain let using semi colon with other statements but you can create multiple variables using comma

wrong usage

```sil
let x; let y
let a; put a
```

right usage

```sil
let x, y
let a
put a
```
-this language doesnt support variable scopes, so using let in blocks like if-elif, for, while is restricted
-you cannot create a new variable with the same name of an already existing variable

## put

-This starts with the key word 'put' followed by atleast a single whitespace.
-This statement prints data to standard output.
-Arguments to 'put' can be strings, variables, or expressions.
-Multiple arguments can be passed to 'put' separated by comma.
-Strings must be enclosed within double quotes.
-Evaluation of expressions happens before printing.
-Newline is not printed automatically unless explicitly specified using "\n".

wrong usage

```sil
put
put x y
put ["hello]
```

right usage

```sil
put "hello world"
put x
put "value is ", x, "\n"
put a + b * c
```
-put does not modify any variable or expression value

## get

-This starts with the key word 'get' followed by atleast a single whitespace.
-This statement reads an integer value from standard input.
-The input value is stored into the given variable.
-The variable must be declared using 'let' before using 'get'.
-Only integer input is supported.
-If input fails, behavior is implementation-defined.

wrong usage

```sil
get
get 10
get x + y
```

right usage

```sil
get x
```
-get overwrites the previous value stored in the variable
-get does not accept expressions or literals

## assignment

-Assignment uses the '=' operator.
-Left-hand side must be a variable name.
-Right-hand side must be an integer literal, variable, or expression.
-Expressions follow operator precedence and parentheses rules.
-Assignment evaluates the right-hand side fully before storing the value.

wrong usage

```sil
x + y = 10
10 = x
a = 
```

right usage

```sil
x = 10
y = x + 5
result = (a + b) * c
```
-Assignment does not create variables; variable must exist before assignment

## kill

-This starts with the key word 'kill' followed by atleast a single whitespace.
-This statement destroys an existing variable.
-After killing a variable, it cannot be accessed or used again.
-The variable must exist before calling 'kill'.
-Memory associated with the variable is released.

wrong usage

```sil
kill
kill 10
kill x + y
```

right usage

```sil
kill x
```
-Using a killed variable results in a runtime error
-kill cannot be used on undeclared variables

## stop

-This starts with the key word 'stop'.
-This statement immediately terminates program execution.
-No statements after 'stop' are executed.
-Control is returned to the operating system or shell.

wrong usage

```sil
stop x
stop 10
```

right usage

```sil
stop
```
-stop does not require any arguments
-stop can appear anywhere in the program

