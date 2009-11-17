#!/bin/sh
# Run phpt tests
#
TEST_PHP_EXECUTABLE=/usr/bin/php \
/usr/bin/php \
  -n -d open_basedir= -d output_buffering=0 -d memory_limit=-1 \
  -d extension_dir=modules/ -d extension=yaml.so \
  run-tests.php tests/
