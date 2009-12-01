#!/usr/bin/env bash
# Run phpt tests
#

TEST=${1:-tests/}

USE_PHP=${TEST_PHP_EXECUTABLE:-/usr/bin/php}

TEST_PHP_EXECUTABLE=${USE_PHP} \
${USE_PHP} \
  -n -d open_basedir= -d output_buffering=0 -d memory_limit=-1 \
  run-tests.php -n -d extension_dir=modules -d extension=yaml.so ${TEST}
  #-d extension_dir=modules -d extension=yaml.so \
