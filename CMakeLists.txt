cmake_minimum_required(VERSION 3.14)
project(CRUtil VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)          # gnu++17, needed for __int128 and _GNU_SOURCE

set(CRUTIL_SOURCES
    Source/condition.cpp
    Source/crexception.cpp
    Source/crstring.cpp
    Source/semaphore.cpp
)

# ---------------------------------------------------------------------------
# Release library  --  libcrutil.a lands in Build/
# ---------------------------------------------------------------------------
add_library(crutil STATIC ${CRUTIL_SOURCES})

set_target_properties(crutil PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Build"
)

target_include_directories(crutil PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(crutil PUBLIC _GNU_SOURCE)

# ---------------------------------------------------------------------------
# Debug library  --  libcrutildbg.a lands in Build/
# Compiled with -g -DDEBUG so that Semaphore layout matches debug test code.
# Test targets link against this; DEBUG=1 propagates to them automatically.
# ---------------------------------------------------------------------------
add_library(crutildbg STATIC ${CRUTIL_SOURCES})

set_target_properties(crutildbg PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Build"
)

target_include_directories(crutildbg PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Source/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(crutildbg PUBLIC _GNU_SOURCE DEBUG=1)
target_compile_options(crutildbg PRIVATE -g)

# ---------------------------------------------------------------------------
# Package  --  tar.gz with lib + headers lands in Artifacts/
# ---------------------------------------------------------------------------
set(CRUTIL_STAGE "${CMAKE_BINARY_DIR}/_stage")

add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CRUTIL_STAGE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${CRUTIL_STAGE}/lib"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${CRUTIL_STAGE}/include"
    COMMAND ${CMAKE_COMMAND} -E copy             "$<TARGET_FILE:crutil>"    "${CRUTIL_STAGE}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy             "$<TARGET_FILE:crutildbg>" "${CRUTIL_STAGE}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory   "${PROJECT_SOURCE_DIR}/Source/include" "${CRUTIL_STAGE}/include"
    COMMAND ${CMAKE_COMMAND} -E make_directory   "${PROJECT_SOURCE_DIR}/Artifacts"
    COMMAND tar -czf "${PROJECT_SOURCE_DIR}/Artifacts/CRUtil-${PROJECT_VERSION}.tar.gz"
        -C "${CRUTIL_STAGE}" lib include
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CRUTIL_STAGE}"
    DEPENDS crutil crutildbg
    COMMENT "Packaging CRUtil-${PROJECT_VERSION}.tar.gz into Artifacts/"
)

# ---------------------------------------------------------------------------
# Tests  (opt-in via cmake -DCRUTIL_BUILD_TESTS=ON ..)
# Links against crutildbg so DEBUG=1 is consistent across all TUs.
# ---------------------------------------------------------------------------
option(CRUTIL_BUILD_TESTS "Build the Catch2 test suite" OFF)

set(CRUTIL_TEST_SOURCES
    Test/test_main.cpp
    Test/test_ainteger.cpp
    Test/test_bigint.cpp
    Test/test_bigint256.cpp
    Test/test_clist.cpp
    Test/test_condition.cpp
    Test/test_crexception.cpp
    Test/test_crstring.cpp
    Test/test_crtimer.cpp
    Test/test_cuset.cpp
    Test/test_lstring.cpp
    Test/test_rqlist.cpp
    Test/test_semaphore.cpp
)

if(CRUTIL_BUILD_TESTS)
    enable_testing()
    find_package(Threads REQUIRED)

    add_executable(test_runner ${CRUTIL_TEST_SOURCES})
    target_link_libraries(test_runner PRIVATE crutildbg Threads::Threads)
    target_compile_options(test_runner PRIVATE -g)

    add_test(NAME CRUtilTests COMMAND test_runner)

    # ---------------------------------------------------------------------------
    # LFList tests  (opt-in via cmake -DCRUTIL_BUILD_TBB_TESTS=ON ..)
    # Requires oneTBB installed at .d/oneTBB/
    # ---------------------------------------------------------------------------
    option(CRUTIL_BUILD_TBB_TESTS "Build LFList tests requiring oneTBB" OFF)

    if(CRUTIL_BUILD_TBB_TESTS)
        set(ONETBB_ROOT "${PROJECT_SOURCE_DIR}/.d/oneTBB")

        add_executable(tbb_test_runner ${CRUTIL_TEST_SOURCES} Test/test_lflist.cpp)
        target_include_directories(tbb_test_runner PRIVATE "${ONETBB_ROOT}/include")
        target_link_directories(tbb_test_runner PRIVATE "${ONETBB_ROOT}/lib")
        target_link_libraries(tbb_test_runner PRIVATE crutildbg tbb Threads::Threads)
        target_compile_options(tbb_test_runner PRIVATE -g)

        add_test(NAME CRUtilTBBTests COMMAND tbb_test_runner)
    endif()
endif()
