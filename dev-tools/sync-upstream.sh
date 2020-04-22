#!/usr/bin/env bash
# Sync master of github clone with canonical upstream.
#

set -e
set -u

UPSTREAM_URL="https://github.com/php/pecl-file_formats-yaml.git"
UPSTREAM=upstream
MASTER=php7

MY_UPSTREAM=$(git remote show ${UPSTREAM} 2>/dev/null |
    grep "Fetch URL"|awk '{print $3}')

if [[ -z ${MY_UPSTREAM} ]]; then
  echo "Adding remote ${UPSTREAM} ${UPSTREAM_URL}"
  git remote add ${UPSTREAM} ${UPSTREAM_URL}
  MY_UPSTREAM=${UPSTREAM_URL}
fi

if [[ ${MY_UPSTREAM} != ${UPSTREAM_URL} ]]; then
  echo "Remote ${UPSTREAM} not tracking expected ${UPSTREAM_URL}"
  echo "Cowardly refusing to continue"
  exit 1
fi

set -x

git fetch ${UPSTREAM} -pv
git checkout ${MASTER}
git merge --ff-only ${UPSTREAM}/${MASTER}
