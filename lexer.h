#ifndef LEXER_H
#define LEXER_H

typedef enum {
	LOCAL, IF, IN, NIL, NOT,
	REPEAT, OR, THEN, TRUE, WHILE,
	UNTIL, RETURN, AND, GOTO, FUNCTION,
	END, FALSE, FOR, ELSE, ELSEIF,
	DO, BREAK,

	INT, FLOAT, STRING_LITERAL,
	UNCLOSED_STRING_LITERAL,
	IDENT,

	ASSIGN, GREATER_EQ, LESS_EQ,
	EQ, NOT_EQ, GREATER, LESS,
	SHIFT_LEFT, SHIFT_RIGHT,

	ADD, SUB, MUL, DIV,
	MOD, POW, LEN,

	SHORT_COMMENT,
	LONG_COMMENT,

	LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET,
	RIGHT_BRACKET, LEFT_CURLY, RIGHT_CURLY,

	CONCAT, DOTS, DOUBLE_COLON, COLON,
	COMMA, SEMICOLON, ATTR, WHITESPACE,
	UNIDENTIFIED, END_OF_FILE
} token_type_t;

typedef struct token_t {
	token_type_t type;
	char* value;
} token_t;

typedef struct tokenizer_t {
	char* input;
	size_t pos;
} tokenizer_t;

static tokenizer_t* tokenizer;

char* token_to_str(const token_type_t type);
token_t** tokenize(const char* input);

#endif /* LEXER_H */
