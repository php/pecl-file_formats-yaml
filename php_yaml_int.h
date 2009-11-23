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


#ifndef PHP_YAML_INT_H
#define PHP_YAML_INT_H

#ifdef __cplusplus
extern "C" {
#endif

/* {{{ ext/yaml macros
 */
#define SCALAR_TAG_IS(event, name) \
  !strcmp((const char *)event.data.scalar.tag, "tag:yaml.org,2002:" name)

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

#define php_yaml_read_all(parser, ndocs, eval_func, callbacks) \
  php_yaml_read_impl((parser), NULL, NULL, NULL, (ndocs), (eval_func), (callbacks) TSRMLS_CC)

#define event_emit(e) \
  if (!yaml_emitter_emit(emitter, e)) { \
    goto emitter_error; \
  }

#define YAML_PHP_TAG        "!php/object"
/* }}} */

/* {{{ ext/yaml prototypes
 */
zval * php_yaml_read_impl (
    yaml_parser_t *parser, yaml_event_t *parent,
    zval *aliases, zval *zv, long *ndocs,
    eval_scalar_func_t eval_func, HashTable *callbacks TSRMLS_DC);

zval * php_yaml_read_partial (
    yaml_parser_t *parser, long pos, long *ndocs,
    eval_scalar_func_t eval_func, HashTable *callbacks TSRMLS_DC);

zval * php_yaml_eval_scalar (
    yaml_event_t event, HashTable *callbacks TSRMLS_DC);

zval * php_yaml_eval_scalar_with_callbacks (
    yaml_event_t event, HashTable *callbacks TSRMLS_DC);

char * php_yaml_detect_scalar_type (
    const char *value, size_t length, const yaml_event_t *event);

int php_yaml_write_impl (
    yaml_emitter_t *emitter, zval *data, yaml_encoding_t encoding TSRMLS_DC);

int php_yaml_write_to_buffer (
    void *data, unsigned char *buffer, size_t size);

/* }}} */

/* {{{ globals */

ZEND_DECLARE_MODULE_GLOBALS(yaml)

/* }}} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PHP_YAML_INT_H */

/*
 * Local variables:
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 * vim600: et sw=2 ts=2 fdm=marker
 * vim<600: et sw=2 ts=2
 */
