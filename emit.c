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
#define y_event_init_failed(e) \
  yaml_event_delete(e); \
  php_error_docref(NULL TSRMLS_CC, E_WARNING,\
	  "Memory error: Not enough memory for creating an event (libyaml)")

#define Y_ARRAY_SEQUENCE 1
#define Y_ARRAY_MAP 2

/* }}} */

/* {{{ local prototypes
 */
static int y_event_emit(
		y_emit_state_t *state, yaml_event_t *event TSRMLS_DC);
static void y_handle_emitter_error(const y_emit_state_t *state TSRMLS_DC);
static int y_array_is_sequence(HashTable *ht TSRMLS_DC);
static void y_scan_recursion(y_emit_state_t *state, zval *data TSRMLS_DC);
static long y_search_recursive(
		y_emit_state_t *state, const unsigned long addr TSRMLS_DC);

static int y_write_zval(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_null(y_emit_state_t *state TSRMLS_DC);
static int y_write_bool(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_long(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_double(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_string(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_array(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_timestamp(y_emit_state_t *state, zval *data TSRMLS_DC);
static int y_write_object(y_emit_state_t *state, zval *data TSRMLS_DC);

/* }}} */


/* {{{ y_event_emit()
 * send an event to the emitter
 */
static int
y_event_emit(y_emit_state_t *state, yaml_event_t *event TSRMLS_DC)
{
	if (!yaml_emitter_emit(state->emitter, event)) {
		yaml_event_delete(event);
		y_handle_emitter_error(state TSRMLS_CC);
		return FAILURE;

	} else {
		return SUCCESS;
	}
}
/* }}} */


/* {{{ y_handle_emitter_error()
 * Emit a warning about an emitter error
 */
static void y_handle_emitter_error(const y_emit_state_t *state TSRMLS_DC)
{
	switch (state->emitter->error) {
	case YAML_MEMORY_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Memory error: Not enough memory for emitting");
		break;

	case YAML_WRITER_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Writer error: %s", state->emitter->problem);
		break;

	case YAML_EMITTER_ERROR:
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Emitter error: %s", state->emitter->problem);
		break;

	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Internal error");
		break;
	}
}
/* }}} */


/* {{{ y_array_is_sequence()
 * Does the array encode a sequence?
 */
static int y_array_is_sequence(HashTable *ht TSRMLS_DC)
{
	HashPosition pos;
	ulong kidx, idx;
	char *kstr;
	int key_type;

	zend_hash_internal_pointer_reset_ex(ht, &pos);
	idx = 0;
	while (SUCCESS == zend_hash_has_more_elements_ex(ht, &pos)) {
		key_type = zend_hash_get_current_key_ex(
				ht, (char **) &kstr, NULL, &kidx, 0, &pos);
		if (HASH_KEY_IS_LONG != key_type) {
			/* non-numeric key found */
			return Y_ARRAY_MAP;

		} else if (kidx != idx) {
			/* gap in sequence found */
			return Y_ARRAY_MAP;
		}

		idx++;
		zend_hash_move_forward_ex(ht, &pos);
	};
	return Y_ARRAY_SEQUENCE;
}
/* }}} */


/* {{{ y_scan_recursion()
 * walk an object graph looking for recursive references
 */
static void y_scan_recursion(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	HashTable *ht;
	HashPosition pos;
	zval **elm;

	ht = HASH_OF(data);

	if (!ht) {
		/* data isn't an array or object, so we're done */
		return;
	}

	if (ht->nApplyCount > 0) {
		zval *tmp;
		MAKE_STD_ZVAL(tmp);
		ZVAL_LONG(tmp, (unsigned long) ht);

		/* we've seen this before, so record address */
		zend_hash_next_index_insert(
				state->recursive, &tmp, sizeof(zval *), NULL);
		return;
	}

	ht->nApplyCount++;
	zend_hash_internal_pointer_reset_ex(ht, &pos);
	while (SUCCESS == zend_hash_has_more_elements_ex(ht, &pos)) {
		zend_hash_get_current_data_ex(ht, (void **) &elm, &pos);
		y_scan_recursion(state, (*elm) TSRMLS_CC);
		zend_hash_move_forward_ex(ht, &pos);
	};
	ht->nApplyCount--;

	return;
}
/* }}} */


/* {{{ y_search_recursive()
 * Search the recursive state hash for an address
 */
static long y_search_recursive(
		y_emit_state_t *state, const unsigned long addr TSRMLS_DC)
{
 	zval **entry;
	HashPosition pos;
   	ulong num_key;
	unsigned long found;

	zend_hash_internal_pointer_reset_ex(state->recursive, &pos);
	while (SUCCESS == zend_hash_has_more_elements_ex(state->recursive, &pos)) {
		zend_hash_get_current_data_ex(state->recursive, (void **)&entry, &pos);
		found = (unsigned long) Z_LVAL_PP(entry);
		if (addr == found) {
			zend_hash_get_current_key_ex(
					state->recursive, NULL, NULL, &num_key, 0, &pos);
			return num_key;
		}
		zend_hash_move_forward_ex(state->recursive, &pos);
	}
	return -1;
}
/* }}} */


/* {{{ y_write_zval()
 * Write a php zval to the emitter
 */
static int y_write_zval(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	int status = FAILURE;

	switch (Z_TYPE_P(data)) {
	case IS_NULL:
		status = y_write_null(state TSRMLS_CC);
		break;

	case IS_BOOL:
		status = y_write_bool(state, data TSRMLS_CC);
		break;

	case IS_LONG:
		status = y_write_long(state, data TSRMLS_CC);
		break;

	case IS_DOUBLE:
		status = y_write_double(state, data TSRMLS_CC);
		break;

	case IS_STRING:
		status = y_write_string(state, data TSRMLS_CC);
		break;

	case IS_ARRAY:
		status = y_write_array(state, data TSRMLS_CC);
		break;

	case IS_OBJECT:
		status = y_write_object(state, data TSRMLS_CC);
		break;

	case IS_RESOURCE:		/* unsupported object */
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
				"Unable to emit PHP resources.");
		break;

	default:				/* something we didn't think of */
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
				"Unsupported php zval type %d.", Z_TYPE_P(data));
		break;
	}

	return status;
}
/* }}} */


/* {{{ y_write_null()
 */
static int y_write_null(y_emit_state_t *state TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	
	status = yaml_scalar_event_initialize(&event,
			NULL, (yaml_char_t *) YAML_NULL_TAG,
			(yaml_char_t *) "~", strlen("~"),
			1, 1, YAML_PLAIN_SCALAR_STYLE);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_bool()
 */
static int y_write_bool(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	char *res = Z_BVAL_P(data) ? "true" : "false";

	status = yaml_scalar_event_initialize(&event,
			NULL, (yaml_char_t *) YAML_BOOL_TAG,
			(yaml_char_t *) res, strlen(res),
			1, 1, YAML_PLAIN_SCALAR_STYLE);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_long()
 */
static int y_write_long(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	char *res = { 0 };
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
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_double()
 */
static int y_write_double(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	char *res = { 0 };
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
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_string()
 */
static int y_write_string(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	yaml_scalar_style_t style = YAML_PLAIN_SCALAR_STYLE;

	if (NULL != detect_scalar_type(Z_STRVAL_P(data), Z_STRLEN_P(data), NULL)) {
		/* looks like some other type to us, make sure it's quoted */
		style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;

	} else {
		const char *ptr, *end;
		end = Z_STRVAL_P(data) + Z_STRLEN_P(data);

		/* TODO: need more test cases for this sniffing */
		for (ptr = Z_STRVAL_P(data); ptr != end; ptr++) {
			if ('\n' == *ptr) {
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
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_array()
 */
static int y_write_array(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	HashTable *ht = Z_ARRVAL_P(data);
	HashPosition pos;
	zval **elm;
	int array_type;
	zval key_zval;
	ulong kidx;
	uint key_len;
	char *kstr = { 0 };
	HashTable *tmp_ht;
	long recursive_idx = -1;
	char *anchor = { 0 };
	size_t anchor_size;

	array_type = y_array_is_sequence(ht TSRMLS_CC);

	/* syck does a check to see if the array is small and all scalars
	 * if it is then it outputs in inline form
	 */

	/* search state->recursive for this ht.
	 * if it exists:
	 *     anchor = "id%04d" % index
	 *   if ht->nApplyCount > 0:
	 *     emit a ref 
	 */
	recursive_idx = y_search_recursive(state, (unsigned long) ht TSRMLS_CC);
	if (-1 != recursive_idx) {
		/* create anchor to refer to this structure */
		anchor_size = snprintf(anchor, 0, "refid%ld", recursive_idx + 1);
		anchor = emalloc(anchor_size + 1);
		snprintf(anchor, anchor_size + 1, "refid%ld", recursive_idx + 1);

		if (ht->nApplyCount > 1) {
			/* node has been visited before */
			status = yaml_alias_event_initialize(
					&event, (yaml_char_t *) anchor);
			if (!status) {
				y_event_init_failed(&event);
				efree(anchor);
				return FAILURE;
			}

			status = y_event_emit(state, &event TSRMLS_CC);
			efree(anchor);
			return status;
		}
	}

	if (Y_ARRAY_SEQUENCE == array_type) {
		status = yaml_sequence_start_event_initialize(&event,
				(yaml_char_t *) anchor, (yaml_char_t *) YAML_SEQ_TAG, 1,
				YAML_ANY_SEQUENCE_STYLE);
	} else {
		status = yaml_mapping_start_event_initialize(&event,
				(yaml_char_t *) anchor, (yaml_char_t *) YAML_MAP_TAG, 1,
				YAML_ANY_MAPPING_STYLE);
	}

	if (!status) {
		y_event_init_failed(&event);
		if (anchor) {
			efree(anchor);
		}
		return FAILURE;
	}
	status = y_event_emit(state, &event TSRMLS_CC);
	if (anchor) {
		efree(anchor);
	}
	if (FAILURE == status) {
		return FAILURE;
	}

	/* emit array elements */
	zend_hash_internal_pointer_reset_ex(ht, &pos);
	while (SUCCESS == zend_hash_has_more_elements_ex(ht, &pos)) {
		if (Y_ARRAY_MAP == array_type) {
			zend_hash_get_current_key_ex(ht, (char **) &kstr, &key_len, &kidx,
					0, &pos);

			/* create zval for key */
			if (HASH_KEY_IS_LONG == 
					zend_hash_get_current_key_type_ex(ht, &pos)) {
				ZVAL_LONG(&key_zval, kidx);

			} else {
				ZVAL_STRINGL(&key_zval, kstr, strlen(kstr), 1);
			}

			/* emit key */
			status = y_write_zval(state, &key_zval TSRMLS_CC);
			zval_dtor(&key_zval);
			if (SUCCESS != status) {
				return FAILURE;
			}
		}

		if (SUCCESS == 
				zend_hash_get_current_data_ex(ht, (void **) &elm, &pos)) {

			tmp_ht = HASH_OF(*elm);
			if (tmp_ht) {
				/* increment access counted for hash */
				tmp_ht->nApplyCount++;
			}

			status = y_write_zval(state, (*elm) TSRMLS_CC);

			if (tmp_ht) {
				tmp_ht->nApplyCount--;
			}

			if (SUCCESS != status) {
				return FAILURE;
			}
		}
		zend_hash_move_forward_ex(ht, &pos);
	};

	if (Y_ARRAY_SEQUENCE == array_type) {
		status = yaml_sequence_end_event_initialize(&event);
	} else {
		status = yaml_mapping_end_event_initialize(&event);
	}

	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* y_write_timestamp()
 */
static int y_write_timestamp(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	zend_class_entry *clazz = Z_OBJCE_P(data);
	zval *timestamp = { 0 };
	zval dtfmt;

	/* get iso-8601 format specifier */
#if ZEND_MODULE_API_NO >= 20071006
	zend_get_constant_ex("DateTime::ISO8601", 17, &dtfmt, clazz, 0 TSRMLS_CC);
#else
	zend_get_constant_ex("DateTime::ISO8601", 17, &dtfmt, clazz TSRMLS_CC);
#endif
	/* format date as iso-8601 string */
	zend_call_method_with_1_params(&data, clazz, NULL,
			"format", &timestamp, &dtfmt);
	zval_dtor(&dtfmt);

	/* emit formatted date */
	status = yaml_scalar_event_initialize(&event,
			NULL, (yaml_char_t *) YAML_TIMESTAMP_TAG,
			(yaml_char_t *) Z_STRVAL_P(timestamp), Z_STRLEN_P(timestamp),
			1, 1, YAML_PLAIN_SCALAR_STYLE);
	zval_dtor(timestamp);
	efree(timestamp);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_object()
 */
static int y_write_object(y_emit_state_t *state, zval *data TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	char *clazz_name = { 0 };
	zend_uint name_len;
	zend_class_entry *clazz;

	clazz = Z_OBJCE_P(data);
	zend_get_object_classname(data, &clazz_name, &name_len TSRMLS_CC);

	if (0 == strncmp(clazz_name, "DateTime", name_len)) {
		status = y_write_timestamp(state, data TSRMLS_CC);

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

		smart_str_free(&buf);
		if (!status) {
			y_event_init_failed(&event);
			status = FAILURE;
		} else {
			status = y_event_emit(state, &event TSRMLS_CC);
		}
	}

	efree(clazz_name);
	return status;
}
/* }}} */


/* {{{ php_yaml_write_impl()
 * Common stream writing logic shared by yaml_emit and yaml_emit_file.
 */
int
php_yaml_write_impl(
		yaml_emitter_t *emitter, zval *data,
		yaml_encoding_t encoding TSRMLS_DC)
{
	y_emit_state_t state;
	yaml_event_t event;
	int status;

	state.emitter = emitter;
	/* scan for recursive objects */
	ALLOC_HASHTABLE(state.recursive);
	zend_hash_init(state.recursive, 8, NULL, NULL, 0);
	y_scan_recursion(&state, data TSRMLS_CC);


	/* start stream */
	status = yaml_stream_start_event_initialize(&event, encoding);
	if (!status) {
		y_event_init_failed(&event);
		status = FAILURE;
		goto cleanup;
	}
	if (FAILURE == y_event_emit(&state, &event TSRMLS_CC)) {
		status = FAILURE;
		goto cleanup;
	}

	/* start document */
	status = yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0);
	if (!status) {
		y_event_init_failed(&event);
		status = FAILURE;
		goto cleanup;
	}
	if (FAILURE == y_event_emit(&state, &event TSRMLS_CC)) {
		status = FAILURE;
		goto cleanup;
	}

	/* output data */
	if (FAILURE == y_write_zval(&state, data TSRMLS_CC)) {
		status = FAILURE;
		goto cleanup;
	}

	/* end document */
	status = yaml_document_end_event_initialize(&event, 0);
	if (!status) {
		y_event_init_failed(&event);
		status = FAILURE;
		goto cleanup;
	}
	if (FAILURE == y_event_emit(&state, &event TSRMLS_CC)) {
		status = FAILURE;
		goto cleanup;
	}

	/* end stream */
	status = yaml_stream_end_event_initialize(&event);
	if (!status) {
		y_event_init_failed(&event);
		status = FAILURE;
		goto cleanup;
	}
	if (FAILURE == y_event_emit(&state, &event TSRMLS_CC)) {
		status = FAILURE;
		goto cleanup;
	}

	yaml_emitter_flush(state.emitter);
	status = SUCCESS;

cleanup:
	zend_hash_destroy(state.recursive);
	FREE_HASHTABLE(state.recursive);

	return status;
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
