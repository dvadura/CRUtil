# CRUtil Project Notes

## Running Tests

`bif test` does not currently work (no test targets are configured in the inference map).

### CMake (preferred)

Configure and build from the project root:

```bash
cmake -S . -B Build/cmake -DCRUTIL_BUILD_TESTS=ON
cmake --build Build/cmake
```

Run the tests:

```bash
./Build/cmake/test_runner
```

The test runner links against `libcrutildbg.a` (compiled with `-g -DDEBUG=1`) to ensure
a consistent `Semaphore` layout across all translation units.

### LFList Tests (Lock-Free List) via CMake

LFList tests require oneTBB, installed locally at `.d/oneTBB/`.

```bash
cmake -S . -B Build/cmake -DCRUTIL_BUILD_TESTS=ON -DCRUTIL_BUILD_TBB_TESTS=ON
cmake --build Build/cmake

# Run (set DYLD_LIBRARY_PATH for oneTBB)
export DYLD_LIBRARY_PATH=/Volumes/Development/DV/Live/CRUtil/.d/oneTBB/lib:$DYLD_LIBRARY_PATH
./Build/cmake/tbb_test_runner

# Or run only LFList tests
./Build/cmake/tbb_test_runner "[lflist]"
```

### Manual Build (fallback)

```bash
cd Test && g++ -std=gnu++17 -D_GNU_SOURCE -DDEBUG=1 -g -I ../Source/include -o test_runner \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_clist.cpp test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_cuset.cpp test_lstring.cpp test_rqlist.cpp test_semaphore.cpp \
  ../Source/condition.cpp ../Source/crexception.cpp ../Source/crstring.cpp ../Source/semaphore.cpp \
  -lpthread
./test_runner
```

Manual build with LFList support:

```bash
cd Test && g++ -std=gnu++17 -D_GNU_SOURCE -DDEBUG=1 -g \
  -I ../Source/include -I ../.d/oneTBB/include \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_clist.cpp test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_cuset.cpp test_lflist.cpp test_lstring.cpp test_rqlist.cpp test_semaphore.cpp \
  ../Source/condition.cpp ../Source/crexception.cpp ../Source/crstring.cpp ../Source/semaphore.cpp \
  -L../.d/oneTBB/lib -ltbb -lpthread -o test_runner

export DYLD_LIBRARY_PATH=/Volumes/Development/DV/Live/CRUtil/.d/oneTBB/lib:$DYLD_LIBRARY_PATH
./test_runner "[lflist]"
```

The test framework is Catch2 (single-header, v2.13.0, located at `Test/catch2.hpp`).

**Test Results:**
- Full suite (no LFList): 296 test cases, 2196 assertions
- Full suite (with LFList): 296+ test cases

See Documents/ONETBB_INTEGRATION.md for details on oneTBB installation and usage.
