# CRUtil Project Notes

## Running Tests

`bif test` does not currently work (no test targets are configured in the inference map).

To run the tests manually:

1. Build the library:
   ```
   bif do 0
   ```

2. Compile the test runner from the project root:
   ```
   cd Test && g++ -std=gnu++17 -D_GNU_SOURCE -I ../Source/include -o test_runner \
     test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
     test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
     test_lstring.cpp test_semaphore.cpp ../Source/crstring.cpp -lpthread
   ```

3. Run the tests:
   ```
   ./test_runner
   ```

The test framework is Catch2 (single-header, v2.13.0, located at `Test/catch2.hpp`).

## LFList Tests (Lock-Free List)

LFList tests require oneTBB library. It's installed locally at `.d/oneTBB/`.

To compile and run tests with LFList support:

```bash
cd Test && g++ -std=gnu++17 -D_GNU_SOURCE \
  -I ../Source/include -I ../.d/oneTBB/include \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_lstring.cpp test_semaphore.cpp test_lflist.cpp \
  ../Source/crstring.cpp ../Source/condition.cpp ../Source/crexception.cpp ../Source/semaphore.cpp \
  -L../.d/oneTBB/lib -ltbb -lpthread -o test_runner

# Run tests (set DYLD_LIBRARY_PATH for oneTBB)
export DYLD_LIBRARY_PATH=/Volumes/Development/DV/Live/CRUtil/.d/oneTBB/lib:$DYLD_LIBRARY_PATH
./test_runner

# Or run only LFList tests
./test_runner "[lflist]"
```

**Test Results:**
- 37 LFList test cases
- 2087 assertions
- Full test suite: 219 test cases, 3111 assertions

See Documents/ONETBB_INTEGRATION.md for details on oneTBB installation and usage.
