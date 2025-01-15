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

CLANG_TIDY=$(which clang-tidy)
if [ ! -e "$CLANG_TIDY" ]; then
  echo "Failed to find clang-tidy ($CLANG_TIDY)"
  exit 1
fi

# This needs for FreeBSD and Darwin which doesn't support readlink -f command
function realpath() {
  python -c "import os,sys; print(os.path.realpath(sys.argv[1]))" $1;
}

CLANG_DIR=$(dirname $(dirname $(realpath $CLANG_TIDY)))
CLANG_TIDY_DIFF=$CLANG_DIR/share/clang/clang-tidy-diff.py
ARG="-quiet -p1 -iregex=src/.*\.(cpp|cc|c\+\+|cxx|c|cl|h|hpp|m|mm)"
if [ ! -e "$CLANG_TIDY_DIFF" ]; then
  echo "Failed to find clang-tidy-diff.py ($CLANG_TIDY_DIFF)"
  exit 1
fi
TIDY_MSG=$(git diff -U0 $BRANCH... | $CLANG_TIDY_DIFF $ARG 2> /dev/null)
if [ -n "$TIDY_MSG" -a "$TIDY_MSG" != "No relevant changes found." ]; then
  echo "Please fix clang-tidy errors before committing"
  echo
  # Run clang-tidy once again to show the error
  git diff -U0 $BRANCH... | $CLANG_TIDY_DIFF $ARG 2> /dev/null
  exit 1
fi
