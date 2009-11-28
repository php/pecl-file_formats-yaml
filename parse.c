/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
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
 * @author      Bryan Davis <bpd@keynetics.com>
 * @copyright   2007 Ryusuke SEKIYAMA
 * @copyright   2009 Keynetics Inc
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */


#include "php_yaml.h"
#include "zval_zval.h"			/* for PHP 4.x */
#include "zval_refcount.h"		/* for PHP < 5.3 */
#include "php_yaml_int.h"

/* {{{ local macros
 */
#define SCALAR_TAG_IS(event, name) \
	!strcmp((const char *)event.data.scalar.tag, name)

#define IS_NOT_IMPLICIT_AND_TAG_IS(event, name) \
	(!event.data.scalar.quoted_implicit && !event.data.scalar.plain_implicit && SCALAR_TAG_IS(event, name))

#define IS_NOT_QUOTED_OR_TAG_IS(event, name) \
	(!event.data.scalar.quoted_implicit && (event.data.scalar.plain_implicit || SCALAR_TAG_IS(event, name)))

#define ts_skip_space() \
	while (ptr < end && (*ptr == ' ' || *ptr == '\t')) { \
		ptr++; \
	}

#define ts_skip_number() \
	while (ptr < end && *ptr >= '0' && *ptr <= '9') { \
		ptr++; \
	}

/* }}} */

/* {{{ local prototypes
 */
static char *php_yaml_convert_to_char(zval * zv TSRMLS_DC);

static void php_yaml_handle_parser_error(
		const yaml_parser_t * parser TSRMLS_DC);

static int php_yaml_apply_filter(
		zval ** zpp, yaml_event_t event, HashTable * callbacks TSRMLS_DC);

static int php_yaml_scalar_is_null(
		const char *value, size_t length, const yaml_event_t * event);

static int php_yaml_scalar_is_bool(
		const char *value, size_t length, const yaml_event_t * event);

static int php_yaml_scalar_is_numeric(
		const char *value, size_t length, long *lval, double *dval, char **str);

static int php_yaml_scalar_is_timestamp(const char *value, size_t length);

char *php_yaml_detect_scalar_type(
		const char *value, size_t length, const yaml_event_t * event);

static long php_yaml_eval_sexagesimal_l(long lval, char *sg, char *eos);

static double php_yaml_eval_sexagesimal_d(
		double dval, char *sg, char *eos);

static int php_yaml_eval_timestamp(
		zval ** zpp, char *ts, int ts_len TSRMLS_DC);

/* }}} */

/* {{{ php_yaml_convert_to_char()
 * Convert a zval to a character array.
 */
static char *php_yaml_convert_to_char(zval * zv TSRMLS_DC)
{
	char *str = NULL;

	switch (Z_TYPE_P(zv)) {
	case IS_BOOL:
		if (Z_BVAL_P(zv)) {
			str = estrndup("1", 1);
		} else {
			str = estrndup("", 0);
		}
		break;

	case IS_DOUBLE:
		{
			char buf[64] = { '\0' };
			(void) snprintf(buf, 64, "%G", Z_DVAL_P(zv));
			str = estrdup(buf);
		}
		break;

	case IS_LONG:
		{
			char buf[32] = { '\0' };
			(void) snprintf(buf, 32, "%ld", Z_LVAL_P(zv));
			str = estrdup(buf);
		}
		break;

	case IS_NULL:
		str = estrndup("", 0);
		break;

	case IS_STRING:
		str = estrndup(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
		break;

#ifdef IS_UNICODE
	case IS_UNICODE:
		{
			int len;
			UErrorCode status = U_ZERO_ERROR;

			zend_unicode_to_string_ex(UG(utf8_conv), &str, &len,
					Z_USTRVAL_P(zv), Z_USTRLEN_P(zv), &status);
			if (U_FAILURE(status)) {
				if (str != NULL) {
					efree(str);
					str = NULL;
				}
			}
		}
		break;
#endif

#ifdef ZEND_ENGINE_2
	case IS_OBJECT:
		{
			zval tmp;

			if (SUCCESS == zend_std_cast_object_tostring(
#if PHP_MAJOR_VERSION >= 6
					zv, &tmp, IS_STRING, UG(utf8_conv) TSRMLS_CC
#elif PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2
					zv, &tmp, IS_STRING TSRMLS_CC
#else
					zv, &tmp, IS_STRING, 0 TSRMLS_CC
#endif
					)) {
				str = estrndup(Z_STRVAL(tmp), Z_STRLEN(tmp));
				zval_dtor(&tmp);
				return str;
			}
		}
#endif

	default:
		{
			php_serialize_data_t var_hash;
			smart_str buf = { 0 };

			PHP_VAR_SERIALIZE_INIT(var_hash);
			php_var_serialize(&buf, &zv, &var_hash TSRMLS_CC);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);

			str = buf.c;
		}
	}

	if (NULL == str) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to convert %s to string", zend_zval_type_name(zv));
	}

	return str;
}

/* }}} */

/* {{{ php_yaml_handle_parser_error()
 * Emit a warning about a parser error
 */
static void
php_yaml_handle_parser_error(const yaml_parser_t * parser TSRMLS_DC)
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

	if (parser->problem != NULL) {
		if (parser->context) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"%s error encountered during parsing: %s "
					"(line %ld, column %ld), "
					"context %s (line %ld, column %ld)",
					error_type,
					parser->problem,
					parser->problem_mark.line + 1,
					parser->problem_mark.column + 1, parser->context,
					parser->context_mark.line + 1,
					parser->context_mark.column + 1);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"%s error encountered during parsing: %s "
					"(line %ld, column %ld)",
					error_type,
					parser->problem,
					parser->problem_mark.line + 1,
					parser->problem_mark.column + 1);
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"%s error encountred during parsing", error_type);
	}
}

/* }}} */

/* {{{ php_yaml_read_impl()
 * Process events from yaml parser
 */
zval *php_yaml_read_impl(yaml_parser_t * parser, yaml_event_t * parent,
		zval * aliases, zval * zv, long *ndocs,
		eval_scalar_func_t eval_func, HashTable * callbacks TSRMLS_DC)
{
	zval *retval = NULL;
	yaml_event_t event = { 0 };
	char *key = NULL;
	int code = Y_PARSER_CONTINUE;

	if (zv != NULL) {
		/* use provided value as our result holder */
		retval = zv;

	} else {
		/* create an empty array to hold results */
		MAKE_STD_ZVAL(retval);
		array_init(retval);
#ifdef IS_UNICODE
		Z_ARRVAL_P(retval)->unicode = UG(unicode);
#endif
	}

	do {
		zval *tmp_p = NULL;
		zval **tmp_pp = NULL;

		if (!yaml_parser_parse(parser, &event)) {
			/* error encountered parsing input */
			php_yaml_handle_parser_error(parser TSRMLS_CC);
			code = Y_PARSER_FAILURE;

			/* break out of do-while and continue to cleanup code */
			break;
		}

		switch (event.type) {
		case YAML_NO_EVENT:
		case YAML_STREAM_START_EVENT:
			/* don't care about stream start */
			break;

		case YAML_DOCUMENT_START_EVENT:
			{
				/* make a new array to hold aliases */
				zval *a = NULL;
				MAKE_STD_ZVAL(a);
				array_init(a);
#ifdef IS_UNICODE
				Z_ARRVAL_P(a)->unicode = UG(unicode);
#endif
				/* parse the found document contents into current retval */
				if (NULL == php_yaml_read_impl(
						parser, &event, a, retval, ndocs, eval_func,
						callbacks TSRMLS_CC)) {
					code = Y_PARSER_FAILURE;
				}
				zval_ptr_dtor(&a);
			}
			(*ndocs)++;
			break;

		case YAML_SEQUENCE_START_EVENT:
		case YAML_MAPPING_START_EVENT:
			/* sequences and maps are both arrays in php-land */
			MAKE_STD_ZVAL(tmp_p);
			array_init(tmp_p);
#ifdef IS_UNICODE
			Z_ARRVAL_P(tmp_p)->unicode = UG(unicode);
#endif

			/* record anchors in current alias table */
			if (YAML_SEQUENCE_START_EVENT == event.type) {
				if (event.data.sequence_start.anchor != NULL) {
					Z_ADDREF_P(tmp_p);
					Z_SET_ISREF_P(tmp_p);
					add_assoc_zval(aliases,
							(char *) event.data.sequence_start.anchor,
							tmp_p);
				}
			} else if (YAML_MAPPING_START_EVENT == event.type) {
				if (event.data.mapping_start.anchor != NULL) {
					Z_ADDREF_P(tmp_p);
					Z_SET_ISREF_P(tmp_p);
					add_assoc_zval(aliases,
							(char *) event.data.mapping_start.anchor,
							tmp_p);
				}
			}

			/* continue parsing using this event as the parent and our newly 
			 * allocated array as the holder container
			 */
			tmp_p = php_yaml_read_impl(parser, &event, aliases, tmp_p,
					ndocs, eval_func, callbacks TSRMLS_CC);
			if (NULL == tmp_p) {
				code = Y_PARSER_FAILURE;
				break;
			}

			if (callbacks != NULL) {
				/* apply callbacks to the collected node */
				if (Y_FILTER_FAILURE == php_yaml_apply_filter(&tmp_p,
								event, callbacks TSRMLS_CC)) {
					zval_ptr_dtor(&tmp_p);
					code = Y_PARSER_FAILURE;
					break;
				}
			}

			if (YAML_MAPPING_START_EVENT == parent->type) {
				if (NULL == key) {
					/* new node is key for mapping */
					key = php_yaml_convert_to_char(tmp_p TSRMLS_CC);
					if (NULL == key) {
						zval_ptr_dtor(&tmp_p);
						code = Y_PARSER_FAILURE;
						break;
					}
					/* assign tmp_p to the alias-storage
					   tmp_p will be freed in its destructor */
					add_next_index_zval(aliases, tmp_p);
				} else {
					/* new node is value, assign to container */
					/* XXX: support << merge */
					add_assoc_zval(retval, key, tmp_p);
					efree(key);
					key = NULL;
				}

			} else {
				/* append the node onto the current container */
				add_next_index_zval(retval, tmp_p);
			}
			break;

		case YAML_STREAM_END_EVENT:
		case YAML_DOCUMENT_END_EVENT:
		case YAML_SEQUENCE_END_EVENT:
		case YAML_MAPPING_END_EVENT:
			code = Y_PARSER_SUCCESS;
			break;

		case YAML_ALIAS_EVENT:
			if (SUCCESS == zend_hash_find(Z_ARRVAL_P(aliases),
					(char *) event.data.alias.anchor,
					(uint) strlen((char *) event.data.alias.
					anchor) + 1, (void **) &tmp_pp)) {
				if (YAML_MAPPING_START_EVENT == parent->type) {
					if (NULL == key) {
						key = php_yaml_convert_to_char(*tmp_pp TSRMLS_CC);
						if (NULL == key) {
							code = Y_PARSER_FAILURE;
							break;
						}
					} else {
						/* add a reference to tmp_pp's internal counter */
						Z_ADDREF_PP(tmp_pp);

						/* XXX: support << merge */
						add_assoc_zval(retval, key, *tmp_pp);
						efree(key);
						key = NULL;
					}
				} else {
					Z_ADDREF_PP(tmp_pp);
					add_next_index_zval(retval, *tmp_pp);
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"alias %s is not registered",
						(char *) event.data.alias.anchor);
				code = Y_PARSER_FAILURE;
			}
			break;

		case YAML_SCALAR_EVENT:
			if (YAML_MAPPING_START_EVENT == parent->type) {
				if (NULL == key) {
					tmp_p = eval_func(event, callbacks TSRMLS_CC);
					key = php_yaml_convert_to_char(tmp_p TSRMLS_CC);
					/* assign tmp_p to the alias-storage
					   tmp_p will be freed in its destructor */
					add_next_index_zval(aliases, tmp_p);
				} else {
					tmp_p = eval_func(event, callbacks TSRMLS_CC);
					if (NULL == tmp_p) {
						code = Y_PARSER_FAILURE;
						break;
					}
					/* XXX: support << merge */
					add_assoc_zval(retval, key, tmp_p);
					efree(key);
					key = NULL;
				}

			} else {
				tmp_p = eval_func(event, callbacks TSRMLS_CC);
				if (NULL == tmp_p) {
					code = Y_PARSER_FAILURE;
					break;
				}
				add_next_index_zval(retval, tmp_p);
			}

			if (event.data.scalar.anchor != NULL) {
				if (NULL == tmp_p) {
					add_assoc_string(aliases,
							(char *) event.data.scalar.anchor, key, 1);
				} else {
					Z_ADDREF_P(tmp_p);
					Z_SET_ISREF_P(tmp_p);
					add_assoc_zval(aliases,
							(char *) event.data.scalar.anchor, tmp_p);
				}
			}
			break;

		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"unknown event type");
			code = Y_PARSER_FAILURE;
		}

		yaml_event_delete(&event);
	} while (Y_PARSER_CONTINUE == code);

	if (key != NULL) {
		if (Y_PARSER_SUCCESS == code) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"invalid mapping structure");
		}
		efree(key);
		code = Y_PARSER_FAILURE;
	}

	if (Y_PARSER_FAILURE == code) {
		*ndocs = -1;
		if (NULL == zv && retval != NULL) {
			zval_ptr_dtor(&retval);
		}
		return NULL;
	}

	return retval;
}

/* }}} */

/* {{{ php_yaml_read_partial()
 * Read a particular document from the parser's document stream.
 */
zval *php_yaml_read_partial(yaml_parser_t * parser, long pos, long *ndocs,
		eval_scalar_func_t eval_func, HashTable * callbacks TSRMLS_DC)
{
	zval *retval = NULL;
	yaml_event_t event = { 0 };
	int code = Y_PARSER_CONTINUE;

	do {
		if (!yaml_parser_parse(parser, &event)) {
			php_yaml_handle_parser_error(parser TSRMLS_CC);
			code = Y_PARSER_FAILURE;
			break;
		}

		if (YAML_DOCUMENT_START_EVENT == event.type) {
			if (*ndocs == pos) {
				/* we're reached the document that the user is interested in */
				zval *tmp_p = NULL;
				zval *aliases = NULL;
				MAKE_STD_ZVAL(aliases);
				array_init(aliases);
#ifdef IS_UNICODE
				Z_ARRVAL_P(aliases)->unicode = UG(unicode);
#endif
				/* parse document contents */
				tmp_p = php_yaml_read_impl(parser, &event, aliases, NULL,
						ndocs, eval_func, callbacks TSRMLS_CC);
				if (tmp_p == NULL) {
					code = Y_PARSER_FAILURE;

				} else {
					/* extract first array entry from parse result */
					zval **tmp_pp = NULL;
					if (SUCCESS == zend_hash_index_find(
							Z_ARRVAL_P(tmp_p), 0, (void **) &tmp_pp)) {
						MAKE_STD_ZVAL(retval);
						ZVAL_ZVAL(retval, *tmp_pp, 1, 0);
					}
					zval_ptr_dtor(&tmp_p);

					/* got what we wanted so we're done now */
					code = Y_PARSER_SUCCESS;
				}
				zval_ptr_dtor(&aliases);
			}
			/* count each document as we pass */
			(*ndocs)++;

		} else if (YAML_STREAM_END_EVENT == event.type) {
			/* reached end of stream without finding what we wanted */
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"end of stream reached without finding document %ld",
					pos);
			code = Y_PARSER_FAILURE;
		}

		yaml_event_delete(&event);
	} while (Y_PARSER_CONTINUE == code);

	if (Y_PARSER_FAILURE == code) {
		*ndocs = -1;
		if (retval != NULL) {
			zval_ptr_dtor(&retval);
		}
		return NULL;
	}

	return retval;
}

/* }}} */

/* {{{ php_yaml_apply_filter()
 * Apply user supplied hander to node
 */
static int
php_yaml_apply_filter(zval ** zpp, yaml_event_t event,
		HashTable * callbacks TSRMLS_DC)
{
	char *tag = NULL;
	zval **callback = NULL;

	/* detect event type and get tag */
	switch (event.type) {
	case YAML_SEQUENCE_START_EVENT:
		if (!event.data.sequence_start.implicit) {
			tag = (char *) event.data.sequence_start.tag;
		}
		break;

	case YAML_MAPPING_START_EVENT:
		if (!event.data.mapping_start.implicit) {
			tag = (char *) event.data.mapping_start.tag;
		}
		break;

	default:
		/* don't care about other event types */
		tag = NULL;
		break;
	}

	if (NULL == tag) {
		return Y_FILTER_NONE;
	}

	/* find and apply the filter function */
	if (SUCCESS == zend_hash_find(
			callbacks, tag, strlen(tag) + 1, (void **) &callback)) {
		zval **argv[] = { zpp };
		zval *retval = NULL;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL,
				*callback, &retval, 1, argv, 0, NULL TSRMLS_CC) ||
				NULL == retval) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to apply filter for tag '%s'"
					" with user defined function", tag);
			return Y_FILTER_FAILURE;

		} else {
			zval_dtor(*zpp);
			ZVAL_ZVAL(*zpp, retval, 1, 1);
			return Y_FILTER_SUCCESS;
		}
	} else {
		return Y_FILTER_NONE;
	}
}

/* }}} */

/* {{{ php_yaml_eval_scalar()
 * Convert a scalar node to the proper PHP data type.
 *
 * All YAML scalar types found at http://yaml.org/type/index.html.
 */
zval *php_yaml_eval_scalar(yaml_event_t event,
		HashTable * callbacks TSRMLS_DC)
{
	zval *tmp = NULL;
	char *value = (char *) event.data.scalar.value;
	size_t length = event.data.scalar.length;
	int flags = 0;

	MAKE_STD_ZVAL(tmp);
	ZVAL_NULL(tmp);

	/* check for null */
	if (php_yaml_scalar_is_null(value, length, &event)) {
		return tmp;
	}

	/* check for bool */
	if ((flags = php_yaml_scalar_is_bool(value, length, &event)) != -1) {
		ZVAL_BOOL(tmp, (zend_bool) flags);
		return tmp;
	}

	/* check for numeric (int or float) */
	if (!event.data.scalar.quoted_implicit &&
			(event.data.scalar.plain_implicit ||
			 SCALAR_TAG_IS(event, YAML_INT_TAG) ||
			 SCALAR_TAG_IS(event, YAML_FLOAT_TAG))) {
		long lval = 0;
		double dval = 0.0;

		flags = php_yaml_scalar_is_numeric(
				value, length, &lval, &dval, NULL);
		if (flags != Y_SCALAR_IS_NOT_NUMERIC) {
			if (flags & Y_SCALAR_IS_FLOAT) {
				ZVAL_DOUBLE(tmp, dval);
			} else {
				ZVAL_LONG(tmp, lval);
			}

			if (event.data.scalar.plain_implicit) {
				/* pass */
			} else if (SCALAR_TAG_IS(event, YAML_FLOAT_TAG) &&
					(flags & Y_SCALAR_IS_INT)) {
				convert_to_double(tmp);
			} else if (SCALAR_TAG_IS(event, YAML_INT_TAG) &&
					(flags & Y_SCALAR_IS_FLOAT)) {
				convert_to_long(tmp);
			}
			return tmp;

		} else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_FLOAT_TAG)) {
			ZVAL_STRINGL(tmp, value, length, 1);
			convert_to_double(tmp);
			return tmp;

		} else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_INT_TAG)) {
			ZVAL_STRINGL(tmp, value, length, 1);
			convert_to_long(tmp);
			return tmp;
		}
	}

	/* check for timestamp */
	if (event.data.scalar.plain_implicit ||
			event.data.scalar.quoted_implicit) {
		if (php_yaml_scalar_is_timestamp(value, length)) {
			if (FAILURE == php_yaml_eval_timestamp(
					&tmp, value, (int) length TSRMLS_CC)) {
				zval_ptr_dtor(&tmp);
				return NULL;
			}
			return tmp;
		}

	} else if (SCALAR_TAG_IS(event, YAML_TIMESTAMP_TAG)) {
		if (FAILURE == php_yaml_eval_timestamp(
				&tmp, value, (int) length TSRMLS_CC)) {
			zval_ptr_dtor(&tmp);
			return NULL;
		}
		return tmp;
	}

	/* check for binary */
	if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_BINARY_TAG)) {
		if (YAML_G(decode_binary)) {
			unsigned char *data = NULL;
			int data_len = 0;

			data = php_base64_decode(
					(const unsigned char *) value, (int) length, &data_len);
			if (NULL == data) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Failed to decode binary");
				ZVAL_NULL(tmp);
			} else {
				ZVAL_STRINGL(tmp, (char *) data, data_len, 0);
			}
		} else {
			ZVAL_STRINGL(tmp, value, length, 1);
		}
		return tmp;
	}

	/* check for php object */
	if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_PHP_TAG)) {
		const unsigned char *p;
		php_unserialize_data_t var_hash;

		p = (const unsigned char *) value;
		PHP_VAR_UNSERIALIZE_INIT(var_hash);

		if (!php_var_unserialize(
				&tmp, &p, p + (int) length, &var_hash TSRMLS_CC)) {
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
			php_error_docref(NULL TSRMLS_CC, E_NOTICE,
					"Failed to unserialize class");
			/* return the serialized string directly */
			ZVAL_STRINGL(tmp, value, length, 1);
		}

		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		return tmp;
	}

	/* others (treat as a string) */
#ifdef IS_UNICODE
	ZVAL_U_STRINGL(UG(utf8_conv), tmp, value, length, ZSTR_DUPLICATE);
#else
	ZVAL_STRINGL(tmp, value, length, 1);
#endif

	return tmp;
}

/* }}} */

/* {{{ php_yaml_eval_scalar_with_callbacks()
 * Convert a scalar node to the proper PHP data type using user supplied input 
 * filters if available.
 */
zval *php_yaml_eval_scalar_with_callbacks(yaml_event_t event,
		HashTable * callbacks TSRMLS_DC)
{
	char *tag = (char *) event.data.scalar.tag;
	zval **callback = NULL;

	if (YAML_PLAIN_SCALAR_STYLE == event.data.scalar.style && NULL == tag) {
		/* plain scalar with no specified type */
		tag = php_yaml_detect_scalar_type(
				(char *) event.data.scalar.value, event.data.scalar.length,
				&event);
	}
	if (NULL == tag) {
		/* couldn't/wouldn't detect tag type, assume string */
		tag = YAML_STR_TAG;
	}

	/* find and apply the evaluation function */
	if (SUCCESS == zend_hash_find(
			callbacks, tag, strlen(tag) + 1, (void **) &callback)) {
		zval **argv[] = { NULL, NULL, NULL };
		zval *arg1 = NULL;
		zval *arg2 = NULL;
		zval *arg3 = NULL;
		zval *retval = NULL;

		MAKE_STD_ZVAL(arg1);
		ZVAL_STRINGL(arg1, (char *) event.data.scalar.value,
				event.data.scalar.length, 1);
		argv[0] = &arg1;

		MAKE_STD_ZVAL(arg2);
		ZVAL_STRINGL(arg2, tag, strlen(tag) + 1, 1);
		argv[1] = &arg2;

		MAKE_STD_ZVAL(arg3);
		ZVAL_LONG(arg3, event.data.scalar.style);
		argv[2] = &arg3;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL,
				*callback, &retval, 3, argv, 0, NULL TSRMLS_CC) ||
				NULL == retval) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to evaluate value for tag '%s'"
					" with user defined function", tag);
		}

		zval_ptr_dtor(&arg1);
		zval_ptr_dtor(&arg2);
		zval_ptr_dtor(&arg3);

		return retval;
	}

	/* no mapping, so handle raw */
	return php_yaml_eval_scalar(event, NULL TSRMLS_CC);
}

/* }}} */

/* {{{ php_yaml_scalar_is_null(const char *,size_t,yaml_event_t)
 * Does this scalar encode a NULL value?
 *
 * specification is found at http://yaml.org/type/null.html.
 */
static int
php_yaml_scalar_is_null(const char *value, size_t length,
		const yaml_event_t * event)
{
	if (NULL != event && event->data.scalar.quoted_implicit) {
		return 0;
	}
	if (NULL == event || event->data.scalar.plain_implicit) {
		if ((length == 1 && *value == '~') || length == 0 ||
				!strcmp("NULL", value) || !strcmp("Null", value) ||
				!strcmp("null", value)) {
			return 1;
		}
	} else if (NULL != event && SCALAR_TAG_IS((*event), YAML_NULL_TAG)) {
		return 1;
	}

	return 0;
}

/* }}} */

/* {{{ php_yaml_scalar_is_bool(const char *,size_t,yaml_event_t)
 * Does this scalar encode a BOOL value?
 *
 * specification is found at http://yaml.org/type/bool.html.
 */
static int
php_yaml_scalar_is_bool(const char *value, size_t length,
		const yaml_event_t * event)
{
	/* TODO: add ini setting to turn 'y'/'n' checks on/off */
	if (NULL == event || IS_NOT_QUOTED_OR_TAG_IS((*event), YAML_BOOL_TAG)) {
		if ((length == 1 && (*value == 'Y' || *value == 'y')) ||
				!strcmp("YES", value) || !strcmp("Yes", value) ||
				!strcmp("yes", value) || !strcmp("TRUE", value) ||
				!strcmp("True", value) || !strcmp("true", value) ||
				!strcmp("ON", value) || !strcmp("On", value) ||
				!strcmp("on", value)) {
			return 1;
		}
		if ((length == 1 && (*value == 'N' || *value == 'n')) ||
				!strcmp("NO", value) || !strcmp("No", value) || 
				!strcmp("no", value) || !strcmp("FALSE", value) || 
				!strcmp("False", value) || !strcmp("false", value) ||
				!strcmp("OFF", value) || !strcmp("Off", value) ||
				!strcmp("off", value)) {
			return 0;
		}

	} else if (NULL != event &&
			IS_NOT_IMPLICIT_AND_TAG_IS((*event), YAML_BOOL_TAG)) {
		if (0 == length || (1 == length && '0' == *value)) {
			return 0;
		} else {
			return 1;
		}
	}

	return -1;
}

/* }}} */

/* {{{ php_yaml_scalar_is_numeric()
 * Does this scalar encode a NUMERIC value?
 *
 * specification is found at http://yaml.org/type/float.html.
 * specification is found at http://yaml.org/type/int.html.
 */
static int
php_yaml_scalar_is_numeric(const char *value, size_t length, long *lval,
		double *dval, char **str)
{
	const char *end = value + length;
	char *buf = NULL, *ptr = NULL;
	int negative = 0;
	int type = 0;

	if (0 == length) {
		goto not_numeric;
	}

	/* trim */
	while (value < end && (*(end - 1) == ' ' || *(end - 1) == '\t')) {
		end--;
	}
	while (value < end && (*value == ' ' || *value == '\t')) {
		value++;
	}
	if (value == end) {
		goto not_numeric;
	}

	/* not a number */
	if (!strcmp(".NAN", value) || !strcmp(".NaN", value) ||
			!strcmp(".nan", value)) {
		type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_NAN;
		goto finish;
	}

	/* sign */
	if (*value == '+') {
		value++;
	} else if (*value == '-') {
		negative = 1;
		value++;
	}
	if (value == end) {
		goto not_numeric;
	}

	/* infinity */
	if (!strcmp(".INF", value) || !strcmp(".Inf", value) ||
			!strcmp(".inf", value)) {
		type = Y_SCALAR_IS_FLOAT;
		type |= (negative ? Y_SCALAR_IS_INFINITY_N : Y_SCALAR_IS_INFINITY_P);
		goto finish;
	}

	/* alloc */
	buf = (char *) emalloc(length + 3);
	ptr = buf;
	if (negative) {
		*ptr++ = '-';
	}

	/* parse */
	if (*value == '0') {
		*ptr++ = *value++;
		if (value == end) {
			goto return_zero;
		}

		if (*value == 'b') {
			/* binary integer */
			*ptr++ = *value++;
			if (value == end) {
				goto not_numeric;
			}
			while (value < end && (*value == '_' || *value == '0')) {
				value++;
			}
			if (value == end) {
				goto return_zero;
			}
			/* check the sequence */
			while (value < end) {
				if (*value == '_') {
					value++;
				} else if (*value == '0' || *value == '1') {
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			}
			type = Y_SCALAR_IS_INT | Y_SCALAR_IS_BINARY;

		} else if (*value == 'x') {
			/* hexadecimal integer */
			*ptr++ = *value++;
			if (value == end) {
				goto not_numeric;
			}
			while (value < end && (*value == '_' || *value == '0')) {
				value++;
			}
			if (value == end) {
				goto return_zero;
			}
			/* check the sequence */
			while (value < end) {
				if (*value == '_') {
					value++;
				} else if ((*value >= '0' && *value <= '9') ||
						(*value >= 'A' && *value <= 'F') ||
						(*value >= 'a' && *value <= 'f')) {
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			}
			type = Y_SCALAR_IS_INT | Y_SCALAR_IS_HEXADECIMAL;

		} else if (*value == '_' || (*value >= '0' && *value <= '7')) {
			/* octal integer */
			while (value < end) {
				if (*value == '_') {
					value++;
				} else if (*value >= '0' && *value <= '7') {
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			}
			type = Y_SCALAR_IS_INT | Y_SCALAR_IS_OCTAL;

		} else if (*value == '.') {
			goto check_float;

		} else {
			goto not_numeric;
		}

	} else if (*value >= '1' && *value <= '9') {
		/* integer */
		*ptr++ = *value++;
		while (value < end) {
			if (*value == '_' || *value == ',') {
				value++;
			} else if (*value >= '0' && *value <= '9') {
				*ptr++ = *value++;
			} else if (*value == ':') {
				goto check_sexa;
			} else if (*value == '.') {
				goto check_float;
			} else {
				goto not_numeric;
			}
		}
		type = Y_SCALAR_IS_INT | Y_SCALAR_IS_DECIMAL;

	} else if (*value == ':') {
		/* sexagecimal */
check_sexa:
		while (value < end - 2) {
			if (*value == '.') {
				type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL;
				goto check_float;
			}
			if (*value != ':') {
				goto not_numeric;
			}
			*ptr++ = *value++;
			if (*(value + 1) == ':') {
				if (*value >= '0' && *value <= '9') {
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			} else {
				if ((*value >= '0' && *value <= '5') &&
						(*(value + 1) >= '0' && *(value + 1) <= '9')) {
					*ptr++ = *value++;
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			}
		}
		if (*value == '.') {
			type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL;
			goto check_float;
		} else if (value == end) {
			type = Y_SCALAR_IS_INT | Y_SCALAR_IS_SEXAGECIMAL;
		} else {
			goto not_numeric;
		}

	} else if (*value == '.') {
		/* float */
		*ptr++ = '0';
check_float:
		*ptr++ = *value++;
		if (value == end) {
			/* don't treat strings ending with a period as numbers */
			/* mostly here to catch the degenerate case of `.` as input */
			goto not_numeric;
		}
		if (type == (Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL)) {
			/* sexagecimal float */
			while (value < end && (*(end - 1) == '_' || *(end - 1) == '0')) {
				end--;
			}
			if (value == end) {
				*ptr++ = '0';
			}
			while (value < end) {
				if (*value == '_') {
					value++;
				} else if (*value >= '0' && *value <= '9') {
					*ptr++ = *value++;
				} else {
					goto not_numeric;
				}
			}
		} else {
			/* decimal float */
			int is_exp = 0;
			while (value < end) {
				if (*value == '_') {
					value++;
				} else if (*value >= '0' && *value <= '9') {
					*ptr++ = *value++;
				} else if (*value == 'E' || *value == 'e') {
					/* exponential */
					is_exp = 1;
					*ptr++ = *value++;
					if (value == end || (*value != '+' && *value != '-')) {
						goto not_numeric;
					}
					*ptr++ = *value++;
					if (value == end || *value < '0' || *value > '9' ||
							(*value == '0' && value + 1 == end)) {
						goto not_numeric;
					}
					*ptr++ = *value++;
					while (value < end) {
						if (*value >= '0' && *value <= '9') {
							*ptr++ = *value++;
						} else {
							goto not_numeric;
						}
					}
				} else {
					goto not_numeric;
				}
			}
			/* trim */
			if (!is_exp) {
				while (*(ptr - 1) == '0') {
					ptr--;
				}
				if (*(ptr - 1) == '.') {
					*ptr++ = '0';
				}
			}
			type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_DECIMAL;
		}

	} else {
		goto not_numeric;
	}

	/* terminate */
	*ptr = '\0';

finish:
	/* convert & assign */
	if ((type & Y_SCALAR_IS_INT) && lval != NULL) {
		switch (type & Y_SCALAR_FORMAT_MASK) {
		case Y_SCALAR_IS_BINARY:
			ptr = buf + 2;
			if (*ptr == 'b') {
				ptr++;
			}
			*lval = strtol(ptr, (char **) NULL, 2);
			if (*buf == '-') {
				*lval *= -1L;
			}
			break;
		case Y_SCALAR_IS_OCTAL:
			*lval = strtol(buf, (char **) NULL, 8);
			break;
		case Y_SCALAR_IS_HEXADECIMAL:
			*lval = strtol(buf, (char **) NULL, 16);
			break;
		case Y_SCALAR_IS_SEXAGECIMAL:
			*lval = php_yaml_eval_sexagesimal_l(0, buf, ptr);
			if (*buf == '-') {
				*lval *= -1L;
			}
			break;
		default:
			*lval = atol(buf);
		}
	} else if ((type & Y_SCALAR_IS_FLOAT) && dval != NULL) {
		switch (type & Y_SCALAR_FORMAT_MASK) {
		case Y_SCALAR_IS_SEXAGECIMAL:
			*dval = php_yaml_eval_sexagesimal_d(0.0, buf, ptr);
			if (*buf == '-') {
				*dval *= -1.0;
			}
			break;
		case Y_SCALAR_IS_INFINITY_P:
			*dval = php_get_inf();
			break;
		case Y_SCALAR_IS_INFINITY_N:
			*dval = -php_get_inf();
			break;
		case Y_SCALAR_IS_NAN:
			*dval = php_get_nan();
			break;
		default:
			*dval = atof(buf);
		}
	}
	if (buf != NULL) {
		if (str != NULL) {
			*str = buf;
		} else {
			efree(buf);
		}
	}

	/* return */
	return type;

return_zero:
	if (lval != NULL) {
		*lval = 0;
	}
	if (dval != NULL) {
		*dval = 0.0;
	}
	if (buf != NULL) {
		efree(buf);
	}
	return (Y_SCALAR_IS_INT | Y_SCALAR_IS_ZERO);

not_numeric:
	if (buf != NULL) {
		efree(buf);
	}
	return Y_SCALAR_IS_NOT_NUMERIC;
}

/* }}} */

/* {{{ php_yaml_scalar_is_timestamp(const char *,size_t)
 * Does this scalar encode a TIMESTAMP value?
 *
 * specification is found at http://yaml.org/type/timestamp.html.
 */
static int php_yaml_scalar_is_timestamp(const char *value, size_t length)
{
	const char *ptr = value;
	const char *end = value + length;
	const char *pos1, *pos2;

	/* skip leading space */
	ts_skip_space();

	/* check 4 digit year and separator */
	pos1 = pos2 = ptr;
	ts_skip_number();
	if (ptr == pos1 || ptr == end || ptr - pos2 != 4 || *ptr != '-') {
		return 0;
	}

	/* check 1 or 2 month and separator */
	pos2 = ++ptr;
	ts_skip_number();
	if (ptr == pos2 || ptr == end || ptr - pos2 > 2 || *ptr != '-') {
		return 0;
	}

	/* check 1 or 2 digit day */
	pos2 = ++ptr;
	ts_skip_number();
	if (ptr == pos2 || ptr - pos2 > 2) {
		return 0;
	}

	/* check separator */
	pos2 = ptr;
	if (ptr == end) {
		/* date only format requires YYYY-MM-DD */
		return (pos2 - pos1 == 10) ? 1 : 0;
	}
	/* time separator is T or whitespace */
	if (*ptr == 'T' || *ptr == 't') {
		ptr++;
	} else {
		ts_skip_space();
	}

	/* check 1 or 2 digit hour and separator */
	pos1 = ptr;
	ts_skip_number();
	if (ptr == pos1 || ptr == end || ptr - pos1 > 2 || *ptr != ':') {
		return 0;
	}

	/* check 2 digit minute and separator */
	pos1 = ++ptr;
	ts_skip_number();
	if (ptr == end || ptr - pos1 != 2 || *ptr != ':') {
		return 0;
	}

	/* check 2 digit second */
	pos1 = ++ptr;
	ts_skip_number();
	if (ptr == end) {
		return (ptr - pos1 == 2) ? 1 : 0;
	}

	/* check optional fraction */
	if (*ptr == '.') {
		ptr++;
		ts_skip_number();
	}

	/* skip optional separator space */
	ts_skip_space();
	if (ptr == end) {
		return 1;
	}

	/* check time zone */
	if (*ptr == 'Z') {
		ptr++;
		ts_skip_space();
		return (ptr == end) ? 1 : 0;
	}

	/* check time zone offset sign */
	if (*ptr != '+' && *ptr != '-') {
		return 0;
	}
	/* check 1 or 2 digit time zone hour */
	pos1 = ++ptr;
	ts_skip_number();
	if (ptr == pos1 || ptr - pos1 == 3 || ptr - pos1 > 4) {
		return 0;
	}
	if (ptr == end) {
		return 1;
	}

	/* optional time zone minute */
	if (*ptr != ':') {
		return 0;
	}
	pos1 = ++ptr;
	ts_skip_number();
	if (ptr - pos1 != 2) {
		return 0;
	}

	/* skip following space */
	ts_skip_space();
	return (ptr == end) ? 1 : 0;
}

/* }}} */

/* {{{ php_yaml_detect_scalar_type(const char *, size_t, yaml_event_t)
 * Guess what datatype the scalar encodes
 */
char *php_yaml_detect_scalar_type(const char *value, size_t length,
		const yaml_event_t * event)
{
	int flags = 0;
	long lval = 0;
	double dval = 0.0;

	/* is value a null? */
	if (0 == length || php_yaml_scalar_is_null(value, length, event)) {
		return YAML_NULL_TAG;
	}

	/* is value numeric? */
	flags = php_yaml_scalar_is_numeric(value, length, &lval, &dval, NULL);
	if (flags != Y_SCALAR_IS_NOT_NUMERIC) {
		return (flags & Y_SCALAR_IS_FLOAT) ? YAML_FLOAT_TAG : YAML_INT_TAG;
	}

	/* is value boolean? */
	flags = php_yaml_scalar_is_bool(value, length, event);
	if (-1 != flags) {
		return YAML_BOOL_TAG;
	}

	/* is value a timestamp? */
	if (php_yaml_scalar_is_timestamp(value, length)) {
		return YAML_TIMESTAMP_TAG;
	}

	/* no guess */
	return NULL;
}

/* }}} */

/* {{{ php_yaml_eval_sexagesimal_l()
 * Convert a base 60 number to a long
 */
static long php_yaml_eval_sexagesimal_l(long lval, char *sg, char *eos)
{
	char *ep;
	while (sg < eos && (*sg < '0' || *sg > '9')) {
		sg++;
	}
	ep = sg;
	while (ep < eos && *ep >= '0' && *ep <= '9') {
		ep++;
	}
	if (sg == eos) {
		return lval;
	}
	return php_yaml_eval_sexagesimal_l(
			lval * 60 + strtol(sg, (char **) NULL, 10), ep, eos);
}

/* }}} */

/* {{{ php_yaml_eval_sexagesimal_d()
 * Convert a base 60 number to a double
 */
static double php_yaml_eval_sexagesimal_d(double dval, char *sg, char *eos)
{
	char *ep;
	while (sg < eos && *sg != '.' && (*sg < '0' || *sg > '9')) {
		sg++;
	}
	ep = sg;
	while (ep < eos && *ep >= '0' && *ep <= '9') {
		ep++;
	}
	if (sg == eos || *sg == '.') {
		return dval;
	}
	return php_yaml_eval_sexagesimal_d(
			dval * 60.0 + strtod(sg, (char **) NULL), ep, eos);
}

/* }}} */

/* {{{ php_yaml_eval_timestamp()
 * Convert a timestamp
 *
 * This is switched on/off by the `yaml.decode_timestamp` ini setting.
 *  - yaml.decode_timestamp=0 for no timestamp parsing
 *  - yaml.decode_timestamp=1 for strtotime parsing
 *  - yaml.decode_timestamp=2 for date_create parsing
 */
static int
php_yaml_eval_timestamp(zval ** zpp, char *ts, int ts_len TSRMLS_DC)
{
	if (YAML_G(timestamp_decoder) != NULL ||
			1L == YAML_G(decode_timestamp) ||
			2L == YAML_G(decode_timestamp)) {
		zval **argv[] = { NULL };
		zval *arg, *retval, *func, afunc;
		char *funcs[] = { "strtotime", "date_create" };

		INIT_ZVAL(afunc);
		if (NULL == YAML_G(timestamp_decoder)) {
			if (2L == YAML_G(decode_timestamp)) {
				ZVAL_STRING(&afunc, funcs[1], 0);
			} else {
				ZVAL_STRING(&afunc, funcs[0], 0);
			}
			func = &afunc;
		} else {
			func = YAML_G(timestamp_decoder);
		}

		MAKE_STD_ZVAL(arg);
#ifdef ZEND_ENGINE_2
		ZVAL_STRINGL(arg, ts, ts_len, 1);
#else
		{
			/* fix timestamp format for PHP4 */
			char *buf, *dst, *end, *src;

			buf = (char *) emalloc((size_t) ts_len + 1);
			dst = buf;
			end = ts + ts_len;
			src = ts;

			while (src < end && *src != '.') {
				if (src + 1 < end &&
						(*(src - 1) >= '0' && *(src - 1) <= '9') && 
						(*src == 'T' || *src == 't') && 
						(*(src + 1) >= '0' && *(src + 1) <= '9')) {
					src++;
					*dst++ = ' ';
				} else if (*src == ':' && src > ts + 2 && (
						((*(src - 2) == '+' || *(src - 2) == '-') && 
						 (*(src - 1) >= '0' || *(src - 1) <= '5')) ||
						((*(src - 3) == '+' || *(src - 3) == '-') &&
						 (*(src - 2) >= '0' || *(src - 2) <= '5') &&
						 (*(src - 1) >= '0' || *(src - 1) <= '9')))) {
					src++;
				} else {
					*dst++ = *src++;
				}
			}
			if (src < end && *src == '.') {
				src++;
				while (src < end && *src >= '0' && *src <= '9') {
					src++;
				}
			}
			while (src < end) {
				if (*src == ':' && src > ts + 2 && (
						((*(src - 2) == '+' || *(src - 2) == '-') &&
						 (*(src - 1) >= '0' || *(src - 1) <= '5')) ||
						((*(src - 3) == '+' || *(src - 3) == '-') &&
						 (*(src - 2) >= '0' || *(src - 2) <= '5') &&
						 (*(src - 1) >= '0' || *(src - 1) <= '9')))) {
					src++;
				} else {
					*dst++ = *src++;
				}
			}
			*dst = '\0';

			ZVAL_STRINGL(arg, buf, dst - buf, 0);
		}
#endif
		argv[0] = &arg;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL, func,
				&retval, 1, argv, 0, NULL TSRMLS_CC) || NULL == retval) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to evaluate string '%s' as timestamp", ts);
			zval_ptr_dtor(&arg);
			return FAILURE;

		} else {
			zval_ptr_dtor(&arg);
			zval_dtor(*zpp);
			ZVAL_ZVAL(*zpp, retval, 1, 1);
			return SUCCESS;
		}
	} else {
		zval_dtor(*zpp);
#ifdef IS_UNICODE
		ZVAL_U_STRINGL(UG(utf8_conv), *zpp, ts, ts_len, 1);
#else
		ZVAL_STRINGL(*zpp, ts, ts_len, 1);
#endif
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
