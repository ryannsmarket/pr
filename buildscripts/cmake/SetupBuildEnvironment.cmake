
include(GetCompilerInfo)
include(GetBuildType)

set(BUILD_SHARED_LIBS OFF)
set(SHARED_LIBS_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(CMAKE_UNITY_BUILD_BATCH_SIZE 12)

if (CC_IS_GCC)
    message(STATUS "Using Compiler GCC ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    if (MUE_COMPILE_USE_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
        set(BUILD_SHARED_LIBS ON)
    endif()

    if (MUSE_COMPILE_ASAN)
        add_compile_options("-fsanitize=address")
        add_compile_options("-fno-omit-frame-pointer")
        link_libraries("-fsanitize=address")
    endif()

elseif(CC_IS_MSVC)
    message(STATUS "Using Compiler MSVC ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS                 "/MP /EHsc /utf-8")
    set(CMAKE_C_FLAGS                   "/MP /utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG           "/MT /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE         "/MT /O2 /Ob2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "/MT /Zi /O2 /Ob1")
    set(CMAKE_C_FLAGS_DEBUG             "/MT /Zi /Ob0 /Od /RTC1")
    set(CMAKE_C_FLAGS_RELEASE           "/MT /O2 /Ob2")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "/MT /Zi /O2 /Ob1")

    add_compile_definitions(WIN32)
    add_compile_definitions(_WINDOWS)
    add_compile_definitions(_UNICODE)
    add_compile_definitions(UNICODE)
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(NOMINMAX)

elseif(CC_IS_MINGW)
    message(STATUS "Using Compiler MINGW ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    if (MUE_COMPILE_USE_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
        set(BUILD_SHARED_LIBS ON)
    endif()

    # -mno-ms-bitfields see #22048
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
    if (NOT MUSE_COMPILE_BUILD_64)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware")
    endif()

    add_compile_definitions(_UNICODE)
    add_compile_definitions(UNICODE)

elseif(CC_IS_CLANG)
    message(STATUS "Using Compiler CLANG ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    if (MUSE_COMPILE_ASAN)
        add_compile_options("-fsanitize=address")
        add_compile_options("-fno-omit-frame-pointer")
        link_libraries("-fsanitize=address")
    endif()

    # On macOS with clang there are problems with debugging
    # - the value of the std::u16string is not visible.
    if (BUILD_IS_DEBUG AND MUSE_COMPILE_STRING_DEBUG_HACK)
        add_compile_definitions(MUSE_STRING_DEBUG_HACK)
    endif()

elseif(CC_IS_EMSCRIPTEN)
    message(STATUS "Using Compiler Emscripten ${CMAKE_CXX_COMPILER_VERSION}")

    set(EMCC_CMAKE_TOOLCHAIN "" CACHE FILEPATH "Path to EMCC CMake Emscripten.cmake")
    set(EMCC_INCLUDE_PATH "." CACHE PATH "Path to EMCC include dir")
    set(EMCC_COMPILE_FLAGS "--bind -o .html --preload-file ../../files")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public_html)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")
    set(CMAKE_TOOLCHAIN_FILE ${EMCC_CMAKE_TOOLCHAIN})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    #for QtCreator
    include_directories(AFTER
        ${EMCC_INCLUDE_PATH}
        ${EMCC_INCLUDE_PATH}/libcxx
        ${EMCC_INCLUDE_PATH}/libc
    )

else()
    message(FATAL_ERROR "Unsupported Compiler CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
endif()

##
# Setup compile warnings
##
include(SetupCompileWarnings)

# Common
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE MATCHES "DEBUG") #Debug

    add_compile_definitions(QT_QML_DEBUG)

else() #Release

    add_compile_definitions(NDEBUG)
    add_compile_definitions(QT_NO_DEBUG)

endif()


# APPLE specific
if (OS_IS_MAC)
    set(MACOSX_DEPLOYMENT_TARGET 10.14)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif(OS_IS_MAC)
