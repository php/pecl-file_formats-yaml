/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any personobtaining a
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

#ifndef PHP_YAML_H
#define PHP_YAML_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <SAPI.h>
#ifdef ZTS
#include "TSRM.h"
#endif
#include <ext/standard/info.h>
#include <ext/standard/base64.h>
#include <ext/standard/basic_functions.h>
#include <ext/standard/php_var.h>
#include <ext/standard/php_smart_str.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#ifdef __cplusplus
} // extern "C"
#endif

#include <yaml.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHP_YAML_MODULE_VERSION "0.6.0-dev"

#define Y_PARSER_CONTINUE 0
#define Y_PARSER_SUCCESS  1
#define Y_PARSER_FAILURE -1

#define Y_FILTER_NONE     0
#define Y_FILTER_SUCCESS  1
#define Y_FILTER_FAILURE -1

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

/* {{{ module globals */

ZEND_BEGIN_MODULE_GLOBALS(yaml)
	zend_bool decode_binary;
	long decode_timestamp;
	zval *timestamp_decoder;
#ifdef IS_UNICODE
	UConverter *orig_runtime_encoding_conv;
#endif
ZEND_END_MODULE_GLOBALS(yaml)

#ifdef ZTS
#define YAML_G(v) TSRMG(yaml_globals_id, zend_yaml_globals *, v)
#else
#define YAML_G(v) (yaml_globals.v)
#endif

/* }}} */

typedef zval* (*eval_scalar_func_t)(yaml_event_t event, HashTable *callbacks TSRMLS_DC);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* PHP_YAML_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
