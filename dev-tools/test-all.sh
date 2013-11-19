#!/usr/bin/env bash

rm .rbenv-version 2>/dev/null

VERSIONS=$(phpenv versions --bare)
SCRIPT_DIR=$(dirname $0)

for v in $VERSIONS system; do
  echo -n "$v: "
  if ${SCRIPT_DIR}/test-build.sh $v >$v.log 2>&1; then
    echo -e "\033[1;32mPASS\033[0m"
  else
    echo -e "\033[1;31mFAIL\033[0m"
  fi
done
