cmake_minimum_required(VERSION 3.14)
project(CRUtil VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)          # gnu++17, needed for __int128 and _GNU_SOURCE

# ---------------------------------------------------------------------------
# Library  --  static archive lands in Build/
# ---------------------------------------------------------------------------
add_library(crutil STATIC
    Source/crstring.cpp
)

set_target_properties(crutil PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Build"
)

target_include_directories(crutil PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(crutil PUBLIC _GNU_SOURCE)

# ---------------------------------------------------------------------------
# Package  --  tar.gz with lib + headers lands in Artifacts/
# ---------------------------------------------------------------------------
set(CRUTIL_STAGE "${CMAKE_BINARY_DIR}/_stage")

add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CRUTIL_STAGE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${CRUTIL_STAGE}/lib"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${CRUTIL_STAGE}/include"
    COMMAND ${CMAKE_COMMAND} -E copy             "$<TARGET_FILE:crutil>" "${CRUTIL_STAGE}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory   "${PROJECT_SOURCE_DIR}/Source/include" "${CRUTIL_STAGE}/include"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${PROJECT_SOURCE_DIR}/Artifacts"
    COMMAND tar -czf "${PROJECT_SOURCE_DIR}/Artifacts/CRUtil-${PROJECT_VERSION}.tar.gz"
        -C "${CRUTIL_STAGE}" lib include
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CRUTIL_STAGE}"
    DEPENDS crutil
    COMMENT "Packaging CRUtil-${PROJECT_VERSION}.tar.gz into Artifacts/"
)

# ---------------------------------------------------------------------------
# Tests  (opt-in via cmake -DCRUTIL_BUILD_TESTS=ON ..)
# ---------------------------------------------------------------------------
option(CRUTIL_BUILD_TESTS "Build the Catch2 test suite" OFF)

if(CRUTIL_BUILD_TESTS)
    enable_testing()
    find_package(Threads REQUIRED)

    add_executable(test_runner
        Test/test_main.cpp
        Test/test_ainteger.cpp
        Test/test_bigint.cpp
        Test/test_bigint256.cpp
        Test/test_condition.cpp
        Test/test_crexception.cpp
        Test/test_crstring.cpp
        Test/test_crtimer.cpp
        Test/test_lstring.cpp
    )

    target_link_libraries(test_runner PRIVATE crutil Threads::Threads)
    target_compile_definitions(test_runner PRIVATE DEBUG=1)

    add_test(NAME CRUtilTests COMMAND test_runner)
endif()
