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
#define event_emit(e) \
  if (!yaml_emitter_emit(emitter, e)) { \
    goto emitter_error; \
  }

/* }}} */

/* {{{ local prototypes
 */
static void php_yaml_handle_emitter_error(
		const yaml_emitter_t * emitter TSRMLS_DC);

static int php_yaml_write_zval(
		yaml_emitter_t * emitter, zval * data TSRMLS_DC);

/* }}} */

/* {{{ php_yaml_handle_emitter_error()
 * Emit a warning about an emitter error
 */
static void
php_yaml_handle_emitter_error(const yaml_emitter_t * emitter TSRMLS_DC)
{
	switch (emitter->error) {
	case YAML_MEMORY_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Memory error: Not enough memory for emitting");
		break;

	case YAML_WRITER_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Writer error: %s", emitter->problem);
		break;

	case YAML_EMITTER_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Emitter error: %s", emitter->problem);
		break;

	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Internal error");
		break;
	}
}
/* }}} */


/* {{{ php_yaml_array_is_sequence()
 * Does the array encode a sequence?
 */
static int php_yaml_array_is_sequence(HashTable * a)
{
	ulong kidx, idx = 0;
	char *kstr;
	int key_type;

	zend_hash_internal_pointer_reset(a);
	while (SUCCESS == zend_hash_has_more_elements(a)) {
		key_type = zend_hash_get_current_key(a, (char **) &kstr, &kidx, 0);
		if (HASH_KEY_IS_LONG != key_type) {
			/* non-numeric key found */
			return 1;

		} else if (kidx != idx) {
			/* gap in sequence found */
			return 1;
		}

		idx++;
		zend_hash_move_forward(a);
	};
	return 0;
}
/* }}} */


/* {{{ php_yaml_write_zval()
 * Write a php zval to the emitter
 */
static int
php_yaml_write_zval(yaml_emitter_t * emitter, zval * data TSRMLS_DC)
{
	yaml_event_t event;
	char *res;
	int status;

	memset(&event, 0, sizeof(event));
	res = NULL;

	switch (Z_TYPE_P(data)) {
	case IS_NULL:
		status = yaml_scalar_event_initialize(&event,
				NULL, (yaml_char_t *) YAML_NULL_TAG,
				(yaml_char_t *) "~", strlen("~"),
				1, 1, YAML_PLAIN_SCALAR_STYLE);
		if (!status) {
			goto event_error;
		}
		event_emit(&event);
		break;

	case IS_BOOL:
		{
			res = Z_BVAL_P(data) ? "true" : "false";
			status = yaml_scalar_event_initialize(&event,
					NULL, (yaml_char_t *) YAML_BOOL_TAG,
					(yaml_char_t *) res, strlen(res),
					1, 1, YAML_PLAIN_SCALAR_STYLE);
			if (!status) {
				goto event_error;
			}
			event_emit(&event);
		}
		break;

	case IS_LONG:
		{
			size_t res_size;

			res_size = snprintf(res, 0, "%ld", Z_LVAL_P(data));
			res = emalloc(res_size + 1);
			snprintf(res, res_size + 1, "%ld", Z_LVAL_P(data));
			status = yaml_scalar_event_initialize(&event,
					NULL, (yaml_char_t *) YAML_INT_TAG,
					(yaml_char_t *) res, strlen(res),
					1, 1, YAML_PLAIN_SCALAR_STYLE);
			efree(res);
			if (!status) {
				goto event_error;
			}
			event_emit(&event);
		}
		break;

	case IS_DOUBLE:
		{
			size_t res_size;

			res_size = snprintf(res, 0, "%f", Z_DVAL_P(data));
			res = emalloc(res_size + 1);
			snprintf(res, res_size + 1, "%f", Z_DVAL_P(data));
			status = yaml_scalar_event_initialize(&event,
					NULL, (yaml_char_t *) YAML_FLOAT_TAG,
					(yaml_char_t *) res, strlen(res),
					1, 1, YAML_PLAIN_SCALAR_STYLE);
			efree(res);
			if (!status) {
				goto event_error;
			}
			event_emit(&event);
		}
		break;

	case IS_STRING:
		{
			yaml_scalar_style_t style = YAML_PLAIN_SCALAR_STYLE;
			const char *ptr, *end;
			end = Z_STRVAL_P(data) + Z_STRLEN_P(data);

			if (NULL != php_yaml_detect_scalar_type(Z_STRVAL_P(data),
							Z_STRLEN_P(data), NULL)) {
				/* looks like some other type to us, make sure it's quoted */
				style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;

			} else {
				/* TODO: need more test cases for this sniffing */
				for (ptr = Z_STRVAL_P(data); ptr != end; ptr++) {
					if (*ptr == '\n') {
						/* has newline(s), make sure they are preserved */
						style = YAML_LITERAL_SCALAR_STYLE;
						break;
					}
				}
			}

			status = yaml_scalar_event_initialize(&event,
					NULL, (yaml_char_t *) YAML_STR_TAG,
					(yaml_char_t *) Z_STRVAL_P(data), Z_STRLEN_P(data),
					1, 1, style);
			if (!status) {
				goto event_error;
			}
			event_emit(&event);
		}
		break;

	case IS_ARRAY:
		{
			HashTable *ht = Z_ARRVAL_P(data);
			zval **elm;
			/* syck does a check to see if the array is small and all scalars
			 * if it is then it outputs in inline form
			 */

			if (0 == php_yaml_array_is_sequence(ht)) {
				/* start sequence */
				status = yaml_sequence_start_event_initialize(&event,
						NULL, (yaml_char_t *) YAML_SEQ_TAG, 1,
						YAML_ANY_SEQUENCE_STYLE);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);

				/* emit array elements */
				zend_hash_internal_pointer_reset(ht);
				while (SUCCESS == zend_hash_has_more_elements(ht)) {
					if (SUCCESS == zend_hash_get_current_data(ht,
									(void **) &elm)) {
						status = php_yaml_write_zval(emitter,
								(*elm) TSRMLS_CC);
						if (SUCCESS != status)
							return FAILURE;
					}
					zend_hash_move_forward(ht);
				};

				/* end sequence */
				status = yaml_sequence_end_event_initialize(&event);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);

			} else {
				zval key_zval;
				ulong kidx;
				char *kstr = NULL;

				/* start map */
				status = yaml_mapping_start_event_initialize(&event,
						NULL, (yaml_char_t *) YAML_MAP_TAG, 1,
						YAML_ANY_MAPPING_STYLE);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);

				/* emit keys and values */
				zend_hash_internal_pointer_reset(ht);
				while (SUCCESS == zend_hash_has_more_elements(ht)) {
					zend_hash_get_current_key(
							ht, (char **) &kstr, &kidx, 0);

					/* create zval for key */
					if (HASH_KEY_IS_LONG ==
							zend_hash_get_current_key_type(ht)) {
						ZVAL_LONG(&key_zval, kidx);
					} else {
						ZVAL_STRINGL(&key_zval, kstr, strlen(kstr), 1);
					}

					/* emit key */
					status = php_yaml_write_zval(
							emitter, &key_zval TSRMLS_CC);
					zval_dtor(&key_zval);
					if (SUCCESS != status)
						return FAILURE;

					/* emit value */
					if (SUCCESS == zend_hash_get_current_data(ht,
									(void **) &elm)) {
						status = php_yaml_write_zval(emitter,
								(*elm) TSRMLS_CC);
						if (SUCCESS != status)
							return FAILURE;
					}

					zend_hash_move_forward(ht);
				}

				/* end map */
				status = yaml_mapping_end_event_initialize(&event);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);
			}
		}
		break;

	case IS_OBJECT:
		{
			char *clazz_name = NULL;
			zend_uint name_len;
			zend_class_entry *clazz;

			clazz = Z_OBJCE_P(data);
			zend_get_object_classname(data, &clazz_name,
					&name_len TSRMLS_CC);

			if (strncmp(clazz_name, "DateTime", name_len) == 0) {
				/* DateTime is encoded as timestamp */
				zval *retval = NULL;
				zval dtfmt;

				/* get iso-8601 format specifier */
#if ZEND_MODULE_API_NO >= 20071006
				zend_get_constant_ex("DateTime::ISO8601", 17, &dtfmt,
						clazz, 0 TSRMLS_CC);
#else
				zend_get_constant_ex("DateTime::ISO8601", 17, &dtfmt,
						clazz TSRMLS_CC);
#endif
				/* format date as iso-8601 string */
				zend_call_method_with_1_params(&data, clazz, NULL,
						"format", &retval, &dtfmt);
				zval_dtor(&dtfmt);

				/* emit formatted date */
				status = yaml_scalar_event_initialize(&event,
						NULL, (yaml_char_t *) YAML_TIMESTAMP_TAG,
						(yaml_char_t *) Z_STRVAL_P(retval),
						Z_STRLEN_P(retval), 1, 1, YAML_PLAIN_SCALAR_STYLE);
				zval_dtor(retval);
				efree(retval);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);

			} else {
				/* tag and emit serialized version of object */
				php_serialize_data_t var_hash;
				smart_str buf = { 0 };

				PHP_VAR_SERIALIZE_INIT(var_hash);
				php_var_serialize(&buf, &data, &var_hash TSRMLS_CC);
				PHP_VAR_SERIALIZE_DESTROY(var_hash);

				status = yaml_scalar_event_initialize(&event,
						NULL, (yaml_char_t *) YAML_PHP_TAG,
						(yaml_char_t *) buf.c, buf.len,
						0, 0, YAML_DOUBLE_QUOTED_SCALAR_STYLE);
				if (!status) {
					goto event_error;
				}
				event_emit(&event);
			}

			efree(clazz_name);
		}
		break;

	case IS_RESOURCE:
		/* unsupported object */
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
				"Unable to emit PHP resources.");
		break;

	default:
		/* something we didn't think of */
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
				"Unsupported php zval type %d.", Z_TYPE_P(data));
		break;
	}

	return SUCCESS;


emitter_error:
	php_yaml_handle_emitter_error(emitter TSRMLS_CC);
	yaml_event_delete(&event);
	return FAILURE;


event_error:
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Memory error: Not enough memory for creating an event (libyaml)");
	yaml_event_delete(&event);
	return FAILURE;
}
/* }}} */


/* {{{ php_yaml_write_impl()
 * Common stream writing logic shared by yaml_emit and yaml_emit_file.
 */
int
php_yaml_write_impl(yaml_emitter_t * emitter, zval * data,
		yaml_encoding_t encoding TSRMLS_DC)
{
	yaml_event_t event;
	int status;

	memset(&event, 0, sizeof(event));

	/* start stream */
	status = yaml_stream_start_event_initialize(&event, encoding);
	if (!status) {
		goto event_error;
	}
	event_emit(&event);

	/* start document */
	status = yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0);
	if (!status) {
		goto event_error;
	}
	event_emit(&event);

	/* output data */
	status = php_yaml_write_zval(emitter, data TSRMLS_CC);
	if (SUCCESS != status) {
		return FAILURE;
	}

	/* end document */
	status = yaml_document_end_event_initialize(&event, 0);
	if (!status) {
		goto event_error;
	}
	event_emit(&event);

	/* end stream */
	status = yaml_stream_end_event_initialize(&event);
	if (!status) {
		goto event_error;
	}
	event_emit(&event);

	yaml_emitter_flush(emitter);
	return SUCCESS;


emitter_error:
	php_yaml_handle_emitter_error(emitter TSRMLS_CC);
	yaml_event_delete(&event);
	return FAILURE;


event_error:
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Memory error: Not enough memory for creating an event (libyaml)");
	yaml_event_delete(&event);
	return FAILURE;
}
/* }}} */


/* {{{ php_yaml_write_to_buffer()
 * Emitter string buffer
 */
int
php_yaml_write_to_buffer(void *data, unsigned char *buffer, size_t size)
{
	smart_str_appendl((smart_str *) data, (char *) buffer, size);
	return 1;
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
