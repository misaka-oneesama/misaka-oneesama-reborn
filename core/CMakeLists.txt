set(CURRENT_TARGET "core")
set(CURRENT_TARGET_NAME "core")
set(CURRENT_TARGET_INTERFACE "${CURRENT_TARGET}_interface")

message(STATUS "Configuring ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC ${CURRENT_TARGET_NAME} 20)

# IXWebSocket
set(USE_TLS TRUE CACHE INTERNAL "")
add_subdirectory("${PROJECT_SOURCE_DIR}/libs/IXWebSocket" "${CMAKE_CURRENT_BINARY_DIR}/IXWebSocket" EXCLUDE_FROM_ALL)

target_link_libraries(${CURRENT_TARGET}
    PRIVATE
        ixwebsocket
        magic_enum
        fmt
        json
)

add_library(${CURRENT_TARGET_INTERFACE} INTERFACE)
target_include_directories(${CURRENT_TARGET_INTERFACE} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${CURRENT_TARGET_INTERFACE} INTERFACE ${CURRENT_TARGET})

message(STATUS "Configured ${CURRENT_TARGET}.")
