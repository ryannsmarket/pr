REM @echo off
ECHO "MuseScore build"

SET ARTIFACTS_DIR=build.artifacts
SET INSTALL_DIR=../build.install
SET BUILD_NUMBER=""
SET CRASH_LOG_SERVER_URL=""
SET TARGET_PROCESSOR_BITS=64
SET BUILD_WIN_PORTABLE=OFF
SET YOUTUBE_API_KEY=""
SET QT5_COMPAT=OFF

:GETOPTS
IF /I "%1" == "-n" SET BUILD_NUMBER=%2& SHIFT
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2& SHIFT
IF /I "%1" == "--crash_log_url" SET CRASH_LOG_SERVER_URL=%2& SHIFT & SHIFT
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2& SHIFT
IF /I "%1" == "--youtube_api_key" SET YOUTUBE_API_KEY=%2& SHIFT
IF /I "%1" == "--qt5_compat" SET QT5_COMPAT=%2 & SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF %BUILD_NUMBER% == "" ( ECHO "error: not set BUILD_NUMBER" & EXIT /b 1)
IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "error: not set TARGET_PROCESSOR_BITS, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        EXIT /b 1
    )
)

SET /p BUILD_MODE=<%ARTIFACTS_DIR%\env\build_mode.env
SET "MUSESCORE_BUILD_MODE=dev"
IF %BUILD_MODE% == devel_build   ( SET "MUSESCORE_BUILD_MODE=dev" ) ELSE (
IF %BUILD_MODE% == nightly_build ( SET "MUSESCORE_BUILD_MODE=dev" ) ELSE (
IF %BUILD_MODE% == testing_build ( SET "MUSESCORE_BUILD_MODE=testing" ) ELSE (
IF %BUILD_MODE% == stable_build  ( SET "MUSESCORE_BUILD_MODE=release" ) ELSE (
    ECHO "error: unknown BUILD_MODE: %BUILD_MODE%"
    EXIT /b 1
))))

ECHO "MUSESCORE_BUILD_MODE: %MUSESCORE_BUILD_MODE%"
ECHO "BUILD_NUMBER: %BUILD_NUMBER%"
ECHO "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
ECHO "CRASH_LOG_SERVER_URL: %CRASH_LOG_SERVER_URL%"
ECHO "BUILD_WIN_PORTABLE: %BUILD_WIN_PORTABLE%"
ECHO "YOUTUBE_API_KEY: %YOUTUBE_API_KEY%"
ECHO "QT5_COMPAT: %QT5_COMPAT%"

XCOPY "C:\musescore_dependencies" %CD% /E /I /Y
ECHO "Finished copy dependencies"


SET "QT_DIR=C:\Qt\6.2.4"
IF %QT5_COMPAT% == ON ( 
    SET "QT_DIR=C:\Qt\5.15.2"
)
SET "PATH=%QT_DIR%\msvc2019_64\bin;%JACK_DIR%;%PATH%"
SET "JACK_DIR=C:\Program Files (x86)\Jack"

:: At the moment not compiling yet.
SET BUILD_VST=ON 
SET VST3_SDK_PATH=C:\vst\VST3_SDK

SET MUSESCORE_BUILD_CONFIGURATION="app"
IF %BUILD_WIN_PORTABLE% == ON ( 
    SET INSTALL_DIR=../build.install/App/MuseScore
    SET MUSESCORE_BUILD_CONFIGURATION="app-portable"
)

bash ./build/ci/tools/make_revision_env.sh 
SET /p MUSESCORE_REVISION=<%ARTIFACTS_DIR%\env\build_revision.env

SET MUSESCORE_BUILD_CONFIGURATION=%MUSESCORE_BUILD_CONFIGURATION%
SET MUSESCORE_BUILD_MODE=%MUSESCORE_BUILD_MODE%
SET MUSESCORE_BUILD_NUMBER=%BUILD_NUMBER%
SET MUSESCORE_REVISION=%MUSESCORE_REVISION%
SET MUSESCORE_INSTALL_DIR=%INSTALL_DIR%
SET MUSESCORE_CRASHREPORT_URL="%CRASH_LOG_SERVER_URL%"
SET MUSESCORE_BUILD_VST_MODULE=%BUILD_VST%
SET MUSESCORE_VST3_SDK_PATH=%VST3_SDK_PATH%
SET MUSESCORE_YOUTUBE_API_KEY=%YOUTUBE_API_KEY%
SET MUSESCORE_QT5_COMPAT=%QT5_COMPAT%

CALL ninja_build.bat -t installrelwithdebinfo || exit \b 1

bash ./build/ci/tools/make_release_channel_env.sh -c %MUSESCORE_BUILD_MODE%
bash ./build/ci/tools/make_version_env.sh %BUILD_NUMBER%
bash ./build/ci/tools/make_branch_env.sh
