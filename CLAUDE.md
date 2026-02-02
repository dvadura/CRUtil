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
   cd Test && g++ -std=gnu++17 -I ../Source/include -o test_runner \
     test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
     test_condition.cpp test_crstring.cpp test_crtimer.cpp test_lstring.cpp \
     ../Source/crstring.cpp -lpthread
   ```

3. Run the tests:
   ```
   ./test_runner
   ```

The test framework is Catch2 (single-header, v2.13.0, located at `Test/catch2.hpp`).
