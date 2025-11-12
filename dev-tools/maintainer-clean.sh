#!/usr/bin/env bash

BASENAME=${0}
TOOLS_DIR=$(dirname ${BASENAME})
WORK_DIR="${TOOLS_DIR}/.."

cd "${WORK_DIR}"

## what `make distclean` does
find . -name \*.gcno -o -name \*.gcda | xargs rm -f
find . -name \*.lo -o -name \*.o -o -name \*.dep | xargs rm -f
find . -name \*.la -o -name \*.a | xargs rm -f
find . -name \*.so | xargs rm -f
find . -name .libs -a -type d|xargs rm -rf
rm -f libphp.la        modules/* libs/*
rm -f ext/opcache/jit/ir/gen_ir_fold_hash
rm -f ext/opcache/jit/ir/minilua
rm -f ext/opcache/jit/ir/ir_fold_hash.h
rm -f ext/opcache/jit/ir/ir_emit_x86.h
rm -f ext/opcache/jit/ir/ir_emit_aarch64.h
rm -f Makefile config.cache config.log config.status Makefile.objects Makefile.fragments libtool main/php_config.h main/internal_functions_cli.c main/internal_functions.c Zend/zend_dtrace_gen.h Zend/zend_dtrace_gen.h.bak Zend/zend_config.h
rm -f main/build-defs.h scripts/phpize
rm -f ext/date/lib/timelib_config.h ext/mbstring/libmbfl/config.h
rm -f scripts/man1/phpize.1 scripts/php-config scripts/man1/php-config.1 sapi/cli/php.1 sapi/cgi/php-cgi.1 sapi/phpdbg/phpdbg.1 ext/phar/phar.1 ext/phar/phar.phar.1
rm -f sapi/fpm/php-fpm.conf sapi/fpm/init.d.php-fpm sapi/fpm/php-fpm.service sapi/fpm/php-fpm.8 sapi/fpm/status.html
rm -f ext/phar/phar.phar ext/phar/phar.php

## get rid of anything that phpize and configure made as well
rm -rf .deps \
.libs \
Makefile \
Makefile.fragments \
Makefile.global \
Makefile.objects \
acinclude.m4 \
aclocal.m4 \
autom4te.cache \
build \
config.guess \
config.h \
config.h.in* \
config.log \
config.nice \
config.status \
config.sub \
configure \
configure.ac \
configure.in \
configure~ \
include \
install-sh \
libtool \
ltmain.sh \
missing \
mkinstalldirs \
modules \
run-tests.php \

## coverage stuff
rm -rf coverage.info reports

## failed tests
rm -f tests/*.{diff,exp,log,out,php,sh}
