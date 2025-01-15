#!/bin/bash

set -o errexit
set -o pipefail

if [ -n "$1" ]; then
  BRANCH="$1"
elif [ -n "$GITHUB_BASE_REF" ]; then
  BRANCH="origin/$GITHUB_BASE_REF"
else
  BRANCH="$(git symbolic-ref --short -q HEAD)"
fi

LLVM_VERSION=${LLVM_VERSION:=17}

MERGE_BASE=$(git merge-base $BRANCH HEAD)
FORMAT_ARGS="--binary=clang-format-${LLVM_VERSION} ${MERGE_BASE}"
FORMAT_MSG=$(git clang-format ${FORMAT_ARGS} -q --diff)
if [ -n "$FORMAT_MSG" -a "$FORMAT_MSG" != "no modified files to format" ]
then
  echo "Please run git clang-format with clang-format-${LLVM_VERSION} before committing, or apply this diff:"
  echo
  # Run git clang-format again, this time without capruting stdout.  This way
  # clang-format format the message nicely and add color.
  git clang-format ${FORMAT_ARGS} -q --diff
  exit 1
fi
