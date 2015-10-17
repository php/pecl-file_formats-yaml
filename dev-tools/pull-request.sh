#!/usr/bin/env bash
# Apply a github pull request.
#
#/ usage: pull-request.sh PULL-REQUEST-ID [BRANCH]
#/   - PULL-REQUEST-ID : Pull request number
#/   - BRANCH          : Branch to land to (default: "master")
#

set -e
set -u

REQ=${1:?PULL-REQUEST-ID required}
GITHUB_REPO="php/pecl-file_formats-yaml"
BRANCH=${2:-master}

# switch to main branch
git checkout ${BRANCH}

# fetch the pull request into a local branch
git fetch git://github.com/${GITHUB_REPO}.git \
  pull/${REQ}/head:pull-request/${REQ}

# review changes
git log --patch-with-stat ${BRANCH}..pull-request/${REQ}

printf "Accept changes? (y/N) "
read -e APPLY
if [[ "y" = $APPLY || "Y" = $APPLY ]]; then
  # merge into working copy
  git merge --no-ff --no-commit pull-request/${REQ}

  echo "Changes staged for commit."
  echo " - Don't forget to make a good commit message!"
  echo " - This might be a good time to update package.xml too."
fi
