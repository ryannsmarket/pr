#!/usr/bin/env bash

echo "Build MuseScore"
#set -x
trap 'echo Build failed; exit 1' ERR
SKIP_ERR=true

ARTIFACTS_DIR=build.artifacts
TELEMETRY_TRACK_ID=""
CRASH_REPORT_URL=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -n|--number) BUILD_NUMBER="$2"; shift ;;
        --telemetry) TELEMETRY_TRACK_ID="$2"; shift ;;
        --crash_log_url) CRASH_REPORT_URL="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$BUILD_NUMBER" ]; then echo "error: not set BUILD_NUMBER"; exit 1; fi
if [ -z "$TELEMETRY_TRACK_ID" ]; then TELEMETRY_TRACK_ID=""; fi

BUILD_MODE=$(cat $ARTIFACTS_DIR/env/build_mode.env)
MUSESCORE_BUILD_CONFIG=dev
if [ "$BUILD_MODE" == "devel_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "nightly_build" ]; then MUSESCORE_BUILD_CONFIG=dev; fi
if [ "$BUILD_MODE" == "testing_build" ]; then MUSESCORE_BUILD_CONFIG=testing; fi
if [ "$BUILD_MODE" == "stable_build" ]; then MUSESCORE_BUILD_CONFIG=release; fi

if [ -z "$VST3_SDK_PATH" ]; then 
echo "warning: not set VST3_SDK_PATH, build VST module disabled"
BUILD_VST=OFF
else
BUILD_VST=ON
fi

echo "MUSESCORE_BUILD_CONFIG: $MUSESCORE_BUILD_CONFIG"
echo "BUILD_NUMBER: $BUILD_NUMBER"
echo "TELEMETRY_TRACK_ID: $TELEMETRY_TRACK_ID"
echo "CRASH_REPORT_URL: $CRASH_REPORT_URL"
echo "VST3_SDK_PATH: $VST3_SDK_PATH"

MUSESCORE_REVISION=$(git rev-parse --short=7 HEAD)

# make -f Makefile.osx \
#     MUSESCORE_BUILD_CONFIG=$MUSESCORE_BUILD_CONFIG \
#     MUSESCORE_REVISION=$MUSESCORE_REVISION \
#     BUILD_NUMBER=$BUILD_NUMBER \
#     TELEMETRY_TRACK_ID=$TELEMETRY_TRACK_ID \
#     CRASH_REPORT_URL=$CRASH_REPORT_URL \
#     BUILD_VST=$BUILD_VST \
#     VST3_SDK_PATH=$VST3_SDK_PATH \
#     ci

PREFIX=../applebuild

mkdir build.release
cd build.release
cmake -GNinja \
        -DCMAKE_INSTALL_PREFIX="${PREFIX}" \
        -DCMAKE_BUILD_TYPE=RELEASE \
        -DCMAKE_BUILD_NUMBER="${BUILD_NUMBER}" \
        -DMUSESCORE_BUILD_CONFIG="${MUSESCORE_BUILD_CONFIG}" \
        -DMUSESCORE_REVISION="${MUSESCORE_REVISION}" \
        -DTELEMETRY_TRACK_ID="${TELEMETRY_TRACK_ID}" \
        -DCRASH_REPORT_URL="${CRASH_REPORT_URL}" \
        ..\

ninja -j 4 install  

cd ..

bash ./build/ci/tools/make_release_channel_env.sh -c $MUSESCORE_BUILD_CONFIG
bash ./build/ci/tools/make_version_env.sh $BUILD_NUMBER
bash ./build/ci/tools/make_revision_env.sh $MUSESCORE_REVISION
bash ./build/ci/tools/make_branch_env.sh
bash ./build/ci/tools/make_datetime_env.sh
