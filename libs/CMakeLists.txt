# fmt
set(BUNDLED_LIBFMT ON CACHE BOOL "Use the bundled libfmt.")
if (BUNDLED_LIBFMT)
    message(STATUS "Building with bundled libfmt")
    message(STATUS "Configuring bundled libfmt...")
    add_subdirectory("${PROJECT_SOURCE_DIR}/libs/fmt" "${PROJECT_BINARY_DIR}/libs/fmt" EXCLUDE_FROM_ALL)
    message(STATUS "Configured bundled libfmt.")
    set(CONFIG_STATUS_LIBFMT "6.2.1 (bundled)" CACHE INTERNAL "")
else()
    message(STATUS "Building with shared libfmt")
    find_package(fmt 6.2 REQUIRED)
    message(STATUS "Found libfmt ${fmt_VERSION}")
    set(CONFIG_STATUS_LIBFMT "${fmt_VERSION} (system)" CACHE INTERNAL "")
endif()

# magic_enum
add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE "${PROJECT_SOURCE_DIR}/libs/magic_enum/include")

# nlohmann json
add_library(json INTERFACE)
target_include_directories(json INTERFACE "${PROJECT_SOURCE_DIR}/libs/json/include")
