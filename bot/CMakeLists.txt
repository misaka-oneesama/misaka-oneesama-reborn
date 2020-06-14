set(CURRENT_TARGET "bot")
set(CURRENT_TARGET_NAME "misaka-oneesama")
set(CURRENT_TARGET_INTERFACE "${CURRENT_TARGET}_interface")

message(STATUS "Configuring ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE ${CURRENT_TARGET_NAME} 20)

target_link_libraries(${CURRENT_TARGET} PRIVATE core_interface fmt)

message(STATUS "Configured ${CURRENT_TARGET}.")
