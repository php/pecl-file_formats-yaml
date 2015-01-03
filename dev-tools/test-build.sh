#!/usr/bin/env bash
# build and test against a phpenv managed php version

set -e

SCRIPT_DIR=$(dirname $0)

hash phpenv &>/dev/null || {
  echo >&2 "phpenv not found in path"
  echo >&2 "Install from https://github.com/CHH/phpenv"
  exit 1
}

[[ -n $1 ]] || {
  echo >&2 "usage: $0 <phpenv version>"
  echo >&2 "versions: "
  phpenv versions >&2
  exit 2
}

make distclean &>/dev/null || true
phpenv local $1 &&
phpize &&
CC=colorgcc CFLAGS="-Wall -fno-strict-aliasing" ./configure --with-yaml &&
make clean all &&
TEST_PHP_EXECUTABLE=$(command -v php) ${SCRIPT_DIR}/test.sh || exit $?
