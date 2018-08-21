find_path(PORTAUDIO_INCLUDE_DIR portaudio.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include;)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
find_library(PORTAUDIO_LIBRARY NAMES portaudio${ARCH_TYPE} PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

if (MINGW)
  set(PORTAUDIO_INCLUDE_DIR "")
  set(PORTAUDIO_LIBRARY "")
endif(MINGW)

message(STATUS ${PORTAUDIO_LIBRARY})

if (PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)
      set(PORTAUDIO_FOUND TRUE)
endif (PORTAUDIO_INCLUDE_DIR AND PORTAUDIO_LIBRARY)

if (PORTAUDIO_FOUND)
      message (STATUS "Found PortAudio: ${PORTAUDIO_LIBRARY}")
else (PORTAUDIO_FOUND)
      message (FATAL_ERROR "Could not find: PortAudio")
endif (PORTAUDIO_FOUND)
