/*
   Author: ilittlebig <elias@jamee.se>
   Last Modified: 01/11/2021

   This file is used to demonstrate how the
   lexer could be used. This is in no shape
   or form, the best way to read input stream.

   It is not a dynamic solution, meaning it
   can only read files with a maximum size
   of 1,048,576 bytes.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"

#define BUFFER_SIZE 1 * 1024 * 1024
#define ITERATIONS 10 * 1024

char* read_file(const char* filename) {
	FILE *fp;
	char tmp[BUFFER_SIZE];

    fp = fopen(filename, "rb");
    for(int i = 0; i < ITERATIONS; ++i) {
        fread(tmp, BUFFER_SIZE, 1, fp);
    }
    fclose(fp);

	char* buffer = malloc(BUFFER_SIZE);
	memcpy(buffer, tmp, BUFFER_SIZE);
	return buffer;
}

int main() {
	const char* buffer = read_file("tests/tmp.lua");
	token_t** tokens = tokenize(buffer);

	size_t i = 0;
	while (tokens[i]->type != END_OF_FILE) {
		token_t* token = tokens[i++];
		if (token->value) {
			if (token->type == STRING_LITERAL) {
				printf("<type: %s | value: \"%s\">\n", token_to_str(token->type), token->value);
			} else {
				printf("<type: %s | value: %s>\n", token_to_str(token->type), token->value);
			}
		} else {
			printf("<type: %s>\n", token_to_str(token->type));
		}
	}
}
