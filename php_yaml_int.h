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


#ifndef PHP_YAML_INT_H
#define PHP_YAML_INT_H

#ifdef __cplusplus
extern "C" {
#endif

/* {{{ backcompat macros
*/
#ifndef ZVAL_OPT_DEREF
/* Taken from php-src/Zned/zend_types.h @ 95ff285 */
#define Z_OPT_ISREF(zval) (Z_OPT_TYPE(zval) == IS_REFERENCE)
#define Z_OPT_ISREF_P(zval_p) Z_OPT_ISREF(*(zval_p))
#define ZVAL_OPT_DEREF(z) do {           \
	if (UNEXPECTED(Z_OPT_ISREF_P(z))) {  \
		(z) = Z_REFVAL_P(z);             \
	}                                    \
} while (0)
#endif

#ifndef TSRMLS_DC
#define TSRMLS_DC
#endif
#ifndef TSRMLS_CC
#define TSRMLS_CC
#endif
/* }}} */

/* {{{ ext/yaml types
*/
typedef void (*eval_scalar_func_t)(yaml_event_t event, HashTable *callbacks, zval *retval TSRMLS_DC);

typedef struct parser_state_s {
	yaml_parser_t parser;
	yaml_event_t event;
	int have_event;
	zval aliases;
	eval_scalar_func_t eval_func;
	HashTable *callbacks;
} parser_state_t;

typedef struct y_emit_state_s {
	yaml_emitter_t *emitter;
	HashTable *recursive;
	HashTable *callbacks;
} y_emit_state_t;

/* }}} */


/* {{{ ext/yaml macros
*/
#define YAML_BINARY_TAG      "tag:yaml.org,2002:binary"
#define YAML_MERGE_TAG       "tag:yaml.org,2002:merge"
#define YAML_PHP_TAG         "!php/object"
#define YAML_NONSPECIFIC_TAG "!"

#define Y_SCALAR_IS_NOT_NUMERIC 0x00
#define Y_SCALAR_IS_INT         0x10
#define Y_SCALAR_IS_FLOAT       0x20
#define Y_SCALAR_IS_ZERO        0x00
#define Y_SCALAR_IS_BINARY      0x01
#define Y_SCALAR_IS_OCTAL       0x02
#define Y_SCALAR_IS_DECIMAL     0x03
#define Y_SCALAR_IS_HEXADECIMAL 0x04
#define Y_SCALAR_IS_SEXAGECIMAL 0x05
#define Y_SCALAR_IS_INFINITY_P  0x06
#define Y_SCALAR_IS_INFINITY_N  0x07
#define Y_SCALAR_IS_NAN         0x08
#define Y_SCALAR_FORMAT_MASK    0x0F


#if (PHP_MAJOR_VERSION > 5) || ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 3))
#	define ZEND_IS_CALLABLE(a,b,c) zend_is_callable((a), (b), (c) TSRMLS_CC)
#else
#	define ZEND_IS_CALLABLE(a,b,c) zend_is_callable((a), (b), (c))
#endif

#define STR_EQ(a, b)\
	(a != NULL && b != NULL && 0 == strcmp(a, b))

#define SCALAR_TAG_IS(event, name) \
	STR_EQ(name, (const char *)event.data.scalar.tag)

#define IS_NOT_IMPLICIT(event) \
	(!event.data.scalar.quoted_implicit && !event.data.scalar.plain_implicit)

#define IS_NOT_IMPLICIT_AND_TAG_IS(event, name) \
	(IS_NOT_IMPLICIT(event) && SCALAR_TAG_IS(event, name))

#define IS_NOT_QUOTED(event) \
	(YAML_PLAIN_SCALAR_STYLE == event.data.scalar.style || YAML_ANY_SCALAR_STYLE == event.data.scalar.style)

#define IS_NOT_QUOTED_OR_TAG_IS(event, name) \
	(IS_NOT_QUOTED(event) && (event.data.scalar.plain_implicit || SCALAR_TAG_IS(event, name)))

#define SCALAR_IS_QUOTED(event) \
	(YAML_SINGLE_QUOTED_SCALAR_STYLE == event.data.scalar.style || YAML_DOUBLE_QUOTED_SCALAR_STYLE == event.data.scalar.style)

/* }}} */

/* {{{ ext/yaml prototypes
*/
void php_yaml_read_all(parser_state_t *state, zend_long *ndocs, zval *retval TSRMLS_DC);

void php_yaml_read_partial(
		parser_state_t *state, zend_long pos, zend_long *ndocs, zval *retval TSRMLS_DC);

void eval_scalar(yaml_event_t event,
		HashTable * callbacks, zval *retval TSRMLS_DC);

void eval_scalar_with_callbacks(
		yaml_event_t event, HashTable *callbacks, zval *retval TSRMLS_DC);

const char *detect_scalar_type(
		const char *value, size_t length, const yaml_event_t *event);

int scalar_is_null(
		const char *value, size_t length, const yaml_event_t *event);

int scalar_is_bool(
		const char *value, size_t length, const yaml_event_t *event);

int scalar_is_numeric(
		const char *value, size_t length, zend_long *lval, double *dval, char **str);

int scalar_is_timestamp(const char *value, size_t length);

int php_yaml_write_impl(yaml_emitter_t *emitter, zval *data,
		yaml_encoding_t encoding, HashTable *callbacks TSRMLS_DC);

int php_yaml_write_to_buffer(
		void *data, unsigned char *buffer, size_t size);

/* }}} */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* PHP_YAML_INT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
