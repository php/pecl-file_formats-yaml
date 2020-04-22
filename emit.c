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
		const y_emit_state_t *state, yaml_event_t *event TSRMLS_DC);
static void y_handle_emitter_error(const y_emit_state_t *state TSRMLS_DC);
static int y_array_is_sequence(HashTable *ht TSRMLS_DC);
static void y_scan_recursion(const y_emit_state_t *state, zval *data TSRMLS_DC);
static zend_long y_search_recursive(
		const y_emit_state_t *state, const zend_ulong addr TSRMLS_DC);

static int y_write_zval(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_null(
		const y_emit_state_t *state, yaml_char_t *tag TSRMLS_DC);
static int y_write_bool(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_long(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_double(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_string(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_array(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_timestamp(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_object(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC);
static int y_write_object_callback (
		const y_emit_state_t *state, zval *callback, zval *data,
		const char *clazz_name TSRMLS_DC);
static inline unsigned int get_next_char(
		const unsigned char *str, size_t str_len, size_t *cursor, int *status);
/* }}} */


/* {{{ y_event_emit()
 * send an event to the emitter
 */
static int
y_event_emit(const y_emit_state_t *state, yaml_event_t *event TSRMLS_DC)
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
	zend_ulong kidx, idx;
	zend_string *str_key;

	idx = 0;
	ZEND_HASH_FOREACH_KEY(ht, kidx, str_key) {
		if (str_key) {
			/* non-numeric key found */
			return Y_ARRAY_MAP;
		} else if (kidx != idx) {
			/* gap in sequence found */
			return Y_ARRAY_MAP;
		}

		idx++;
	} ZEND_HASH_FOREACH_END();
	return Y_ARRAY_SEQUENCE;
}
/* }}} */


/* {{{ y_scan_recursion()
 * walk an object graph looking for recursive references
 */
static void y_scan_recursion(const y_emit_state_t *state, zval *data TSRMLS_DC)
{
	HashTable *ht;
	zval *elm;

	ZVAL_DEREF(data);

	ht = HASH_OF(data);

	if (!ht) {
		/* data isn't an array or object, so we're done */
		return;
	}

#if PHP_VERSION_ID >= 70300
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE) && GC_IS_RECURSIVE(ht)) {
#else
	if (ZEND_HASH_APPLY_PROTECTION(ht) && ht->u.v.nApplyCount > 0) {
#endif
		zval tmp;
		ZVAL_LONG(&tmp, (zend_ulong) ht);

		/* we've seen this before, so record address */
		zend_hash_next_index_insert(state->recursive, &tmp);
		return;
	}

#if PHP_VERSION_ID >= 70300
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		GC_PROTECT_RECURSION(ht);
	}
#else
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		ht->u.v.nApplyCount++;
	}
#endif

	ZEND_HASH_FOREACH_VAL(ht, elm) {
		y_scan_recursion(state, elm TSRMLS_CC);
	} ZEND_HASH_FOREACH_END();

#if PHP_VERSION_ID >= 70300
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		GC_UNPROTECT_RECURSION(ht);
	}
#else
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		ht->u.v.nApplyCount--;
	}
#endif

	return;
}
/* }}} */


/* {{{ y_search_recursive()
 * Search the recursive state hash for an address
 */
static zend_long y_search_recursive(
		const y_emit_state_t *state, const zend_ulong addr TSRMLS_DC)
{
	zval *entry;
	zend_ulong num_key;
	zend_ulong found;

	ZEND_HASH_FOREACH_NUM_KEY_VAL(state->recursive, num_key, entry) {
		found = Z_LVAL_P(entry);
		if (addr == found) {
			return num_key;
		}
	} ZEND_HASH_FOREACH_END();
	return -1;
}
/* }}} */


/* {{{ y_write_zval()
 * Write a php zval to the emitter
 */
static int y_write_zval(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	int status = FAILURE;

	switch (Z_TYPE_P(data)) {
	case IS_REFERENCE:
		status = y_write_zval(state, Z_REFVAL_P(data), tag);
		break;

	case IS_NULL:
		status = y_write_null(state, tag TSRMLS_CC);
		break;

	case IS_TRUE:
	case IS_FALSE:
		status = y_write_bool(state, data, tag TSRMLS_CC);
		break;

	case IS_LONG:
		status = y_write_long(state, data, tag TSRMLS_CC);
		break;

	case IS_DOUBLE:
		status = y_write_double(state, data, tag TSRMLS_CC);
		break;

	case IS_STRING:
		status = y_write_string(state, data, tag TSRMLS_CC);
		break;

	case IS_ARRAY:
		status = y_write_array(state, data, tag TSRMLS_CC);
		break;

	case IS_OBJECT:
		status = y_write_object(state, data, tag TSRMLS_CC);
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
static int y_write_null(const y_emit_state_t *state, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_NULL_TAG;
		omit_tag = 1;
	}

	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) "~", strlen("~"),
			omit_tag, omit_tag, YAML_PLAIN_SCALAR_STYLE);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_bool()
 */
static int y_write_bool(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	const char *res = Z_TYPE_P(data) == IS_TRUE ? "true" : "false";

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_BOOL_TAG;
		omit_tag = 1;
	}

	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) res, strlen(res),
			omit_tag, omit_tag, YAML_PLAIN_SCALAR_STYLE);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_long()
 */
static int y_write_long(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	char *res = { 0 };
	size_t res_size;

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_INT_TAG;
		omit_tag = 1;
	}

	res_size = snprintf(res, 0, ZEND_LONG_FMT, Z_LVAL_P(data));
	res = (char*) emalloc(res_size + 1);
	snprintf(res, res_size + 1, ZEND_LONG_FMT, Z_LVAL_P(data));

	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) res, strlen(res),
			omit_tag, omit_tag, YAML_PLAIN_SCALAR_STYLE);
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
static int y_write_double(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	char *res = { 0 };
	size_t res_size;

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_FLOAT_TAG;
		omit_tag = 1;
	}

	res_size = snprintf(res, 0, "%f", Z_DVAL_P(data));
	res = (char*) emalloc(res_size + 1);
	snprintf(res, res_size + 1, "%f", Z_DVAL_P(data));

	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) res, strlen(res),
			omit_tag, omit_tag, YAML_PLAIN_SCALAR_STYLE);
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
static int y_write_string(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	yaml_scalar_style_t style = YAML_PLAIN_SCALAR_STYLE;

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_STR_TAG;
		omit_tag = 1;
	}

	if (NULL != detect_scalar_type(Z_STRVAL_P(data), Z_STRLEN_P(data), NULL)) {
		/* looks like some other type to us, make sure it's quoted */
		style = YAML_DOUBLE_QUOTED_SCALAR_STYLE;

	} else {
		size_t pos = 0, us;
		int j;
		const unsigned char *s = (const unsigned char *)Z_STRVAL_P(data);
		int len = Z_STRLEN_P(data);

		for (j = 0; pos < len; j++) {
			us = get_next_char(s, len, &pos, &status);
			if (status != SUCCESS) {
				/* invalid UTF-8 character found */
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Invalid UTF-8 sequence in argument");
				return FAILURE;

			} else if ('\n' == us) {
				/* has newline(s), make sure they are preserved */
				style = YAML_LITERAL_SCALAR_STYLE;
			}
		}
	}

	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) Z_STRVAL_P(data), Z_STRLEN_P(data),
			omit_tag, omit_tag, style);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_array()
 */
static int y_write_array(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	HashTable *ht = Z_ARRVAL_P(data);
	zval *elm;
	int array_type;
	zval key_zval;
	zend_ulong kidx;
	zend_string *kstr;
	zend_long recursive_idx = -1;
	char *anchor = { 0 };
	size_t anchor_size;

	array_type = y_array_is_sequence(ht TSRMLS_CC);

	if (NULL == tag) {
		if (Y_ARRAY_SEQUENCE == array_type) {
			tag = (yaml_char_t *) YAML_SEQ_TAG;
		} else {
			tag = (yaml_char_t *) YAML_MAP_TAG;
		}
		omit_tag = 1;
	}

	/* syck does a check to see if the array is small and all scalars
	 * if it is then it outputs in inline form
	 */

	/* search state->recursive for this ht.
	 * if it exists:
	 *     anchor = "id%04d" % index
	 *   if ht->nApplyCount > 0:
	 *     emit a ref
	 */
	recursive_idx = y_search_recursive(state, (zend_ulong) ht TSRMLS_CC);
	if (-1 != recursive_idx) {
		/* create anchor to refer to this structure */
		anchor_size = snprintf(anchor, 0, "refid%ld", recursive_idx + 1);
		anchor = (char*) emalloc(anchor_size + 1);
		snprintf(anchor, anchor_size + 1, "refid%ld", recursive_idx + 1);

#if PHP_VERSION_ID >= 70300
		if (!(GC_FLAGS(ht) & GC_IMMUTABLE) && GC_IS_RECURSIVE(ht)) {
#else
		if (ZEND_HASH_APPLY_PROTECTION(ht) && ht->u.v.nApplyCount > 0) {
#endif
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
				(yaml_char_t *) anchor, tag, omit_tag,
				YAML_ANY_SEQUENCE_STYLE);
	} else {
		status = yaml_mapping_start_event_initialize(&event,
				(yaml_char_t *) anchor, tag, omit_tag,
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

#if PHP_VERSION_ID >= 70300
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		/* increment access count for hash */
		GC_PROTECT_RECURSION(ht);
	}
#else
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		/* increment access count for hash */
		ht->u.v.nApplyCount++;
	}
#endif

	/* emit array elements */
	ZEND_HASH_FOREACH_KEY_VAL(ht, kidx, kstr, elm) {
		ZVAL_DEREF(elm);

		if (Y_ARRAY_MAP == array_type) {
			/* create zval for key */
			if (kstr) {
				ZVAL_STR(&key_zval, kstr);
			} else {
				ZVAL_LONG(&key_zval, kidx);
			}

			/* emit key */
			status = y_write_zval(state, &key_zval, NULL TSRMLS_CC);
			if (SUCCESS != status) {
				return FAILURE;
			}
		}

		status = y_write_zval(state, elm, NULL TSRMLS_CC);


		if (SUCCESS != status) {
			return FAILURE;
		}
	} ZEND_HASH_FOREACH_END();

#if PHP_VERSION_ID >= 70300
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		GC_UNPROTECT_RECURSION(ht);
	}
#else
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		ht->u.v.nApplyCount--;
	}
#endif

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
static int y_write_timestamp(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int omit_tag = 0;
	int status;
	zend_class_entry *clazz = Z_OBJCE_P(data);
	zval timestamp = {{0} };
	zval dtfmt;

	if (NULL == tag) {
		tag = (yaml_char_t *) YAML_TIMESTAMP_TAG;
		omit_tag = 1;
	}

	/* iso-8601 format specifier including milliseconds */
	ZVAL_STRING(&dtfmt, "Y-m-d\\TH:i:s.uP");

	/* format date as iso-8601 string */
#if PHP_VERSION_ID >= 80000
	zend_call_method_with_1_params(Z_OBJ_P(data), clazz, NULL, "format", &timestamp, &dtfmt);
#else
	zend_call_method_with_1_params(data, clazz, NULL, "format", &timestamp, &dtfmt);
#endif

	/* emit formatted date */
	status = yaml_scalar_event_initialize(&event, NULL, tag,
			(yaml_char_t *) Z_STRVAL_P(&timestamp), Z_STRLEN_P(&timestamp),
			omit_tag, omit_tag, YAML_PLAIN_SCALAR_STYLE);
	zval_ptr_dtor(&timestamp);
	if (!status) {
		y_event_init_failed(&event);
		return FAILURE;
	}
	return y_event_emit(state, &event TSRMLS_CC);
}
/* }}} */


/* {{{ y_write_object()
 */
static int y_write_object(
		const y_emit_state_t *state, zval *data, yaml_char_t *tag TSRMLS_DC)
{
	yaml_event_t event;
	int status;
	zend_string *clazz_name;
	zval *callback;

	clazz_name = Z_OBJCE_P(data)->name;

	/* TODO check for a "yamlSerialize()" instance method */
	if (NULL != state->callbacks && (callback = zend_hash_find(
			state->callbacks, clazz_name)) != NULL) {
		/* found a registered callback for this class */
		status = y_write_object_callback(
				state, callback, data, clazz_name->val TSRMLS_CC);

	} else if (0 == strncmp(clazz_name->val, "DateTime", clazz_name->len)) {
		status = y_write_timestamp(state, data, tag TSRMLS_CC);
	} else {
		/* tag and emit serialized version of object */
		php_serialize_data_t var_hash;
		smart_str buf = { 0 };

		PHP_VAR_SERIALIZE_INIT(var_hash);
		php_var_serialize(&buf, data, &var_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(var_hash);

		status = yaml_scalar_event_initialize(&event,
				NULL, (yaml_char_t *) YAML_PHP_TAG, (yaml_char_t *) buf.s->val, buf.s->len,
				0, 0, YAML_DOUBLE_QUOTED_SCALAR_STYLE);

		if (!status) {
			y_event_init_failed(&event);
			status = FAILURE;
		} else {
			status = y_event_emit(state, &event TSRMLS_CC);
		}
		smart_str_free(&buf);
	}

	return status;
}
/* }}} */

/* {{{ y_write_object_callback()
 */
static int
y_write_object_callback (
		const y_emit_state_t *state, zval *callback, zval *data,
		const char *clazz_name TSRMLS_DC) {
	zval argv[1];
	argv[0] = *data;
	zval zret;
	zval *ztag;
	zval *zdata;
	zend_string *str_key;

	/* call the user function */
	if (FAILURE == call_user_function_ex(EG(function_table), NULL,
			callback, &zret, 1, argv, 0, NULL TSRMLS_CC) ||
			Z_TYPE_P(&zret) == IS_UNDEF) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to apply callback for class '%s'"
				" with user defined function", clazz_name);
		return FAILURE;
	}

	/* return val should be array */
	if (IS_ARRAY != Z_TYPE_P(&zret)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Expected callback for class '%s'"
				" to return an array", clazz_name);
		return FAILURE;
	}

	/* pull out the tag and surrogate object */
	str_key = zend_string_init("tag", sizeof("tag") - 1, 0);
	if ((ztag = zend_hash_find(Z_ARRVAL_P(&zret), str_key)) == NULL ||  Z_TYPE_P(ztag) != IS_STRING) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Expected callback result for class '%s'"
				" to contain a key named 'tag' with a string value",
				clazz_name);
		zend_string_release(str_key);
		return FAILURE;
	}
	zend_string_release(str_key);

	str_key = zend_string_init("data", sizeof("data") - 1, 0);
	if ((zdata = zend_hash_find(Z_ARRVAL_P(&zret), str_key)) == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Expected callback result for class '%s'"
				" to contain a key named 'data'",
				clazz_name);
		zend_string_release(str_key);
		return FAILURE;
	}
	zend_string_release(str_key);


	/* emit surrogate object and tag */
	return y_write_zval(
			state, zdata, (yaml_char_t *) Z_STRVAL_P(ztag) TSRMLS_CC);
}
/* }}} */

/* {{{ php_yaml_write_impl()
 * Common stream writing logic shared by yaml_emit and yaml_emit_file.
 */
int
php_yaml_write_impl(
		yaml_emitter_t *emitter, zval *data,
		yaml_encoding_t encoding, HashTable *callbacks TSRMLS_DC)
{
	y_emit_state_t state;
	yaml_event_t event;
	int status;

	state.emitter = emitter;
	/* scan for recursive objects */
	ALLOC_HASHTABLE(state.recursive);
	zend_hash_init(state.recursive, 8, NULL, NULL, 0);
	y_scan_recursion(&state, data TSRMLS_CC);
	state.callbacks = callbacks;


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
	if (FAILURE == y_write_zval(&state, data, NULL TSRMLS_CC)) {
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
	smart_string_appendl((smart_string *) data, (char *) buffer, size);
	return 1;
}
/* }}} */


/* {{{ get_next_char
 * Copied from ext/standard/html.c @ a37ff1fa4bb149dc81fc812d03cdf7685c499403
 * and trimmed to include only utf8 code branch. I would have liked to use
 * php_next_utf8_char() but it isn't available until php 5.4.
 *
 * Thank you cataphract@php.net!
 */
#define CHECK_LEN(pos, chars_need) ((str_len - (pos)) >= (chars_need))
#define MB_FAILURE(pos, advance) do { \
	*cursor = pos + (advance); \
	*status = FAILURE; \
	return 0; \
} while (0)
#define utf8_lead(c)  ((c) < 0x80 || ((c) >= 0xC2 && (c) <= 0xF4))
#define utf8_trail(c) ((c) >= 0x80 && (c) <= 0xBF)

static inline unsigned int get_next_char(
		const unsigned char *str,
		size_t str_len,
		size_t *cursor,
		int *status)
{
	size_t pos = *cursor;
	unsigned int this_char = 0;
	unsigned char c;

	*status = SUCCESS;
	assert(pos <= str_len);

	if (!CHECK_LEN(pos, 1))
		MB_FAILURE(pos, 1);

	/* We'll follow strategy 2. from section 3.6.1 of UTR #36:
		* "In a reported illegal byte sequence, do not include any
		*  non-initial byte that encodes a valid character or is a leading
		*  byte for a valid sequence." */
	c = str[pos];
	if (c < 0x80) {
		this_char = c;
		pos++;
	} else if (c < 0xc2) {
		MB_FAILURE(pos, 1);
	} else if (c < 0xe0) {
		if (!CHECK_LEN(pos, 2))
			MB_FAILURE(pos, 1);

		if (!utf8_trail(str[pos + 1])) {
			MB_FAILURE(pos, utf8_lead(str[pos + 1]) ? 1 : 2);
		}
		this_char = ((c & 0x1f) << 6) | (str[pos + 1] & 0x3f);
		if (this_char < 0x80) { /* non-shortest form */
			MB_FAILURE(pos, 2);
		}
		pos += 2;
	} else if (c < 0xf0) {
		size_t avail = str_len - pos;

		if (avail < 3 ||
				!utf8_trail(str[pos + 1]) || !utf8_trail(str[pos + 2])) {
			if (avail < 2 || utf8_lead(str[pos + 1]))
				MB_FAILURE(pos, 1);
			else if (avail < 3 || utf8_lead(str[pos + 2]))
				MB_FAILURE(pos, 2);
			else
				MB_FAILURE(pos, 3);
		}

		this_char = ((c & 0x0f) << 12) | ((str[pos + 1] & 0x3f) << 6) |
				(str[pos + 2] & 0x3f);
		if (this_char < 0x800) {
			/* non-shortest form */
			MB_FAILURE(pos, 3);
		} else if (this_char >= 0xd800 && this_char <= 0xdfff) {
			/* surrogate */
			MB_FAILURE(pos, 3);
		}
		pos += 3;
	} else if (c < 0xf5) {
		size_t avail = str_len - pos;

		if (avail < 4 ||
				!utf8_trail(str[pos + 1]) || !utf8_trail(str[pos + 2]) ||
				!utf8_trail(str[pos + 3])) {
			if (avail < 2 || utf8_lead(str[pos + 1]))
				MB_FAILURE(pos, 1);
			else if (avail < 3 || utf8_lead(str[pos + 2]))
				MB_FAILURE(pos, 2);
			else if (avail < 4 || utf8_lead(str[pos + 3]))
				MB_FAILURE(pos, 3);
			else
				MB_FAILURE(pos, 4);
		}

		this_char = ((c & 0x07) << 18) | ((str[pos + 1] & 0x3f) << 12) |
				((str[pos + 2] & 0x3f) << 6) | (str[pos + 3] & 0x3f);
		if (this_char < 0x10000 || this_char > 0x10FFFF) {
			/* non-shortest form or outside range */
			MB_FAILURE(pos, 4);
		}
		pos += 4;
	} else {
		MB_FAILURE(pos, 1);
	}

	*cursor = pos;
	return this_char;
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
