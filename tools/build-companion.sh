#!/bin/bash

# Stops on first error, echo on
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
. "$SCRIPT_DIR/build-common.sh" 

if [ "$(uname)" = "Darwin" ]; then
  num_cpus=$(sysctl -n hw.ncpu)
  : "${JOBS:=$num_cpus}"
else
  JOBS=3
fi

while [ $# -gt 0 ]
do
  case "$1" in
    --jobs=*)
      JOBS="${1#*=}";;
    -j*)
      JOBS="${1#*j}";;
    -*)
      echo >&2 "usage: $0 [-j<jobs>|--jobs=<jobs>] SRCDIR OUTDIR"
      exit 1;;
    *)
      break;;   # terminate while loop
  esac
  shift
done

SRCDIR=$1
OUTDIR=$2

# Generate EDGETX_VERSION_SUFFIX if not already set
if [[ -z ${EDGETX_VERSION_SUFFIX} ]]; then
  gh_type=$(echo "$GITHUB_REF" | awk -F / '{print $2}') #heads|tags|pull
  if [[ $gh_type = "tags" ]]; then
    # tags: refs/tags/<tag_name>
    gh_tag=${GITHUB_REF##*/}
    export EDGETX_VERSION_TAG=$gh_tag
  elif [[ $gh_type = "pull" ]]; then
    # pull: refs/pull/<pr_number>/merge
    gh_pull_number=PR$(echo "$GITHUB_REF" | awk -F / '{print $3}')
    export EDGETX_VERSION_SUFFIX=$gh_pull_number
  elif [[ $gh_type = "heads" ]]; then
    # heads: refs/heads/<branch_name>
    gh_branch=${GITHUB_REF##*/}
    export EDGETX_VERSION_SUFFIX=$gh_branch
  fi
fi

rm -rf build
mkdir build
cd build


if [ "$(uname)" = "Darwin" ]; then
    cmake -DDISABLE_SIMULATOR=y -DDISABLE_RADIO=y "${SRCDIR}"
    cmake --build . --target native-configure
    cmake --build native -j"${JOBS}" --target package
    cp native/*.dmg "${OUTDIR}"
elif [ "$(uname)" = "Linux" ]; then
    cmake -DDISABLE_SIMULATOR=y -DDISABLE_RADIO=y "${SRCDIR}"
    cmake --build . --target native-configure
    cmake --build native -j"${JOBS}" --target package
    cp native/*.AppImage "${OUTDIR}"
else
    cmake -G Ninja -DDISABLE_SIMULATOR=y -DDISABLE_RADIO=y "${SRCDIR}"
    cmake --build . --target native-configure
    cmake --build native --target installer
    cp native/companion/*.exe "${OUTDIR}"
fi
