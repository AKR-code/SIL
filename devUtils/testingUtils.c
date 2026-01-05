#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define EXPECTARG 2
#define VAR_LENGTH 15
typedef struct {
	int nan;
	int inf;
	int value;
	char name[15];
} VAR;

typedef enum {
	STP, KIL, LET, PUT, GET, CON, ELCON, FOR, TIL, END_BLOCK, ASN, // Explicit functions
	DVD, MLT, SUB, ADD,							 	//Explicit arthematic operators
	EQL, NTE, LES, GRT, LOE, GOE,					//Explicit relational operators
	AND, ORR, NOT,										//Explicit Short circuit operators
	VARIABLE, STRING, CONSTANT, END_STMT, GRD,
	TOTAL_TOKENS
} TOKEN_VAL;

typedef struct {
	char pattern[10];
	int weight;
} TOKEN;

typedef struct {
	TOKEN_VAL token;
	int weight;
	int index;
	int length;
	int constVal;
	VAR *var;
} TOKEN_ARR;

typedef struct {
	VAR *varTable;		//pointer to symbol table for storing user created variables
	char *string;		//pointer to buffer, where string read from file is stored
	TOKEN_ARR *tokenArr;//lexer generated array which stores tokens and references for parsing and executing
	int noOfVars;
	int lineNum;
	int noOfTokens;
	int tokenArrCap;
	int varTableCap;
} PSTAT;

#define EXITCHAR 0
#define NOISSUE 1
#define LEXE_RECYCLE 2
#define CONTCHAR 3
#define TRUE 4
#define FALSE 5
#define INIT_BUFF_SIZE 32
#undef INIT_BUFF_SIZE
#define INIT_BUFF_SIZE 64
#define GROWTH_FACTOR 2
#undef INIT_BUFF_SIZE 
#define INIT_BUFF_SIZE 32
#define INIT_STRING_SIZE 64


char parserTesting (PSTAT *info) {
	for (int i = 0; i < info->noOfTokens; i++) {
		printf("Token %d: ", i);
		switch (info->tokenArr[i].token) {
			case STP: printf("STOP\n"); break;
			case KIL: printf("KILL\n"); break;
			case LET: printf("LET\n"); break;
			case PUT: printf("PUT\n"); break;
			case GET: printf("GET\n"); break;
			case CON: printf("IF\n"); break;
			case ELCON: printf("ELIF\n"); break;
			case FOR: printf("FOR\n"); break;
			case TIL: printf("WHILE\n"); break;
			case ASN: printf("ASSIGNMENT\n"); break;
			case VARIABLE: printf("VARIABLE (%s)\n", info->tokenArr[i].var->name); break;
			case DVD: printf("DIVIDE\n"); break;
			case MLT: printf("MULTIPLY\n"); break;
			case SUB: printf("SUBTRACT\n"); break;
			case ADD: printf("ADD\n"); break;
			case EQL: printf("EQUALS\n"); break;
			case NTE: printf("NOT EQUALS\n"); break;
			case LES: printf("LESS THAN\n"); break;
			case GRT: printf("GREATER THAN\n"); break;
			case LOE: printf("LESS OR EQUALS\n"); break;
			case GOE: printf("GREATER OR EQUALS\n"); break;
			case AND: printf("AND\n"); break;
			case ORR: printf("OR\n"); break;
			case NOT: printf("NOT\n"); break;
			case CONSTANT: printf("CONSTANT (%d)\n", info->tokenArr[i].constVal); break;
			case STRING: 
				printf("STRING (");
				for (int j = 0; j < info->tokenArr[i].length; j++) {
					printf("%c", info->string[info->tokenArr[i].index + j]);
				}
				printf(")\n");
				break;
			case END_STMT: printf("END OF STATEMENT\n"); break;
			case END_BLOCK: printf("END OF BLOCK\n"); break;
			default: printf("OTHER TOKEN\n"); break;
		}
	}
	return NOISSUE;
}

typedef enum {
	ER_LONG_INPUT, ER_NO_INPUT, ER_WRONG_INPUT
} ER;

typedef enum {
	ER_LEAD_NUM, ER_SPC_CHAR, ER_WHT_SPA, ER_EXC_LEN, ER_USE_LOOP, ER_USE_CHAIN
} ERROR_TYPE_02;

typedef enum {
	ER_WANT_MRG, ER_EXTRA_MRG, ER_UNCLOSED_PAIR
} ER_PUT;

typedef enum {
	EXP_BLOCK, OPT_UNEXPECTED, OPT_NEEDED, MISSING_GDR
} ER_CON;

typedef enum {
	CALLER_IF, CALLER_FOR, CALLER_TIL
} CALLER;

typedef enum {
	EXPECTING_EXPR, TOO_MANY_CLO, OPERATOR_UNEXPECTED, OPERATOR_NEEDED, PAIR_CLO_ER
} ER_ASN;

typedef struct {
	char prog;
	VAR number;
	int isString;
	int start;
	int end;
} RES;

void debugLog (PSTAT *info, RES init) {
	printf("               +------+----------+-------------+\n");
	printf("               |%6s|%10s| WINDOW INFO |\n", "si.no", "TOKEN");
	printf("               +------+----------+-------------+\n");
	for (int x = 0; x < info->noOfTokens; x++) {
		printf("               |%6d", x);
		switch(info->tokenArr[x].token) {
			case STP: printf("|%10s|", " STP "); break;
			case KIL: printf("|%10s|", " KIL "); break;
			case LET: printf("|%10s|", " LET "); break;
			case PUT: printf("|%10s|", " PUT "); break;
			case GET: printf("|%10s|", " GET "); break;
			case CON: printf("|%10s|", " CON "); break;
			case ELCON: printf("|%10s|", " ELCON "); break;
			case FOR: printf("|%10s|", " FOR "); break;
			case TIL: printf("|%10s|", " TIL "); break;
			case ASN: printf("|%10s|", " ASN "); break;
			case DVD: printf("|%10s|", " DVD "); break;
			case MLT: printf("|%10s|", " MLT "); break;
			case SUB: printf("|%10s|", " SUB "); break;
			case ADD: printf("|%10s|", " ADD "); break;
			case EQL: printf("|%10s|", " EQL "); break;
			case NTE: printf("|%10s|", " NTE "); break;
			case LES: printf("|%10s|", " LES "); break;
			case GRT: printf("|%10s|", " GRT "); break;
			case LOE: printf("|%10s|", " LOE "); break;
			case GOE: printf("|%10s|", " GOE "); break;
			case AND: printf("|%10s|", " AND "); break;
			case ORR: printf("|%10s|", " ORR "); break;
			case NOT: printf("|%10s|", " NOT "); break;
			case VARIABLE: printf("|%10s|", " VARIABLE "); break;
			case STRING: printf("|%10s|", " STRING "); break;
			case CONSTANT: printf("|%10s|", " CONSTANT "); break;
			case END_STMT: printf("|%10s|", " END_STMT "); break;
			case END_BLOCK: printf("|%10s|", " END_BLOCK "); break;
			default : printf("\a ERROR UNKNOWN TOKEN\n");
		}
		if (x >= init.start && x < init.end) {
			printf(" TO BE PARSED|\n");
		} else {
			printf("             |\n");
		}
		printf("               +------+----------+-------------+\n");
	}
}

