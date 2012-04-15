#!/bin/sh

## what `make distclean` does
find . -name \*.gcno -o -name \*.gcda | xargs rm -f
find . -name \*.lo -o -name \*.o | xargs rm -f
find . -name \*.la -o -name \*.a | xargs rm -f
find . -name \*.so | xargs rm -f
find . -name .libs -a -type d|xargs rm -rf
rm -f libphp.la   modules/* libs/*
rm -f Makefile config.cache config.log config.status Makefile.objects Makefile.fragments libtool main/php_config.h stamp-h php5.spec sapi/apache/libphp.module buildmk.stamp
[ -f configure ] && /bin/grep -E define'.*include/php' configure | /bin/sed 's/.*>//'|xargs rm -f

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
config.h.in \
config.log \
config.nice \
config.status \
config.sub \
configure \
configure.in \
include \
install-sh \
libtool \
ltmain.sh \
missing \
mkinstalldirs \
modules \
run-tests.php \

