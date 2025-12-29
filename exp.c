#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 256

// Token types
typedef enum
{
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct
{
    TokenType type;
    int value;    // For numbers
    char op;      // For operators
    int weight;   // Precedence weight
    int position; // Position in input string
} Token;

// Token array structure with start and end indices
typedef struct
{
    Token *tokens; // Pointer to token array
    int start;     // Start index for parsing
    int end;       // End index for parsing
    int capacity;  // Total capacity
} TokenArray;

// Function prototypes
TokenArray *create_token_array(int capacity);
void free_token_array(TokenArray *arr);
const char *token_type_name(TokenType type);
void print_token(Token *token);
void print_all_tokens(TokenArray *arr);
bool lexer(const char *input, TokenArray *arr);
int parser(TokenArray *arr);

// Create token array
TokenArray *create_token_array(int capacity)
{
    TokenArray *arr = (TokenArray *)malloc(sizeof(TokenArray));
    arr->tokens = (Token *)malloc(sizeof(Token) * capacity);
    arr->start = 0;
    arr->end = 0;
    arr->capacity = capacity;
    return arr;
}

// Free token array
void free_token_array(TokenArray *arr)
{
    if (arr)
    {
        free(arr->tokens);
        free(arr);
    }
}

// Get token type name for display
const char *token_type_name(TokenType type)
{
    switch (type)
    {
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_END:
        return "END";
    case TOKEN_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

// Print single token
void print_token(Token *token)
{
    printf("  [%d] %-10s  ", token->position, token_type_name(token->type));

    switch (token->type)
    {
    case TOKEN_NUMBER:
        printf("value: %-5d  ", token->value);
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
        printf("op: '%c'      ", token->op);
        break;
    default:
        printf("            ");
        break;
    }

    printf("weight: %d\n", token->weight);
}

// Print all tokens in array
void print_all_tokens(TokenArray *arr)
{
    printf("\n=== TOKENS (start: %d, end: %d) ===\n", arr->start, arr->end);
    printf("  Pos  Type        Value      Weight\n");
    printf("  ─────────────────────────────────────\n");

    for (int i = arr->start; i < arr->end; i++)
    {
        print_token(&arr->tokens[i]);
    }
    printf("\n");
}

// Get base weight for operator
int get_base_weight(char op)
{
    switch (op)
    {
    case '+':
    case '-':
        return 1; // Lower precedence
    case '*':
    case '/':
        return 2; // Higher precedence
    default:
        return 0;
    }
}

// Lexer - tokenizes input and assigns weights
bool lexer(const char *input, TokenArray *arr)
{
    int pos = 0;
    int token_count = 0;
    int paren_depth = 0; // Track parenthesis nesting level
    int load = 0;        // Additional weight from parentheses

    while (input[pos] != '\0' && token_count < arr->capacity)
    {
        // Skip whitespace
        if (isspace(input[pos]))
        {
            pos++;
            continue;
        }

        Token *token = &arr->tokens[token_count];
        token->position = pos;
        token->weight = 0;
        token->value = 0;
        token->op = '\0';

        // Parse number
        if (isdigit(input[pos]))
        {
            int num = 0;
            while (isdigit(input[pos]))
            {
                num = num * 10 + (input[pos] - '0');
                pos++;
            }
            token->type = TOKEN_NUMBER;
            token->value = num;
            token->weight = 0; // Numbers don't have precedence weight
            token_count++;
        }
        // Parse operators
        else if (input[pos] == '+' || input[pos] == '-' ||
                 input[pos] == '*' || input[pos] == '/')
        {
            char op = input[pos];

            switch (op)
            {
            case '+':
                token->type = TOKEN_PLUS;
                break;
            case '-':
                token->type = TOKEN_MINUS;
                break;
            case '*':
                token->type = TOKEN_MULTIPLY;
                break;
            case '/':
                token->type = TOKEN_DIVIDE;
                break;
            }

            token->op = op;
            // Base weight + load from parentheses
            token->weight = get_base_weight(op) + load;
            pos++;
            token_count++;
        }
        // Parse left parenthesis
        else if (input[pos] == '(')
        {
            token->type = TOKEN_LPAREN;
            token->weight = 0;
            paren_depth++;
            load += 1000; // Increase load for operators inside parentheses
            pos++;
            token_count++;
        }
        // Parse right parenthesis
        else if (input[pos] == ')')
        {
            token->type = TOKEN_RPAREN;
            token->weight = 0;
            paren_depth--;
            load -= 1000; // Decrease load when exiting parentheses

            if (paren_depth < 0)
            {
                fprintf(stderr, "Error: Unmatched closing parenthesis at position %d\n", pos);
                return false;
            }
            pos++;
            token_count++;
        }
        // Unknown character
        else
        {
            fprintf(stderr, "Error: Invalid character '%c' at position %d\n", input[pos], pos);
            token->type = TOKEN_ERROR;
            return false;
        }
    }

    // Check for unmatched opening parentheses
    if (paren_depth != 0)
    {
        fprintf(stderr, "Error: Unmatched opening parenthesis\n");
        return false;
    }

    // Add END token
    if (token_count < arr->capacity)
    {
        arr->tokens[token_count].type = TOKEN_END;
        arr->tokens[token_count].position = pos;
        arr->tokens[token_count].weight = 0;
        token_count++;
    }

    arr->start = 0;
    arr->end = token_count;

    return true;
}

// Parser - returns int by recursively evaluating tokens using weights
// Does not modify token data; uses start/end views only
int parser(TokenArray *arr)
{
    // Echo the tokens received for this parse window
    printf("\n=== PARSER RECEIVED TOKENS ===\n");
    printf("Token array: start=%d, end=%d\n", arr->start, arr->end);
    print_all_tokens(arr);

    // Find the operator with the lowest weight in the current window
    int minWeightIndex = -1;
    for (int i = arr->start; i < arr->end; i++)
    {
        Token *token = &arr->tokens[i];
        if (token->type == TOKEN_PLUS || token->type == TOKEN_MINUS ||
            token->type == TOKEN_MULTIPLY || token->type == TOKEN_DIVIDE)
        {
            if (minWeightIndex == -1 || token->weight < arr->tokens[minWeightIndex].weight)
            {
                minWeightIndex = i;
            }
        }
    }

    // Base cases when no operator found in window
    if (minWeightIndex == -1)
    {
        // If the window is just a single number, return it
        for (int i = arr->start; i < arr->end; i++)
        {
            if (arr->tokens[i].type == TOKEN_NUMBER)
            {
                return arr->tokens[i].value;
            }
        }

        // If wrapped in parentheses, strip outermost and recurse
        if (arr->tokens[arr->start].type == TOKEN_LPAREN)
        {
            int depth = 0;
            int match = -1;
            for (int i = arr->start; i < arr->end; i++)
            {
                if (arr->tokens[i].type == TOKEN_LPAREN)
                    depth++;
                else if (arr->tokens[i].type == TOKEN_RPAREN)
                {
                    depth--;
                    if (depth == 0)
                    {
                        match = i;
                        break;
                    }
                }
            }
            if (match != -1 && match > arr->start + 1)
            {
                TokenArray inner;
                inner.tokens = arr->tokens;
                inner.capacity = arr->capacity;
                inner.start = arr->start + 1;
                inner.end = match;
                return parser(&inner);
            }
        }

        // Fallback: nothing evaluatable
        return 0;
    }

    // Build left and right views and recursively evaluate
    TokenArray leftView;
    leftView.tokens = arr->tokens;
    leftView.capacity = arr->capacity;
    leftView.start = arr->start;
    leftView.end = minWeightIndex;

    TokenArray rightView;
    rightView.tokens = arr->tokens;
    rightView.capacity = arr->capacity;
    rightView.start = minWeightIndex + 1;
    rightView.end = arr->end;

    int leftVal = parser(&leftView);
    int rightVal = parser(&rightView);

    // Apply the operator at minWeightIndex
    TokenType t = arr->tokens[minWeightIndex].type;
    switch (t)
    {
    case TOKEN_PLUS:
        return leftVal + rightVal;
    case TOKEN_MINUS:
        return leftVal - rightVal;
    case TOKEN_MULTIPLY:
        return leftVal * rightVal;
    case TOKEN_DIVIDE:
        if (rightVal == 0)
        {
            fprintf(stderr, "Error: Division by zero in parse window [%d,%d)\n", arr->start, arr->end);
            return 0;
        }
        return leftVal / rightVal;
    default:
        fprintf(stderr, "Error: Unknown operator encountered during parse\n");
        return 0;
    }
}

// REPL main loop
void repl()
{
    char input[MAX_INPUT];
    TokenArray *tokens = create_token_array(MAX_TOKENS);

    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Expression Parser REPL - Token Weight Experiment       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\nSupported operators: + - * /\n");
    printf("Supported syntax: numbers, operators, parentheses\n");
    printf("Example: 2 + (3 - 4) * 5\n");
    printf("\nWeight system:\n");
    printf("  - Base weights: + - (1), * / (2)\n");
    printf("  - Inside '()': +1000 per nesting level\n");
    printf("\nType 'exit' to quit\n");
    printf("════════════════════════════════════════════════════════════\n\n");

    while (true)
    {
        printf(">> ");
        fflush(stdout);

        // Read input
        if (fgets(input, MAX_INPUT, stdin) == NULL)
        {
            break;
        }

        // Remove trailing newline
        input[strcspn(input, "\n")] = '\0';

        // Check for exit command
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0)
        {
            printf("Goodbye!\n");
            break;
        }

        // Skip empty lines
        if (strlen(input) == 0)
        {
            continue;
        }

        // Lexer tokenizes input and sends to parser
        if (lexer(input, tokens))
        {
            // Send tokens to parser and print result
            int result = parser(tokens);
            printf("Result: %d\n\n", result);
        }
        else
        {
            printf("Tokenization failed. Please check your input.\n\n");
        }
    }

    free_token_array(tokens);
}

int main()
{
    repl();
    return 0;
}
