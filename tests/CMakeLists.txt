set(CURRENT_TARGET "tests")
set(CURRENT_TARGET_NAME "tests")
set(CURRENT_TARGET_INTERFACE "${CURRENT_TARGET}_interface")

message(STATUS "Configuring ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE ${CURRENT_TARGET_NAME} 20)

target_link_libraries(${CURRENT_TARGET} PRIVATE core_interface fmt)

target_include_directories(${CURRENT_TARGET} SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/libs/bandit")

message(STATUS "Configured ${CURRENT_TARGET}.")
