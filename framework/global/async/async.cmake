

include(${PROJECT_SOURCE_DIR}/thirdparty/deto_async/async/async.cmake)

set(ASYNC_SRC
    ${ASYNC_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/asyncable.h
    ${CMAKE_CURRENT_LIST_DIR}/notify.h
    )
