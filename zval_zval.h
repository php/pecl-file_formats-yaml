/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2007 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Andrei Zmievski <andrei@php.net>                            |
   +----------------------------------------------------------------------+
*/

#ifndef ZVAL_ZVAL

#define ZVAL_ZVAL(z, zv, copy, dtor) {  \
		int is_ref, refcount;           \
		is_ref = (z)->is_ref;           \
		refcount = (z)->refcount;       \
		*(z) = *(zv);                   \
		if (copy) {                     \
			zval_copy_ctor(z);          \
		}                               \
		if (dtor) {                     \
			if (!copy) {                \
				ZVAL_NULL(zv);          \
			}                           \
			zval_ptr_dtor(&zv);         \
		}                               \
		(z)->is_ref = is_ref;           \
		(z)->refcount = refcount;       \
	}

#define RETVAL_ZVAL(zv, copy, dtor) ZVAL_ZVAL(return_value, zv, copy, dtor)
#define RETURN_ZVAL(zv, copy, dtor) { RETVAL_ZVAL(zv, copy, dtor); return; }

#endif /* ZVAL_ZVAL */
