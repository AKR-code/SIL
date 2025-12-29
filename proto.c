#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 128
#define MAX_LINE   256
#define PAREN_LOAD 1000

/* ---------------- Token definitions ---------------- */

typedef enum {
    TOK_NUM,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_INVALID,
    TOK_END
} TokenType;

typedef struct {
    TokenType type;
    int value;      // for numbers
    int weight;     // for operators only
} Token;

/* ---------------- Helpers ---------------- */

int base_weight(TokenType type) {
    switch (type) {
        case TOK_PLUS:
        case TOK_MINUS:
            return 10;
        case TOK_MUL:
        case TOK_DIV:
            return 20;
        default:
            return 0;
    }
}

/* ---------------- Lexer ---------------- */

int tokenize(const char *line, Token *tokens) {
    int pos = 0;
    int i = 0;
    int paren_depth = 0;

    while (line[pos] != '\0' && line[pos] != '\n') {

        if (isspace((unsigned char)line[pos])) {
            pos++;
            continue;
        }

        /* number */
        if (isdigit((unsigned char)line[pos])) {
            int val = 0;
            while (isdigit((unsigned char)line[pos])) {
                val = val * 10 + (line[pos] - '0');
                pos++;
            }
            tokens[i++] = (Token){ TOK_NUM, val, 0 };
            continue;
        }

        /* parentheses */
        if (line[pos] == '(') {
            paren_depth++;
            tokens[i++] = (Token){ TOK_LPAREN, 0, 0 };
            pos++;
            continue;
        }

        if (line[pos] == ')') {
            paren_depth--;
            tokens[i++] = (Token){ TOK_RPAREN, 0, 0 };
            pos++;
            continue;
        }

        /* operators */
        TokenType type;
        switch (line[pos]) {
            case '+': type = TOK_PLUS;  break;
            case '-': type = TOK_MINUS; break;
            case '*': type = TOK_MUL;   break;
            case '/': type = TOK_DIV;   break;
            default:
                tokens[i++] = (Token){ TOK_INVALID, line[pos], 0 };
                pos++;
                continue;
        }

        int w = base_weight(type) + (paren_depth * PAREN_LOAD);
        tokens[i++] = (Token){ type, 0, w };
        pos++;
    }

    tokens[i++] = (Token){ TOK_END, 0, 0 };
    return i;
}

/* ---------------- Parser (echo only) ---------------- */

int parse(Token *tokens, int start, int end) {
    int i = 0;
    while (tokens[i].type != TOK_END) {
        switch (tokens[i].type) {
            case TOK_NUM:
                printf("[NUM] %d\n", tokens[i].value);
                break;

            case TOK_PLUS:
                printf("[PLUS] weight=%d\n", tokens[i].weight);
                break;

            case TOK_MINUS:
                printf("[MINUS] weight=%d\n", tokens[i].weight);
                break;

            case TOK_MUL:
                printf("[MUL] weight=%d\n", tokens[i].weight);
                break;

            case TOK_DIV:
                printf("[DIV] weight=%d\n", tokens[i].weight);
                break;

            case TOK_LPAREN:
                printf("[LPAREN]\n");
                break;

            case TOK_RPAREN:
                printf("[RPAREN]\n");
                break;

            case TOK_INVALID:
                printf("[INVALID] '%c'\n", tokens[i].value);
                break;
        }
        i++;
    }

	int minWeightIndex = i;
	while (tokens[i].type != TOK_END) {
		//search for first minimum weight
		i++;
		if (token[minWeightIndex].weight > token[i].weight) minWeightIndex = i; 
	}

	if (minWeightIndex == )

	switch (token[minWeightIndex].type) {
		case TOK_PLUS: adder(tokens, minWeightIndex); break;
		case TOK_MINUS: minus(tokens, minWeightIndex); break;
		case TOK_MUL: multiplier(tokens, minWeightIndex); break;
		case TOK_DIV: divider(tokens, minWeightIndex); break;
	}
}

/* ---------------- REPL ---------------- */

int main(void) {
    char line[MAX_LINE];
    Token tokens[MAX_TOKENS];

    printf("Expression tokenizer (operator weights enabled)\n");
    printf("Ctrl+D to exit\n");

    while (1) {
        printf("expr> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
		
		int noOfTokens = 0;
        tokenize(line, tokens, &noOfTokens);
        parse(tokens, 0, noOfTokens);
    }

    return 0;
}

