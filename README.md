# CRUtil

A modern C++17 utility library providing 128-bit and 256-bit arithmetic, concurrency primitives,
safe string handling, and portable platform abstractions. Built with
[BuildItFast](https://github.com/pyvadura/BuildItFast).

## Components

| Header | Description |
|--------|-------------|
| `bigint128.h` | 128-bit unsigned integer with full operator suite, constexpr/noexcept support, hex/oct output, bit utilities, std::hash, std::numeric_limits, and `_u128` literal |
| `bigint256.h` | 256-bit unsigned integer built on `uint128_t` pairs, with full operator suite, string I/O, std::hash, std::numeric_limits, and `_u256` literal |
| `bigint.h` | Backward-compatibility shim that includes `bigint128.h` |
| `ainteger.h` | Thread-safe atomic 64-bit integer with CAS, get-and-set, and atomic clamping |
| `crstring.h` | Safe C-string operations: snprintf, strncpy, trim, split, join with bounds checking |
| `lstring.h` | Compile-time XOR string obfuscation -- plaintext never appears in the binary |
| `crtimer.h` | Nanosecond-precision timer for measurement, sleeping, interval delays, and date formatting |
| `semaphore.h` | P()/V() semaphore wrapper over pthread_mutex; PP/VV macros capture call-site in DEBUG; SEMTRACE ring for lock contention analysis |
| `condition.h` | Condition variable with nanosecond-resolution timeouts; COND_DEBUG traces waitFor/raise cycles to stderr |
| `crexception.h` | Exception class with file/line/function tracking, demangled stack traces, and a rich macro API for throwing, catching, and reporting |
| `crtypes.h` | Common type definitions, time constants, platform-specific event headers |
| `endian.h` | Portable endianness detection (Linux, macOS, BSD, Windows) |
| `needs.h` | SFINAE template constraint helpers (`NEEDS`/`REQUIRES` macros) |
| `crlikely.h` | GCC branch prediction hints (`likely`/`unlikely`) |

## Quick Start

```cpp
#include "bigint128.h"
using namespace crutil;

uint128p_t a("340282366920938463463374607431768211455");  // 2^128 - 1
uint128p_t b(1000000000UL);

auto q = a / b;
auto r = a % b;
assert(q * b + r == a);

std::cout << std::hex << std::showbase << q << std::endl;
```

For 256-bit arithmetic:

```cpp
#include "bigint256.h"
using namespace crutil;

uint256_t x("115792089237316195423570985008687907853269984665640564039457584007913129639935");
uint256_t y(1000000000UL);

auto q = x / y;
auto r = x % y;
assert(q * y + r == x);
```

## CRException -- Throwing, Catching, and Stack Traces

CRException provides a set of macros that automatically capture file, line, and
function name at every throw/catch site.  Stack traces are demangled into
readable C++ names.

### Throwing

```cpp
#include "crexception.h"
using namespace crutil;

CRX_THROW("something went wrong: %s", detail);       // errno = -1
CRX_THROW_ERR(ENOENT, "file %s not found", path);    // explicit errno
CRX_TIF(ptr == nullptr, "unexpected null");           // throw if true
CRX_TUNLESS(count > 0, "count must be positive");     // throw if false
CRX_TIF_ERR(fd < 0, errno, "open failed: %s", path); // throw with errno if true
int* p = CRX_TIFNULL(getPointer());                   // throw on NULL, else return ptr
```

### Catching and Reporting

```cpp
try {
   doWork();
} catch (CRException& e) {
   // Append a full catch summary (pid, tid, file:line, raised detail) to a string
   string report;
   CRX_CAPTURE_CATCH(report, e);

   // Or write directly to a FILE*
   CRX_REPORT_CATCH(stderr, e);
}
```

### Stack Traces

```cpp
// Capture a stack trace to stderr, do not rethrow
CRX_STACKTRACE(stderr, -1, false, "checkpoint reached: state=%d", state);

// Capture a stack trace and rethrow the exception
CRX_STACKTRACE(stderr, errno, true, "operation failed: %s", msg);

// CRX_REPORT_TRACE is an alias for CRX_STACKTRACE
CRX_REPORT_TRACE(stderr, -1, false, "diagnostic trace");
```

### CRException and Thread Cancellation

`CRX_THROW_CHK` and the rethrow path of `CRX_STACKTRACE` consult an internal
cancel map.  Call `CRException::notifyCancel(tid)` to mark a thread as
canceled; subsequent throws from **other** threads are suppressed while the
canceled thread shuts down.  Call `CRException::clearCancel(tid)` when the
thread that trows is joined.

## Semaphore -- DEBUG and SEMTRACE Modes

In **release** builds, `PP` and `VV` expand to plain `P()` / `V()` calls.

In **DEBUG** builds (`-DDEBUG`), `PP` and `VV` automatically capture
`__FILE__`, `__METHOD_NAME__`, and `__LINE__`.  Error messages and stack traces
then report the exact lock/unlock call site, the previous owner thread, and a
full acquisition history.  Use-after-destroy is also detected.

When compiled with both `-DDEBUG` and `-DSEMTRACE`, every `PP`/`VV` call is
recorded in a global trace ring.  Each entry stores:

- timestamp (nanosecond precision)
- thread id
- semaphore pointer
- P or V flag
- cost in nanoseconds
- call-site string

Call `semtracedump(stderr)` to dump the ring -- invaluable for diagnosing lock
contention and ordering issues.

## Condition -- COND_DEBUG Mode

`COND_DEBUG` is defined by default in `condition.h`.  Each `Condition` instance
has a debug flag (off by default) that can be enabled via the constructor's
`dflag` parameter or by calling `setDebug(true)` at runtime.  When active,
`CO_DEBUG` traces are emitted to stderr showing:

- thread id
- broadcast vs. normal mode
- fired count and waiter count
- timeout value

This makes it straightforward to diagnose missed signals, spurious wakeups, and
ordering problems in multi-threaded code.

## Documentation

- **[Library Reference](Documents/CRUtil-Reference.md)** -- detailed API documentation for all components
- **[Changelog](Documents/CHANGELOG.md)** -- history of changes

## Building

### Bif

```bash
bif do -l    # list available build targets
bif do 1     # build target #1
```

### CMake

A `CMakeLists.txt` is provided for environments that prefer CMake:

```bash
cmake -B build .
cmake --build build                # library -> Build/
cmake --build build --target package   # tar.gz -> Artifacts/

# with tests
cmake -B build -DCRUTIL_BUILD_TESTS=ON .
cmake --build build
ctest --test-dir build
```

### Manual Compilation

```bash
c++ -std=gnu++17 -I Source/include -c Source/crstring.cpp -o crstring.o
```

## Testing

Tests use [Catch2](https://github.com/catchorg/Catch2) (v2.13.0, single-header).
The full suite has 164 test cases with 966 assertions covering arithmetic,
conversions, constexpr, noexcept, hex/oct output, bit utilities, std::hash,
std::numeric_limits, string operations, timers, conditions, exceptions, and
obfuscation.

```bash
bif test    # build and run all tests
```

Or manually:

```bash
bif do 0    # build the debug library first

cd Test && g++ -std=gnu++17 -D_GNU_SOURCE -I ../Source/include -o test_runner \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_lstring.cpp ../Source/crstring.cpp -lpthread

./test_runner            # run all tests
./test_runner "[bigint256]"  # run a specific tag
```

## Project Structure

```
CRUtil/
  Source/include/    14 header files
  Source/            1 implementation file (.cpp)
  Test/              7 test files (Catch2 + pytest)
  Documents/         Reference docs, changelog
  Config/            Build configuration and plugins
  Build/             Intermediate build artifacts
  Artifacts/         Final built libraries
  bif.js5            BuildItFast project definition
  imap.yml           Build inference map
```

## License

See the license file in the distribution root.
