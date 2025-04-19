#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/refobj.h>
#include <emerald/token.h>

/* token type names */
static const char *typenames[EM_TOKEN_TYPE_COUNT] = {
	"(None)",
	"INT",
	"FLOAT",
	"IDENTIFIER",
	"KEYWORD",
	"STRING",
	"PLUS",
	"MINUS",
	"ASTERISK",
	"SLASH",
	"OPEN_PAREN",
	"CLOSE_PAREN",
	"OPEN_SQUARE_BRACKET",
	"CLOSE_SQUARE_BRACKET",
	"COMMA",
	"DOT",
	"EQUALS",
	"LESS_THAN",
	"GREATER_THAN",
	"DOUBLE_EQUALS",
	"NOT_EQUALS",
	"LESS_THAN_EQUALS",
	"GREATER_THAN_EQUALS",
	"BITWISE_LEFT_SHIFT",
	"BITWISE_RIGHT_SHIFT",
	"BITWISE_AND",
	"BITWISE_OR",
	"BITWISE_NOT",
	"EOF",
};

/* get name of token type */
EM_API const char *em_get_token_type_name(em_token_type_t type) {

	if (type < 1 || type >= EM_TOKEN_TYPE_COUNT)
		return NULL;
	return typenames[type];
}

/* create token */
EM_API em_token_t *em_token_new(em_token_type_t type, em_pos_t *pos, const char *value, size_t len) {

	em_token_t *token = EM_TOKEN(em_refobj_new(&em_reflist_token, sizeof(em_token_t)+len+1, EM_CLEANUP_MODE_IMMEDIATE));
	if (!token) return NULL;

	token->type = type;
	memcpy(&token->pos, pos, sizeof(token->pos));
	if (value) memcpy(token->value, value, len);
	else memset(token->value, 0, len);
	token->value[len] = 0;

	return token;
}

/* print token list (debug) */
EM_API void em_token_print(em_token_t *token) {

	em_log_info("%s:%s", em_get_token_type_name(token->type), token->value);
}
