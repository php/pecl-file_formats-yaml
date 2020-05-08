/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
 * Copyright (c) 2015 Bryan Davis and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @package     php_yaml
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @author      Bryan Davis <bd808@bd808.com>
 * @copyright   2007 Ryusuke SEKIYAMA
 * @copyright   2009 Keynetics Inc
 * @copyright   2015 Bryan Davis and contributors
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */


#include "php_yaml.h"
#include "php_yaml_int.h"

/* {{{ local macros
 */
#define Y_PARSER_CONTINUE 0
#define Y_PARSER_SUCCESS  1
#define Y_PARSER_FAILURE -1

#define Y_FILTER_NONE     0
#define Y_FILTER_SUCCESS  1
#define Y_FILTER_FAILURE -1

#define NEXT_EVENT() yaml_next_event(state)

#define COPY_EVENT(dest, state) \
	memcpy(&dest, &state->event, sizeof(yaml_event_t)); \
	state->have_event = 0; \
	memset(&state->event, 0, sizeof(yaml_event_t))
/* }}} */


/* {{{ local prototypes
 */
static void handle_parser_error(const yaml_parser_t *parser);

static inline int yaml_next_event(parser_state_t *state);

void get_next_element( parser_state_t *state, zval *retval);

void handle_document(parser_state_t *state, zval *retval);

void handle_mapping(parser_state_t *state, zval *retval);

void handle_sequence(parser_state_t *state, zval *retval);

void handle_scalar(parser_state_t *state, zval *retval);

void handle_alias(parser_state_t *state, zval *retval);

static zval *record_anchor_make_ref(zval *aliases, const char *anchor, zval *value);

static int apply_filter(
		zval *zp, yaml_event_t event, HashTable *callbacks);

static int eval_timestamp(zval **zpp, const char *ts, size_t ts_len);

/* }}} */


/* {{{ php_yaml_read_all()
 * Process events from yaml parser
 */
void php_yaml_read_all(parser_state_t *state, zend_long *ndocs, zval *retval)
{
	zval doc;
	int code = Y_PARSER_CONTINUE;

	/* create an empty array to hold results */
	array_init(retval);

	while (Y_PARSER_CONTINUE == code) {

		if (!NEXT_EVENT()) {
			code = Y_PARSER_FAILURE;
			break;

		} else if (YAML_STREAM_END_EVENT == state->event.type) {
			code = Y_PARSER_SUCCESS;
			break;

		} else if (YAML_STREAM_START_EVENT == state->event.type) {
			if (!NEXT_EVENT()) {
				code = Y_PARSER_FAILURE;
				break;
			}
			if (YAML_STREAM_END_EVENT == state->event.type) {
				/* entire stream consisted of an empty document */
				code = Y_PARSER_SUCCESS;
				ZVAL_NULL(&doc);
				add_next_index_zval(retval, &doc);
				(*ndocs)++;
				break;
			}

		}

		if (YAML_DOCUMENT_START_EVENT != state->event.type) {
			code = Y_PARSER_FAILURE;

			php_error_docref(NULL, E_WARNING,
					"expected DOCUMENT_START event, got %d "
					"(line %zd, column %zd)",
					state->event.type,
					state->parser.mark.line + 1,
					state->parser.mark.column + 1);
			break;
		}

		handle_document(state, &doc);

		if (Z_TYPE_P(&doc) == IS_UNDEF) {
			code = Y_PARSER_FAILURE;
			break;
		}

		add_next_index_zval(retval, &doc);

		(*ndocs)++;
	}

	if (state->have_event) {
		yaml_event_delete(&state->event);
	}

	if (Y_PARSER_FAILURE == code) {
		//TODO sdubois
		//zval_ptr_dtor(&retval);
		ZVAL_UNDEF(retval);
	}
}
/* }}} */


/* {{{ php_yaml_read_partial()
 * Read a particular document from the parser's document stream.
 */
void php_yaml_read_partial(
		parser_state_t *state, zend_long pos, zend_long *ndocs, zval *retval)
{
	int code = Y_PARSER_CONTINUE;

	while (Y_PARSER_CONTINUE == code) {

		if (!NEXT_EVENT()) {
			code = Y_PARSER_FAILURE;

		} else if (YAML_STREAM_END_EVENT == state->event.type) {
			if (pos != 0) {
				/* reached end of stream without finding what we wanted */
				php_error_docref(NULL, E_WARNING,
						"end of stream reached without finding document " ZEND_LONG_FMT,
						pos);
				code = Y_PARSER_FAILURE;
			} else {
				/* an empty document is valid YAML */
				code = Y_PARSER_SUCCESS;
				ZVAL_NULL(retval);
			}
		} else if (YAML_DOCUMENT_START_EVENT == state->event.type) {
			if (*ndocs == pos) {
				handle_document(state, retval);
				if (Z_TYPE_P(retval) == IS_UNDEF) {
					code = Y_PARSER_FAILURE;
					break;
				}
				code = Y_PARSER_SUCCESS;
			}
			/* count each document as we pass */
			(*ndocs)++;
		}
	}

	if (state->have_event) {
		yaml_event_delete(&state->event);
	}

	if (Y_PARSER_FAILURE == code) {
		//TODO sdubois
		if (Z_TYPE_P(retval) != IS_UNDEF) {
			ZVAL_UNDEF(retval);
		}
	}
}
/* }}} */


/* {{{ handle_parser_error()
 * Emit a warning about a parser error
 */
static void handle_parser_error(const yaml_parser_t *parser)
{
	const char *error_type;

	switch (parser->error) {
	case YAML_MEMORY_ERROR:
		error_type = "memory allocation";
		break;

	case YAML_READER_ERROR:
		error_type = "reading";
		break;

	case YAML_SCANNER_ERROR:
		error_type = "scanning";
		break;

	case YAML_PARSER_ERROR:
		error_type = "parsing";
		break;

	default:
		/* Shouldn't happen. */
		error_type = "unknown";
		break;
	}

	if (NULL != parser->problem) {
		if (parser->context) {
			php_error_docref(NULL, E_WARNING,
					"%s error encountered during parsing: %s "
					"(line %zd, column %zd), "
					"context %s (line %zd, column %zd)",
					error_type,
					parser->problem,
					parser->problem_mark.line + 1,
					parser->problem_mark.column + 1, parser->context,
					parser->context_mark.line + 1,
					parser->context_mark.column + 1);
		} else {
			php_error_docref(NULL, E_WARNING,
					"%s error encountered during parsing: %s "
					"(line %zd, column %zd)",
					error_type,
					parser->problem,
					parser->problem_mark.line + 1,
					parser->problem_mark.column + 1);
		}
	} else {
		php_error_docref(NULL, E_WARNING,
				"%s error encountred during parsing", error_type);
	}
}
/* }}} */


/* {{{ yaml_next_event()
 * Load the next parser event
 */
static inline int yaml_next_event(parser_state_t *state)
{
	if (state->have_event) {
		/* free prior event */
		yaml_event_delete(&state->event);
		state->have_event = 0;
	}

	if (!yaml_parser_parse(&state->parser, &state->event)) {
		/* error encountered parsing input */
		state->have_event = 0;
		handle_parser_error(&state->parser);

	} else {
		state->have_event = 1;
	}

	return state->have_event;
}
/* }}} */


/* {{{ get_next_element()
 * Extract the next whole element from the parse stream
 */
void get_next_element(parser_state_t *state, zval *retval)
{
	if (!NEXT_EVENT()) {
		/* check state->event if you need to know the difference between
		 * this error and a halt event
		 */
		ZVAL_UNDEF(retval);
		return;
	}

	switch (state->event.type) {
	case YAML_DOCUMENT_END_EVENT:
	case YAML_MAPPING_END_EVENT:
	case YAML_SEQUENCE_END_EVENT:
	case YAML_STREAM_END_EVENT:
		/* halting events */
		break;

	case YAML_DOCUMENT_START_EVENT:
		handle_document(state, retval);
		break;

	case YAML_MAPPING_START_EVENT:
		handle_mapping(state, retval);
		break;

	case YAML_SEQUENCE_START_EVENT:
		handle_sequence(state, retval);
		break;

	case YAML_SCALAR_EVENT:
		handle_scalar(state, retval);
		break;

	case YAML_ALIAS_EVENT:
		handle_alias(state, retval);
		break;

	default:
		/* any other event is an error */
		php_error_docref(NULL, E_WARNING,
				"Unexpected event type %d "
				"(line %zd, column %zd)",
				state->event.type,
				state->parser.mark.line + 1,
				state->parser.mark.column + 1);
		break;
	}
}
/* }}} */


/* {{{ handle_document()
 * Handle a document event
 */
void handle_document(parser_state_t *state, zval *retval)
{
	/* make a new array to hold aliases */
	array_init(&state->aliases);

	/* document consists of next element */
	get_next_element(state, retval);

	/* clean up aliases */
	zval_ptr_dtor(&state->aliases);

	/* assert that end event is next in stream */
	if (NULL != retval && NEXT_EVENT() &&
			YAML_DOCUMENT_END_EVENT != state->event.type) {
		zval_ptr_dtor(retval);
		ZVAL_UNDEF(retval);
		//retval = NULL;
	}
}
/* }}} */


/* {{{ handle_mapping()
 * Handle a mapping event
 */
void handle_mapping(parser_state_t *state, zval *retval)
{
	yaml_event_t src_event = { YAML_NO_EVENT }, key_event = { YAML_NO_EVENT };
	zval key = {{0} };
	zval value = {{0} };
	zval *arrval = retval;
	zval *valptr;

	/* save copy of mapping start event */
	COPY_EVENT(src_event, state);

	/* make a new array to hold mapping */
	array_init(retval);

	if (NULL != src_event.data.mapping_start.anchor) {
		/* record anchors in current alias table */
		arrval = record_anchor_make_ref(&state->aliases, (char *) src_event.data.mapping_start.anchor, retval);
	}

	for (get_next_element(state, &key); Z_TYPE(key) != IS_UNDEF; get_next_element(state, &key)) {
		COPY_EVENT(key_event, state);
		get_next_element(state, &value);

		if (Z_TYPE(value) == IS_UNDEF) {
			yaml_event_delete(&src_event);
			yaml_event_delete(&key_event);
			return;
		}

		valptr = Z_ISREF(value) ? Z_REFVAL(value) : &value;

		/* check for '<<' and handle merge */
		if (key_event.type == YAML_SCALAR_EVENT &&
				IS_NOT_QUOTED_OR_TAG_IS(key_event, YAML_MERGE_TAG) &&
				Z_TYPE(key) == IS_STRING &&
				Z_TYPE_P(valptr) == IS_ARRAY &&
				STR_EQ("<<", Z_STRVAL(key))) {
			/* zend_hash_merge */
			/*
			 * value is either a single ref or a simple array of refs
			 */
			if (YAML_ALIAS_EVENT == state->event.type) {
				/* single ref */
				zend_hash_merge(Z_ARRVAL_P(arrval), Z_ARRVAL_P(valptr), zval_add_ref, 0);
			} else {
				/* array of refs */
				zval *zvalp;
				ZEND_HASH_FOREACH_VAL(HASH_OF(valptr), zvalp) {
					if (Z_ISREF_P(zvalp)) {
						ZVAL_DEREF(zvalp);
						zend_hash_merge(
								Z_ARRVAL_P(arrval), Z_ARRVAL_P(zvalp),
								zval_add_ref, 0);
					} else {
						php_error_docref(NULL, E_WARNING,
								"expected a mapping for merging, but found scalar "
								"(line %zd, column %zd)",
								state->parser.mark.line + 1,
								state->parser.mark.column + 1);
					}
				} ZEND_HASH_FOREACH_END();
			}
			zval_ptr_dtor(&value);
		} else {
			/* add key => value to arrval */
			zval *offset = &key;
			ZVAL_DEREF(offset);
			switch(Z_TYPE_P(offset)) {
			case IS_DOUBLE:
			case IS_FALSE:
			case IS_LONG:
			case IS_NULL:
			case IS_RESOURCE:
			case IS_STRING:
			case IS_TRUE:
				array_set_zval_key(Z_ARRVAL_P(arrval), offset, &value);
				Z_TRY_DELREF(value);
				break;
			case IS_UNDEF:
				zend_hash_update(Z_ARRVAL_P(arrval), ZSTR_EMPTY_ALLOC(), &value);
				break;
			default:
				php_error_docref(
					NULL, E_WARNING,
					"Illegal offset type %s (line %zd, column %zd)",
					zend_zval_type_name(offset),
					state->parser.mark.line + 1,
					state->parser.mark.column + 1
				);
				break;
			}
		}
		yaml_event_delete(&key_event);
		zval_ptr_dtor(&key);
		ZVAL_UNDEF(&key);
	}

	if (YAML_MAPPING_END_EVENT != state->event.type) {
		//TODO Sean-Der
		ZVAL_UNDEF(retval);
	}

	if (NULL != retval && NULL != state->callbacks) {
		/* apply callbacks to the collected node */
		if (Y_FILTER_FAILURE == apply_filter(
				retval, src_event, state->callbacks)) {
			//TODO Sean-Der
			ZVAL_UNDEF(retval);
		}
	}

	yaml_event_delete(&src_event);
}
/* }}} */


/* {{{ handle_sequence
 * Handle a sequence event
 */
void handle_sequence (parser_state_t *state, zval *retval) {
	yaml_event_t src_event = { YAML_NO_EVENT };
	zval value = {{0} };
	zval *arrval = retval;

	/* save copy of sequence start event */
	COPY_EVENT(src_event, state);

	/* make a new array to hold mapping */
	array_init(retval);

	if (NULL != src_event.data.sequence_start.anchor) {
		arrval = record_anchor_make_ref(&state->aliases, (char *) src_event.data.sequence_start.anchor, retval);
	}

	for (get_next_element(state, &value); Z_TYPE_P(&value) != IS_UNDEF; get_next_element(state, &value)) {
		add_next_index_zval(arrval, &value);
		ZVAL_UNDEF(&value);
	}

	if (YAML_SEQUENCE_END_EVENT != state->event.type) {
		//TODO Sean-Der
		ZVAL_UNDEF(retval);
		//zval_ptr_dtor(&retval);
		//retval = NULL;
	}

	if (NULL != retval && NULL != state->callbacks) {
		/* apply callbacks to the collected node */
		if (Y_FILTER_FAILURE == apply_filter(
				retval, src_event, state->callbacks)) {
			//TODO Sean-Der
			ZVAL_UNDEF(retval);
			//zval_ptr_dtor(&retval);
			//retval = NULL;
		}
	}

	yaml_event_delete(&src_event);
}
/* }}} */


/* {{{ handle_scalar()
 * Handle a scalar event
 */
void handle_scalar(parser_state_t *state, zval *retval) {
	state->eval_func(state->event, state->callbacks, retval);
	if (NULL != retval && NULL != state->event.data.scalar.anchor) {
		record_anchor_make_ref(&state->aliases, (char *) state->event.data.scalar.anchor, retval);
	}
}
/* }}} */


/* {{{ handle_alias()
 * Handle an alias event
 */
void handle_alias(parser_state_t *state, zval *retval) {
	char *anchor = (char *) state->event.data.alias.anchor;
	zend_string *anchor_zstring = zend_string_init(anchor, strlen(anchor), 0);
	zval *alias;

	if ((alias = zend_hash_find(Z_ARRVAL_P(&state->aliases), anchor_zstring)) == NULL) {
		php_error_docref(NULL, E_WARNING,
				"alias %s is not registered "
				"(line %zd, column %zd)",
				anchor,
				state->parser.mark.line + 1,
				state->parser.mark.column + 1);
		zend_string_release(anchor_zstring);
		ZVAL_UNDEF(retval);
		return;
	}
	zend_string_release(anchor_zstring);

	/* add a reference to retval's internal counter */
	Z_TRY_ADDREF_P(alias);
	ZVAL_COPY_VALUE(retval, alias);
}
/* }}} */




/* {{{ record_anchor_make_ref()
 * Record an anchor in alias table
 */
static zval *record_anchor_make_ref(zval *aliases, const char *anchor, zval *value)
{
	ZVAL_MAKE_REF(value);
	Z_TRY_ADDREF_P(value);
	add_assoc_zval(aliases, anchor, value);
	return Z_REFVAL_P(value);
}
/* }}} */


/* {{{ apply_filter()
 * Apply user supplied hander to node
 */
static int
apply_filter(zval *zp, yaml_event_t event, HashTable *callbacks)
{
	char *tag = { 0 };
	zend_string *tag_zstring;
	zval *callback = { 0 };

	/* detect event type and get tag */
	switch (event.type) {
	case YAML_SEQUENCE_START_EVENT:
		if (event.data.sequence_start.implicit) {
			tag = YAML_SEQ_TAG;
		} else {
			tag = (char *) event.data.sequence_start.tag;
		}
		break;

	case YAML_MAPPING_START_EVENT:
		if (event.data.sequence_start.implicit) {
			tag = YAML_MAP_TAG;
		} else {
			tag = (char *) event.data.mapping_start.tag;
		}
		break;

	default:
		/* don't care about other event types */
		break;
	}

	if (NULL == tag) {
		return Y_FILTER_NONE;
	}

	/* find and apply the filter function */
	tag_zstring = zend_string_init(tag, strlen(tag), 0);
	if ((callback = zend_hash_find(callbacks, tag_zstring)) != NULL) {
		int callback_result;
		zval callback_args[3];
		zval retval;

		callback_args[0] = *zp;
		ZVAL_STRINGL(&callback_args[1], tag, strlen(tag));
		ZVAL_LONG(&callback_args[2], 0);

		/* call the user function */
		callback_result = call_user_function_ex(EG(function_table), NULL, callback, &retval, 3, callback_args, 0, NULL);

		/* cleanup our temp variables */
		zval_ptr_dtor(&callback_args[1]);
		zval_ptr_dtor(&callback_args[2]);
		zend_string_release(tag_zstring);

		if (FAILURE == callback_result || Z_TYPE_P(&retval) == IS_UNDEF) {
			php_error_docref(NULL, E_WARNING,
					"Failed to apply filter for tag '%s'"
					" with user defined function", tag);
			return Y_FILTER_FAILURE;

		} else {
			if (&retval == zp) {
				/* throw away duplicate response */
				zval_ptr_dtor(&retval);
			} else {
				/* copy result into our return var */
				if (Z_ISREF_P(zp)) {
					zval *tmp = Z_REFVAL_P(zp);
					zval_ptr_dtor(tmp);
					ZVAL_COPY_VALUE(tmp, &retval);
				} else {
					zval_ptr_dtor(zp);
					ZVAL_COPY_VALUE(zp, &retval);
				}
			}
			return Y_FILTER_SUCCESS;
		}

	} else {
		zend_string_release(tag_zstring);
		return Y_FILTER_NONE;
	}
}
/* }}} */


/* {{{ eval_scalar()
 * Convert a scalar node to the proper PHP data type.
 *
 * All YAML scalar types found at http://yaml.org/type/index.html.
 */
void eval_scalar(yaml_event_t event,
		HashTable * callbacks, zval *retval)
{
	char *value = (char *) event.data.scalar.value;
	size_t length = event.data.scalar.length;
	int flags = 0;

	ZVAL_NULL(retval);

	/* check for non-specific tag (treat as a string) */
	if (SCALAR_TAG_IS(event, YAML_NONSPECIFIC_TAG) ||
			event.data.scalar.quoted_implicit) {
		ZVAL_STRINGL(retval, value, length);
		return;
	}

	/* check for null */
	if (scalar_is_null(value, length, &event)) {
		return;
	}

	/* check for bool */
	if (-1 != (flags = scalar_is_bool(value, length, &event))) {
		ZVAL_BOOL(retval, (zend_bool) flags);
		return;
	}

	/* check for numeric (int or float) */
	if (!event.data.scalar.quoted_implicit &&
			(event.data.scalar.plain_implicit ||
			 SCALAR_TAG_IS(event, YAML_INT_TAG) ||
			 SCALAR_TAG_IS(event, YAML_FLOAT_TAG))) {
		zend_long lval = 0;
		double dval = 0.0;

		flags = scalar_is_numeric(
				value, length, &lval, &dval, NULL);
		if (flags != Y_SCALAR_IS_NOT_NUMERIC) {
			if (flags & Y_SCALAR_IS_FLOAT) {
				ZVAL_DOUBLE(retval, dval);

			} else {
				ZVAL_LONG(retval, lval);
			}

			if (event.data.scalar.plain_implicit) {
				/* pass */

			} else if (SCALAR_TAG_IS(event, YAML_FLOAT_TAG) &&
					(flags & Y_SCALAR_IS_INT)) {
				convert_to_double(retval);

			} else if (SCALAR_TAG_IS(event, YAML_INT_TAG) &&
					(flags & Y_SCALAR_IS_FLOAT)) {
				convert_to_long(retval);
			}

			return;

		} else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_FLOAT_TAG)) {
			ZVAL_STRINGL(retval, value, length);
			convert_to_double(retval);
			return;

		} else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_INT_TAG)) {
			ZVAL_STRINGL(retval, value, length);
			convert_to_long(retval);
			return;
		}
	}

	/* check for timestamp */
	if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_TIMESTAMP_TAG) ||
			 scalar_is_timestamp(value, length)) {
		if (FAILURE == eval_timestamp(
				&retval, value, (int) length)) {
			ZVAL_NULL(retval);
		}
		return;
	}

	/* check for binary */
	if (YAML_G(decode_binary) &&
			IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_BINARY_TAG)) {
		zend_string *data;

		data = php_base64_decode((const unsigned char *) value, (int) length);
		if (NULL == data) {
			php_error_docref(NULL, E_WARNING,
					"Failed to decode binary");
			ZVAL_NULL(retval);

		} else {
			ZVAL_STR(retval, data);
		}

		return;
	}

	/* check for php object */
	if (YAML_G(decode_php) &&
			IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_PHP_TAG)) {
		const unsigned char *p;
		php_unserialize_data_t var_hash;

		p = (const unsigned char *) value;
		PHP_VAR_UNSERIALIZE_INIT(var_hash);

		if (!php_var_unserialize(
				retval, &p, p + (int) length, &var_hash)) {
			php_error_docref(NULL, E_NOTICE,
					"Failed to unserialize class");
			/* return the serialized string directly */
			ZVAL_STRINGL(retval, value, length);
		}

		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		return;
	}

	/* others (treat as a string) */
	ZVAL_STRINGL(retval, value, length);

	return;
}
/* }}} */


/* {{{ eval_scalar_with_callbacks()
 * Convert a scalar node to the proper PHP data type using user supplied input
 * filters if available.
 */
void eval_scalar_with_callbacks(yaml_event_t event,
		HashTable *callbacks, zval *retval)
{
	const char *tag = (char *) event.data.scalar.tag;
	zend_string *tag_zstring;
	zval *callback;

	if (YAML_PLAIN_SCALAR_STYLE == event.data.scalar.style && NULL == tag) {
		/* plain scalar with no specified type */
		tag = detect_scalar_type(
				(char *) event.data.scalar.value, event.data.scalar.length,
				&event);
	}
	if (NULL == tag) {
		/* couldn't/wouldn't detect tag type, assume string */
		tag = YAML_STR_TAG;
	}

	tag_zstring = zend_string_init(tag, strlen(tag), 0);
	/* find and apply the evaluation function */
	if ((callback = zend_hash_find(callbacks, tag_zstring)) != NULL) {
		zval argv[3];

		ZVAL_STRINGL(&argv[0], (const char *) event.data.scalar.value, event.data.scalar.length);
		ZVAL_STRINGL(&argv[1], tag, strlen(tag));
		ZVAL_LONG(&argv[2], event.data.scalar.style);

		if (FAILURE == call_user_function_ex(EG(function_table), NULL, callback, retval, 3, argv, 0, NULL) || Z_TYPE_P(retval) == IS_UNDEF) {
			php_error_docref(NULL, E_WARNING,
					"Failed to evaluate value for tag '%s'"
					" with user defined function", tag);
		}

		zval_ptr_dtor(&argv[0]);
		zval_ptr_dtor(&argv[1]);
		zval_ptr_dtor(&argv[2]);
	} else {
		/* no mapping, so handle raw */
		eval_scalar(event, NULL, retval);
	}
	zend_string_release(tag_zstring);
}
/* }}} */


/* {{{ eval_timestamp()
 * Convert a timestamp
 *
 * This is switched on/off by the `yaml.decode_timestamp` ini setting.
 *  - yaml.decode_timestamp=0 for no timestamp parsing
 *  - yaml.decode_timestamp=1 for strtotime parsing
 *  - yaml.decode_timestamp=2 for date_create parsing
 */
static int
eval_timestamp(zval **zpp, const char *ts, size_t ts_len)
{
	if (NULL != YAML_G(timestamp_decoder) ||
			1L == YAML_G(decode_timestamp) ||
			2L == YAML_G(decode_timestamp)) {
		zval argv[1];
		zval arg, retval, *func, afunc;
		const char *funcs[] = { "strtotime", "date_create" };

		if (NULL == YAML_G(timestamp_decoder)) {
			if (2L == YAML_G(decode_timestamp)) {
				ZVAL_STRING(&afunc, funcs[1]);

			} else {
				ZVAL_STRING(&afunc, funcs[0]);
			}

			func = &afunc;
		} else {
			func = YAML_G(timestamp_decoder);
		}

		ZVAL_STRINGL(&arg, ts, ts_len);
		argv[0] = arg;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL, func,
				&retval, 1, argv, 0, NULL) || Z_TYPE_P(&retval) == IS_UNDEF) {
			php_error_docref(NULL, E_WARNING,
					"Failed to evaluate string '%s' as timestamp", ts);
			if (func != NULL) {
				zval_ptr_dtor(func);
			}
			zval_ptr_dtor(&arg);
			return FAILURE;

		} else {
			if (func != NULL) {
				zval_ptr_dtor(func);
			}
			zval_ptr_dtor(&arg);
			ZVAL_COPY_VALUE(*zpp, &retval);
			return SUCCESS;
		}

	} else {
		zval_dtor(*zpp);
		ZVAL_STRINGL(*zpp, ts, ts_len);
		return SUCCESS;
	}
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
