# CRUtil

A modern C++17 utility library providing 128-bit arithmetic, concurrency primitives,
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
| `semaphore.h` | P()/V() semaphore wrapper over pthread_mutex (recursive and non-recursive) |
| `condition.h` | Condition variable with nanosecond-resolution timeouts, built on Semaphore |
| `crexception.h` | Exception class with file/line/function tracking and optional stack traces |
| `crtypes.h` | Common type definitions, time constants, platform-specific event headers |
| `endian.h` | Portable endianness detection (Linux, macOS, BSD, Windows) |
| `needs.h` | SFINAE template constraint helpers (`NEEDS`/`REQUIRES` macros) |
| `crlikely.h` | GCC branch prediction hints (`likely`/`unlikely`) |

## Quick Start

```cpp
#include "bigint128.h"
using namespace crunnable;

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
using namespace crunnable;

uint256_t x("115792089237316195423570985008687907853269984665640564039457584007913129639935");
uint256_t y(1000000000UL);

auto q = x / y;
auto r = x % y;
assert(q * y + r == x);
```

## Documentation

- **[Library Reference](Documents/CRUtil-Reference.md)** -- detailed API documentation for all components
- **[Changelog](Documents/CHANGELOG.md)** -- history of changes

## Building

```bash
bif do -l    # list available build targets
bif do 1     # build target #1
```

Or compile manually:

```bash
c++ -std=gnu++17 -I Source/include -c Source/crstring.cpp -o crstring.o
```

## Testing

Tests use [Catch2](https://github.com/catchorg/Catch2). The bigint suites have
78 test cases with 523 assertions covering arithmetic, conversions, constexpr,
noexcept, hex/oct output, bit utilities, std::hash, and std::numeric_limits
for both 128-bit and 256-bit integers.

## Project Structure

```
CRUtil/
  Source/include/    12 header files
  Source/            2 implementation files (.cpp)
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
