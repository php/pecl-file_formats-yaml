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
CC=colorgcc \
CFLAGS="-Wall -fno-strict-aliasing -g -O0 --coverage -fprofile-arcs -ftest-coverage" \
LDFLAGS="--coverage" \
./configure --with-yaml &&
make clean all &&
lcov --directory . --zerocounters &&
lcov --directory . --capture --initial --output-file coverage.info
TEST_PHP_EXECUTABLE=$(command -v php) ${SCRIPT_DIR}/test.sh || exit $?
[[ -f .libs/yaml.gcno ]] || touch .libs/yaml.gcno # HACK
lcov --no-checksum --directory . --capture --output-file coverage.info
genhtml --highlight --legend --output-directory reports coverage.info
