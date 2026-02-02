# CRUtil Library Reference

CRUtil is a modern C++17 utility library providing 128-bit and 256-bit arithmetic, concurrency
primitives, safe string handling, compile-time string obfuscation, nanosecond-precision
timing, and portable platform abstractions. All public types live in the `crutil` namespace.

Built with [BuildItFast](https://github.com/pyvadura/BuildItFast) (`bif`).

---

## Table of Contents

1. [128-bit Unsigned Integer (bigint128.h)](#128-bit-unsigned-integer)
2. [256-bit Unsigned Integer (bigint256.h)](#256-bit-unsigned-integer)
3. [Atomic Integer (ainteger.h)](#atomic-integer)
4. [Safe Strings (crstring.h)](#safe-strings)
5. [String Obfuscation (lstring.h)](#string-obfuscation)
6. [Timer (crtimer.h)](#timer)
7. [Semaphore (semaphore.h)](#semaphore)
8. [Condition Variable (condition.h)](#condition-variable)
9. [Exception (crexception.h)](#exception)
10. [Platform & Types (crtypes.h, endian.h, crlikely.h, needs.h)](#platform--types)
11. [Building](#building)
12. [Testing](#testing)

---

## 128-bit Unsigned Integer

**Header:** `Source/include/bigint128.h`

Provides two implementations of a 128-bit unsigned integer, selectable at compile time:

| Type | Class | When Available |
|------|-------|----------------|
| `uint128p_t` | `pair<uint64_t, uint64_t>` | Always (portable) |
| `uint128_t` | `single<unsigned __int128>` | When `INT128_INTRINSIC` is defined (GCC/Clang) |

Both types share the same public interface.

### Construction

```cpp
#include "bigint128.h"
using namespace crutil;

uint128p_t a;                              // default (uninitialized)
uint128p_t b(42UL);                        // from integral
uint128p_t c(0xABCD, 0x1234567890UL);     // (hi64, lo64)
uint128p_t d("340282366920938463463374607431768211455");  // from decimal string
uint128p_t e(d);                           // copy
```

Integral constructors are `constexpr` and `noexcept`. String constructors throw
`CRException` on invalid input.

### Arithmetic

All standard arithmetic operators are supported. Division and modulus throw
`CRException` on divide-by-zero.

```cpp
uint128p_t sum  = a + b;
uint128p_t diff = a - b;
uint128p_t prod = a * b;
uint128p_t quot = a / b;    // throws on b == 0
uint128p_t rem  = a % b;    // throws on b == 0

a += b;  a -= b;  a *= b;  a /= b;  a %= b;
++a;  a++;  --a;  a--;
```

Unary operators:

```cpp
uint128p_t pos = +a;        // identity
uint128p_t neg = -a;        // two's complement negation
```

### Bitwise and Shift

```cpp
uint128p_t r = a & b;   // AND
uint128p_t r = a | b;   // OR
uint128p_t r = a ^ b;   // XOR
uint128p_t r = ~a;      // NOT
uint128p_t r = a << 4;  // left shift
uint128p_t r = a >> 4;  // right shift
```

All bitwise and shift operators (and their compound assignment forms) are `constexpr`
and `noexcept` on the pair class for operations that don't require local temporaries.

### Comparison

All six comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`) work between two
128-bit values or between a 128-bit value and any integral type. Free-standing
operators allow `T op uint128_t` ordering as well.

### Conversion

```cpp
bool     b = a;                          // implicit, true if non-zero
uint64_t v = a;                          // implicit
uint32_t w = static_cast<uint32_t>(a);   // explicit (narrowing)
uint16_t x = static_cast<uint16_t>(a);   // explicit (narrowing)
uint8_t  y = static_cast<uint8_t>(a);    // explicit (narrowing)
```

### Named Accessors

```cpp
uint64_t lo = a.lo64();    // low 64 bits
uint64_t hi = a.hi64();    // high 64 bits
```

Both are `constexpr` and `noexcept`.

### String Output

```cpp
std::string s;
a.toString(s);       // decimal: "340282366920938463463374607431768211455"
a.toHexString(s);    // hex:     "ffffffffffffffffffffffffffffffff"
a.toOctString(s);    // octal:   "3777777777777777777777777777777777777777777"
```

The `operator<<` respects `std::ios` format flags:

```cpp
std::cout << std::hex << std::showbase << a;   // "0xffffffffffffffffffffffffffffffff"
std::cout << std::oct << std::showbase << a;   // "0377..."
std::cout << std::dec << a;                    // "340282..."
```

### Utility Methods

```cpp
a.isZero();    // true if value == 0
a.isOne();     // true if value == 1
a.isLow();     // true if high 64 bits are zero
a.isPow2();    // true if exactly one bit is set
a.getPow2();   // returns bit position (throws if not power of 2)
```

### Bit Utilities

```cpp
size_t pc  = a.popcount();      // number of set bits
size_t clz = a.countl_zero();   // leading zeros from MSB (128 if zero)
size_t ctz = a.countr_zero();   // trailing zeros from LSB (128 if zero)
```

### Standard Library Integration

**std::hash** -- enables use in `std::unordered_map` and `std::unordered_set`:

```cpp
std::unordered_map<uint128p_t, int> map;
map[uint128p_t(42)] = 1;
```

**std::numeric_limits** -- full specialization:

```cpp
using lim = std::numeric_limits<uint128p_t>;
static_assert(lim::digits == 128);
static_assert(lim::digits10 == 38);
auto max_val = lim::max();   // 2^128 - 1
```

### User-defined Literal

```cpp
using namespace crutil::literals;

auto big = "340282366920938463463374607431768211455"_u128;
```

Returns a `uint128_t` (intrinsic) when `INT128_INTRINSIC` is defined.

### constexpr and noexcept

Most operators are marked `noexcept`. Division and modulus are not (they throw on
divide-by-zero). String operations are not (they allocate or may throw on parse
errors).

Integral constructors, comparisons, bitwise, shift, addition/subtraction compound
assignments, and utility predicates are `constexpr`:

```cpp
constexpr uint128p_t a(10UL);
constexpr uint128p_t b(3UL);
static_assert(a > b);
static_assert((a & b) == uint128p_t(2UL));
```

Binary `+` and `-` on the pair class are not `constexpr` because they require
default-constructing a local temporary, which the packed union prevents.

### Free-standing Operators

All operators have free-standing versions for `T op uint128_t` and `T op= uint128_t`
where `T` is any integral type:

```cpp
uint64_t x = 100;
x += uint128p_t(10);      // compound assignment
auto r = 5UL + uint128p_t(10);  // binary
```

### Portability

The pair implementation uses a packed union with 8/16/32/64-bit array overlays for
the multiplication and division algorithms. These algorithms index the arrays
assuming little-endian byte order. A `static_assert` at compile time rejects
big-endian platforms.

---

## 256-bit Unsigned Integer

**Header:** `Source/include/bigint256.h`

Provides a 256-bit unsigned integer as `pair<uint128_t, uint128_t>`. Includes
`bigint128.h` automatically. All operations delegate to the 128-bit `m_hi`/`m_lo`
sub-objects — no raw byte array overlays, no endian-specific code.

| Type | Class | Description |
|------|-------|-------------|
| `uint256_t` | `pair<uint128_t, uint128_t>` | 256-bit unsigned integer |

### Construction

```cpp
#include "bigint256.h"
using namespace crutil;

uint256_t a;                                              // default
uint256_t b(42UL);                                        // from integral
uint256_t c(uint128_t(hi128), uint128_t(lo128));          // (hi, lo) halves
uint256_t d("115792089237316195423570985008687907853269984665640564039457584007913129639935");
uint256_t e(d);                                           // copy
```

### Arithmetic

All standard operators: `+`, `-`, `*`, `/`, `%`, compound assignments, `++`, `--`,
unary `+`/`-`. Division and modulus throw on divide-by-zero.

```cpp
uint256_t q = a / b;
uint256_t r = a % b;
assert(q * b + r == a);
```

### Bitwise, Shift, Comparison

Same interface as the 128-bit type. Shifts split across the 128-bit boundary.

### Named Accessors

```cpp
uint128_t lo = a.lo128();
uint128_t hi = a.hi128();
uint64_t  l  = a.lo64();   // lowest 64 bits
uint64_t  h  = a.hi64();   // highest 64 bits
```

### String Output

```cpp
std::string s;
a.toString(s);       // decimal
a.toHexString(s);    // hex (delegates to 128-bit halves)
a.toOctString(s);    // octal (shift-and-mask)

std::cout << std::hex << std::showbase << a;  // "0x..."
```

### Utility Methods

Same interface as 128-bit: `isZero()`, `isOne()`, `isLow()`, `isPow2()`,
`getPow2()`, `popcount()`, `countl_zero()`, `countr_zero()`.

### Standard Library Integration

`std::hash<uint256_t>` and `std::numeric_limits<uint256_t>` (digits=256,
digits10=77, max_digits10=78).

### User-defined Literal

```cpp
using namespace crutil::literals;
auto big = "115792089237316195423570985008687907853269984665640564039457584007913129639935"_u256;
```

### Free-standing Operators

All comparison, compound assignment, and binary operators have `T op uint256_t`
free-standing versions for integral types.

---

## Atomic Integer

**Header:** `Source/include/ainteger.h`

Thread-safe 64-bit signed integer using `std::atomic<int64_t>`.

```cpp
#include "ainteger.h"
using namespace crutil;

AInteger counter;          // default: 0
AInteger counter(42);      // initialize to 42

counter.inc();             // atomic increment
counter.dec();             // atomic decrement
counter.add(10);           // atomic add
counter.sub(5);            // atomic subtract
counter.set(100);          // atomic store
int64_t v = counter.value(); // atomic load
```

### Atomic Primitives

```cpp
bool ok = counter.cas(expected, newval);   // compare-and-swap
int64_t old = counter.gas(newval);         // get-and-set (exchange)
bool ok = counter.test_and_set(exp, val);  // alias for CAS
```

### Clamping

```cpp
counter.floor(10);    // atomically ensure value >= 10
counter.ceiling(100); // atomically ensure value <= 100
```

### Operators

Supports `++`, `--` (prefix/postfix), `+=`, `-=`, all six comparison operators
(against `AInteger` or any `T`), and `operator<<` for ostream output.

---

## Safe Strings

**Header:** `Source/include/crstring.h`
**Implementation:** `Source/crstring.cpp`

The `CRS` class provides static methods for safe C-string manipulation with bounds
checking.

### Convenience Macros

```cpp
char buf[256];
CRSnprintf(buf, "Hello %s, you are %d", name, age);  // safe snprintf
CRStrcpy(buf, source);                                // safe strncpy
CRStradd(buf, " world");                              // safe append
CRStrjoin(buf, "/path");                              // safe join
```

The macros automatically pass `sizeof(buf)` as the size argument.

### String Operations

```cpp
// Trimming (C-strings and std::string)
CRS::trim(str);           // trim both ends
CRS::ltrim(str);          // trim left
CRS::rtrim(str);          // trim right
CRS::trim(str, " \t");   // custom whitespace set

// Splitting
std::vector<std::string> parts;
CRS::split("a,b,c", parts, ",");

// Searching
CRS::skip(str, " \t");   // skip matching characters
CRS::skip(str, "abc", false);  // skip non-matching characters

// Validation
CRS::empty(str);          // true if null or zero-length
CRS::noe(str);            // null or empty
CRS::eos(str);            // end of string

// Misc
CRS::replace(subject, search, replacement);  // returns new string
CRS::throwifempty(str);   // throws CRException if empty
```

---

## String Obfuscation

**Header:** `Source/include/lstring.h` (header-only)

Compile-time XOR-based string obfuscation. Plaintext string literals never appear in
the compiled binary.

```cpp
#include "lstring.h"
using namespace crutil;

// Encode at compile time
constexpr auto secret = Obfuscate::encode("my secret string");

// Decode at runtime
std::string plain;
Obfuscate::decode(secret.data, plain);
// plain == "my secret string"
```

Encoded strings can be stored in constexpr containers:

```cpp
constexpr auto passwords = std::array{
   Obfuscate::encode("password1"),
   Obfuscate::encode("password2"),
};
```

The `cstr()` function is a legacy alias for `Obfuscate::encode()`.

---

## Timer

**Header:** `Source/include/crtimer.h`

Nanosecond-precision timer (`CRTime` / `crts`) for measuring elapsed time, sleeping,
interval delays, and date/time formatting.

```cpp
#include "crtimer.h"
using namespace crutil;

CRTime now;                    // captures current time
CRTime later(NS_IN_ONE_SEC);  // current time + 1 second offset

uint64_t ns = now.nsec();      // nanoseconds since epoch
uint64_t us = now.usec();      // microseconds
uint64_t ms = now.msec();      // milliseconds
uint64_t s  = now.sec();       // seconds
```

### Sleeping

```cpp
CRTime t;
t.nsleep(500000000UL);         // sleep 500ms (in nanoseconds)
t.usleep(500000);              // sleep 500ms (in microseconds)
t.msleep(500);                 // sleep 500ms (in milliseconds)
t.sleep(1);                    // sleep 1 second
```

### Interval Delays

Compensates for elapsed time to maintain a target interval:

```cpp
CRTime t;
while (running) {
   do_work();
   t.ndelay(NS_IN_ONE_SEC);   // maintain 1-second interval
}
```

### Elapsed Time

```cpp
CRTime start;
do_work();
uint64_t elapsed_ns = start.diff();
uint64_t elapsed_us = start.diffus();
uint64_t elapsed_ms = start.diffms();
```

### Date/Time Formatting

```cpp
std::string s;
now.rfc1123(s);     // "Sun, 02 Feb 2026 18:19:00 GMT"
now.rfc822(s);      // "Sun, 02 Feb 2026 18:19:00 +0000"
now.iso8601(s);     // "2026-02-02 18:19:00 +00:00"
now.sql(s);         // alias for iso8601

// Parse from string
CRTime parsed("2026-02-02 18:19:00 +00:00");
```

### Conversion

```cpp
ts_t& ts = now.ts();    // struct timespec (sec/nsec)
tv_t& tv = now.tv();    // struct timeval (sec/usec)
```

Supports comparison operators and arithmetic (`+=`, `-=`) against other `CRTime`
values or raw nanosecond counts.

### Time Constants

Defined in `crtypes.h`:

| Constant | Value |
|----------|-------|
| `NS_IN_ONE_SEC` | 1,000,000,000 |
| `NS_IN_ONE_MSEC` | 1,000,000 |
| `NS_IN_ONE_USEC` | 1,000 |
| `US_IN_ONE_SEC` | 1,000,000 |
| `MS_IN_ONE_SEC` | 1,000 |
| `NS_IN_ONE_MIN` | 60,000,000,000 |

---

## Semaphore

**Header:** `Source/include/semaphore.h`

A pthread_mutex wrapper providing classical P() and V() semaphore operations.
Supports recursive and non-recursive modes.

```cpp
#include "semaphore.h"
using namespace crutil;

Semaphore sem;                // non-recursive
Semaphore sem_r(true);        // recursive

sem.PP;   // lock   (P operation, debug macro records file/function/line)
sem.VV;   // unlock (V operation)
```

In release builds, use `sem.PP` / `sem.VV`. These expand to `P()` / `V()` without
debug info.

Try-lock:

```cpp
sem.TRYPP;   // returns immediately, does not block if already locked
```

---

## Condition Variable

**Header:** `Source/include/condition.h`

A pthread_cond wrapper supporting nanosecond-resolution timeouts, built on top of
the Semaphore class.

```cpp
#include "condition.h"
using namespace crutil;

Condition cond;

// Wait (blocking)
cond.waitFor();

// Wait with timeout (nanoseconds)
cond.waitFor(NS_IN_ONE_SEC);  // 1-second timeout

// Signal one waiter
cond.raise();

// Broadcast to all waiters
cond.raise(100);  // signal up to 100 times
```

---

## Exception

**Header:** `Source/include/crexception.h`

`CRException` extends `std::runtime_error` with file/line/function tracking and
optional stack traces (when compiled with `DEBUG` and linked with `-rdynamic`).

### Throwing

```cpp
CRX_THROW("something went wrong: %s", detail);
CRX_THROW_ERR(errno, "I/O error: %s", path);
CRX_TIF(ptr == nullptr, "null pointer");          // throw if true
CRX_TUNLESS(size > 0, "size must be positive");   // throw unless true
auto p = CRX_TIFNULL(ptr);                        // throw if null, return ptr
```

### Catching

```cpp
try {
   do_work();
} catch (CRException& e) {
   std::cerr << e;          // prints file, line, function, message, stack trace
   std::cerr << e.what();   // just the message
   std::cerr << e.errmsg(); // system error message if errno was set
}
```

### Thread Cancellation

```cpp
CRException::notifyCancel(tid);    // mark thread as canceled
CRException::clearCancel(tid);     // clear cancellation
CRException::isThreadCanceled(tid); // check if canceled
```

---

## Platform & Types

### crtypes.h

Common type definitions and system includes. Defines time constants, includes
platform-specific event headers (epoll on Linux, kqueue on macOS), and provides
network byte order macros (`htonll`/`ntohll`).

### endian.h

Portable endianness detection and byte-swap macros. Supports Linux, macOS, FreeBSD,
NetBSD, OpenBSD, DragonFly, Windows. Defines `__BYTE_ORDER`, `__LITTLE_ENDIAN`,
`__BIG_ENDIAN` on all platforms.

### crlikely.h

GCC branch prediction hints:

```cpp
if (likely(condition)) { ... }    // branch is expected to be taken
if (unlikely(error)) { ... }     // branch is rarely taken
```

Falls back to plain expressions on non-GCC compilers.

### needs.h

SFINAE template constraint helpers:

```cpp
// Use NEEDS when multiple overloads have the same signature
template <typename T, NEEDS(std::is_integral<T>())>
void process(T value);

// Use REQUIRES for simpler cases without overload conflicts
template <typename T, REQUIRES(std::is_floating_point<T>::value)>
void process(T value);
```

`NEEDS` generates unique types per `__LINE__` to disambiguate overloads with identical
signatures but different constraints.

---

## Building

CRUtil uses [BuildItFast](https://github.com/pyvadura/BuildItFast) (bif):

```bash
# List available build targets
bif do -l

# Build a specific target (by number from the list)
bif do 1

# Get help
bif -h
bif do -h
```

### Manual Compilation

```bash
c++ -std=gnu++17 -I Source/include -c Source/crstring.cpp -o crstring.o
```

### Requirements

- C++17 compiler (GCC or Clang)
- POSIX system (Linux, macOS)
- `__int128` support for intrinsic uint128_t (GCC/Clang on 64-bit)

---

## Testing

Tests use the [Catch2](https://github.com/catchorg/Catch2) framework (v2.13.0,
single-header, included in `Test/catch2.hpp`). The full suite has 132 test cases
with 907 assertions.

| Test File | Component |
|-----------|-----------|
| test_bigint.cpp | bigint128.h (uint128p_t and uint128_t) |
| test_bigint256.cpp | bigint256.h (uint256_t) |
| test_crstring.cpp | crstring.h |
| test_ainteger.cpp | ainteger.h |
| test_lstring.cpp | lstring.h |
| test_condition.cpp | condition.h |
| test_crtimer.cpp | crtimer.h |

Build and run:

```bash
bif do 0    # build the debug library first

cd Test && g++ -std=gnu++17 -I ../Source/include -o test_runner \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_condition.cpp test_crstring.cpp test_crtimer.cpp test_lstring.cpp \
  ../Source/crstring.cpp -lpthread

./test_runner                # run all tests
./test_runner "[bigint256]"  # run a specific tag
```

Python tests use pytest (`Test/test_hello.py`).
