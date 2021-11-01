/*
   Author: ilittlebig <elias@jamee.se>
   Last Modified: 01/11/2021

   I chose to take a less popular route, and I will not be
   freeing any memory because lexers are short-lived
   programs.

   Freeing memory would only make the code more difficult
   to comprehend, which would be counterproductive in
   this case.

   TODO: Implement a Deterministic State Machine to make the lexer
   faster. Tokens should be parsed before using token-specific
   code, similar to how flex handles it.

   A lookup table should be implemented to prevent a large
   number of operations per character.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

/* Return true if the strings prefix matches the
   given prefix. */

static int starts_with(const char* str, const char* prefix, const int pos) {
	for (int i = 0; i < strlen(prefix); ++i) {
		if (str[pos + i] != prefix[i]) {
			return 0;
		}
	}
	return 1;
}

/* Return true if the string has N amount of
   characters left at the given position. */

static int has_at_least(const char* str, const int n) {
	return tokenizer->pos + n < strlen(str);
}

/* Return true if the input stream has no more
   available characters. */

static int is_eof() {
	return !has_at_least(tokenizer->input, 0);
}

/* Checks if the identifier starts with a valid
   character.

   An identifier can start with:
       any alphabetic char
       '_'

   A number is not available in this case. */

static int is_valid_ident_start() {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || c == '_') {
		return 1;
	}
	return 0;
}

/* Matches the first character of the word, and
   if the character matches the rules for what
   an identifier is defined as, return 1.

   An identifier can have:
       any alphabetic char
       any numeric char
       '_'

   Anything that does not follow these rules, should
   not be treated as an identifier. */

static int is_valid_ident() {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || isdigit(c) || c == '_') {
		return 1;
	}
	return 0;
}

/* Compares the string pointed to, by str1 to the string
   pointed to by str2. */

static int compare(const char* str1, const char* str2) {
	if (strcmp(str1, str2) == 0) {
		return 1;
	}
	return 0;
}

char* token_to_str(const token_type_t type) {
	switch (type) {
		case LOCAL:    return "local";
		case IF:       return "if";
		case IN:       return "in";
		case NIL:      return "nil";
		case NOT:      return "not";
		case REPEAT:   return "repeat";
		case OR:       return "or";
		case THEN:     return "then";
		case TRUE:     return "true";
		case WHILE:    return "while";
		case UNTIL:    return "until";
		case RETURN:   return "return";
		case AND:      return "and";
		case GOTO:     return "goto";
		case FUNCTION: return "function";
		case END:      return "end";
		case FALSE:    return "false";
		case FOR:      return "for";
		case ELSE:     return "else";
		case ELSEIF:   return "elseif";
		case DO:       return "do";
		case BREAK:    return "break";

		case INT:                     return "int";
		case FLOAT:                   return "float";
		case STRING_LITERAL:          return "string_literal";
		case UNCLOSED_STRING_LITERAL: return "unclosed_string_literal";
		case IDENT:                   return "ident";

		case ASSIGN:     return "assign";
		case GREATER_EQ: return "greater_eq";
		case LESS_EQ:    return "less_eq";
		case EQ:         return "eq";
		case NOT_EQ:     return "not_eq";
		case GREATER:    return "greater";
		case LESS:       return "less";

		case ADD: return "add";
		case SUB: return "sub";
		case MUL: return "mul";
		case DIV: return "div";
		case MOD: return "mod";
		case POW: return "pow";
		case LEN: return "len";

		case SHORT_COMMENT: return "short_comment";
		case LONG_COMMENT:  return "long_comment";

		case LEFT_PAREN:    return "left_paren";
		case RIGHT_PAREN:   return "right_paren";
		case LEFT_BRACKET:  return "left_bracket";
		case RIGHT_BRACKET: return "right_bracket";
		case LEFT_CURLY:    return "left_curly";
		case RIGHT_CURLY:   return "right_curly";

		case CONCAT:       return "concat";
		case DOTS:         return "dots";
		case DOUBLE_COLON: return "double_colon";
		case COLON:        return "colon";
		case COMMA:        return "comma";
		case SEMICOLON:    return "semicolon";
		case ATTR:         return "attr";
		case WHITESPACE:   return "whitespace";
		case UNIDENTIFIED: return "unidentified";
		case END_OF_FILE:  return "end_of_file";
	}
}

static token_type_t keyword_to_token_type(const char* keyword) {
	if (compare(keyword, "and")) {
		return AND;
	} else if (compare(keyword, "goto")) {
		return GOTO;
	} else if (compare(keyword, "function")) {
		return FUNCTION;
	} else if (compare(keyword, "end")) {
		return END;
	} else if (compare(keyword, "false")) {
		return FALSE;
	} else if (compare(keyword, "for")) {
		return FOR;
	} else if (compare(keyword, "else")) {
		return ELSE;
	} else if (compare(keyword, "elseif")) {
		return ELSEIF;
	} else if (compare(keyword, "do")) {
		return DO;
	} else if (compare(keyword, "break")) {
		return BREAK;
	} else if (compare(keyword, "local")) {
		return LOCAL;
	} else if (compare(keyword, "if")) {
		return IF;
	} else if (compare(keyword, "in")) {
		return IN;
	} else if (compare(keyword, "nil")) {
		return NIL;
	} else if (compare(keyword, "not")) {
		return NOT;
	} else if (compare(keyword, "repeat")) {
		return REPEAT;
	} else if (compare(keyword, "or")) {
		return OR;
	} else if (compare(keyword, "then")) {
		return THEN;
	} else if (compare(keyword, "true")) {
		return TRUE;
	} else if (compare(keyword, "while")) {
		return WHILE;
	} else if (compare(keyword, "until")) {
		return UNTIL;
	} else if (compare(keyword, "return")) {
		return RETURN;
	}
	return IDENT;
}

static token_t* read_digit() {
	token_t* token = malloc(sizeof(token_t));
	char* digit = malloc(sizeof(char));
	char* input = tokenizer->input;

	int is_float = 0;
	int is_hex = 0;

	size_t size = 0;

	if (starts_with(input, "0x", tokenizer->pos) || starts_with(input, "0X", tokenizer->pos)) {
		is_hex = 1;
		digit = realloc(digit, ++size + 1);
		digit[size-1] = input[tokenizer->pos++];
		digit = realloc(digit, ++size + 1);
		digit[size-1] = input[tokenizer->pos++];
	}

	for (;;) {
		const char c = input[tokenizer->pos];
		if (c == '.') {
			is_float = 1;
			digit = realloc(digit, ++size + 1);
			digit[size-1] = c;
		} else if (c == '-' || isdigit(c)) {
			digit = realloc(digit, ++size + 1);
			digit[size-1] = c;
		} else if (c == 'e' || c == 'E') {
			digit = realloc(digit, ++size + 1);
			digit[size-1] = c;

			if (c == '-' || c == '+') {
				digit = realloc(digit, ++size + 1);
				digit[size-1] = c;
			}
		} else if (is_hex && isxdigit(c)) {
			digit = realloc(digit, ++size + 1);
			digit[size-1] = c;
		} else {
			break;
		}
		tokenizer->pos++;
	}
	digit[size] = '\0';

	if (is_float) {
		token->type = FLOAT;
	} else {
		token->type = INT;
	}
	token->value = digit;
	return token;
}

/* Read strings that are delimited by matching.
   qoutations marks "...". These strings may
   only run for one line. */

static token_t* read_short_string() {
	token_t* token = malloc(sizeof(token_t));
	char* string = malloc(sizeof(char));

	char* input = tokenizer->input;
	const char start = input[tokenizer->pos++];

	size_t size = 0;
	int is_closed = 0;

	while (!is_closed && has_at_least(input, 0)) {
		const char c = input[tokenizer->pos];
		if (c == '\\') {
			string = realloc(string, ++size + 1);
			string[size-1] = c;
			tokenizer->pos++;
		}

		if (c != start) {
			string = realloc(string, ++size + 1);
			string[size-1] = c;
		} else if (c == start) {
			is_closed = 1;
		}
		tokenizer->pos++;
	}
	string[size] = '\0';

	if (is_closed) {
		token->type = STRING_LITERAL;
	} else {
		token->type = UNCLOSED_STRING_LITERAL;
	}

	token->value = string;
	return token;
}

/* Read strings that can be delimited by matching
   double square brackets [[...]]. These strings
   may run for serveral lines.

   They do not interpret escape sequences. */

static token_t* read_long_string() {
	token_t* token = malloc(sizeof(token_t));
	char* string = malloc(sizeof(char));
	char* input = tokenizer->input;

	size_t size = 0;
	int is_closed = 0;

	tokenizer->pos += 2;

	while (!is_closed && has_at_least(input, 0)) {
		const char c1 = input[tokenizer->pos];
		const char c2 = input[tokenizer->pos + 1];

		if (c1 != ']' || c2 != ']') {
			string = realloc(string, ++size + 1);
			string[size-1] = c1;
			tokenizer->pos++;
		} else {
			is_closed = 1;
			tokenizer->pos += 2;
		}
	}
	string[size] = '\0';

	if (is_closed) {
		token->type = STRING_LITERAL;
	} else {
		token->type = UNCLOSED_STRING_LITERAL;
	}

	token->value = string;
	return token;
}

/* Read comments that are delimited by the --
   character sequence. These comments may only
   run for one line.  */

static token_t* read_short_comment() {
	token_t* token = malloc(sizeof(token_t));
	char* comment = malloc(sizeof(char));

	char* input = tokenizer->input;
	size_t size = 0;

	while (input[tokenizer->pos] != '\n' && has_at_least(input, 0)) {
		const char c = input[tokenizer->pos++];
		comment = realloc(comment, ++size + 1);
		comment[size-1] = c;
	}
	comment[size] = '\0';

	token->type = SHORT_COMMENT;
	token->value = comment;
	return token;
}

/* Read comments that are delimited by the --[[
   character sequence. These comments may run for
   serveral lines.  */

static token_t* read_long_comment() {
	token_t* token = malloc(sizeof(token_t));
	char* comment = malloc(sizeof(char));

	char* input = tokenizer->input;
	size_t size = 0;

	while (!starts_with(input, "]]", tokenizer->pos) && has_at_least(input, 0)) {
		const char c = input[tokenizer->pos++];
		comment = realloc(comment, ++size + 1);
		comment[size-1] = c;
	}

	size += 2;
	comment = realloc(comment, size);
	comment[size-1] = ']';
	comment[size-2] = ']';
	comment[size] = '\0';

	token->type = LONG_COMMENT;
	token->value = comment;
	return token;
}

/* Matches a prefix to the input. The position
   is incremented, and one of two token types
   can be returned.

   Which type is returned depends on if the prefix
   matches the input's current prefix at its current
   position. */

static token_type_t get_matching_token_type(const char* str, token_type_t true_token, token_type_t false_token) {
	tokenizer->pos += strlen(str);
	if (starts_with(tokenizer->input, str, tokenizer->pos - strlen(str))) {
		return true_token;
	}
	return false_token;
}

/* Read more complex tokens such as tokens with
   multiple characters. */

static token_t* read_other_tokens() {
	token_t* token = malloc(sizeof(token_t));
	token->value = NULL;

	const char c = tokenizer->input[tokenizer->pos];
	switch (c) {
		case '~':
			token->type = get_matching_token_type("~=", NOT_EQ, UNIDENTIFIED);
			break;
		case '>':
			token->type = get_matching_token_type(">=", GREATER_EQ, GREATER);
			break;
		case '<':
			token->type = get_matching_token_type("<=", LESS_EQ, LESS);
			break;
		case '=':
			token->type = get_matching_token_type("==", EQ, ASSIGN);
			break;
		case ':':
			token->type = get_matching_token_type("::", DOUBLE_COLON, COLON);
			break;
		case '-':
			if (starts_with(tokenizer->input, "--[[", tokenizer->pos)) {
				return read_long_comment();
			} else if (starts_with(tokenizer->input, "--", tokenizer->pos)) {
				return read_short_comment();
			} else {
				if (isdigit(tokenizer->input[tokenizer->pos + 1])) {
					return read_digit();
				}
				tokenizer->pos++;
				token->type = SUB;
			}
			break;
		case '.':
			if (isdigit(tokenizer->input[tokenizer->pos + 1])) {
				return read_digit();
			} else {
				tokenizer->pos++;
				token->type = ATTR;
			}
			break;
		case '[':
			if (starts_with(tokenizer->input, "[[", tokenizer->pos)) {
				return read_long_string();
			} else {
				tokenizer->pos++;
				token->type = LEFT_BRACKET;
			}
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (is_valid_ident_start()) {
		char* word = malloc(sizeof(char));
		size_t size = 0;

		while (is_valid_ident()) {
			word = realloc(word, ++size + 1);
			word[size-1] = tokenizer->input[tokenizer->pos++];
		}
		word[size] = '\0';

		token_type_t token_type = keyword_to_token_type(word);
		token->type = token_type;
		if (token_type == IDENT) {
			token->value = word;
		}
	}
	return token;
}

/* Read less complex tokens such as tokens with
   only 1 character. */

static token_t* next_token() {
	token_t* token = malloc(sizeof(token_t));
	token->value = NULL;

	const char c = tokenizer->input[tokenizer->pos];
	switch (c) {
		case '\'':
		case '\"':
			return read_short_string();
		case '(':
			token->type = LEFT_PAREN;
			break;
		case ')':
			token->type = RIGHT_PAREN;
			break;
		case ']':
			token->type = RIGHT_BRACKET;
			break;
		case '{':
			token->type = LEFT_CURLY;
			break;
		case '}':
			token->type = RIGHT_CURLY;
			break;
		case ';':
			token->type = SEMICOLON;
			break;
		case ',':
			token->type = COMMA;
			break;
		case '+':
			token->type = ADD;
			break;
		case '*':
			token->type = MUL;
			break;
		case '/':
			token->type = DIV;
			break;
		case '%':
			token->type = MOD;
			break;
		case '^':
			token->type = POW;
			break;
		case '#':
			token->type = LEN;
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (isdigit(c)) {
		return read_digit();
	} else if (isspace(c) != 0) {
		token->type = WHITESPACE;
	}

	if (token->type == UNIDENTIFIED) {
		return read_other_tokens();
	} else {
		tokenizer->pos++;
	}
	return token;
}

token_t** tokenize(const char* input) {
	tokenizer = malloc(sizeof(struct tokenizer_t));
	tokenizer->input = strdup(input);
	tokenizer->pos = 0;

	token_t** tokens = malloc(sizeof(token_t));
	size_t size = 0;

	while (!is_eof()) {
		token_t* token = next_token();
		if (token->type != WHITESPACE) {
			size++;
			tokens = realloc(tokens, size * sizeof(token_t));
			tokens[size - 1] = token;
		}
	}

	token_t* token = malloc(sizeof(token_t));
	token->type = END_OF_FILE;

	tokens = realloc(tokens, size * sizeof(token_t));
	tokens[size] = token;
	return tokens;
}
