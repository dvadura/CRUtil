# CRUtil Library Reference

CRUtil is a modern C++17 utility library providing 128-bit and 256-bit arithmetic, concurrency
primitives, thread-safe containers, lock-free queues, safe shared pointers, safe string handling,
compile-time string obfuscation, nanosecond-precision timing, and portable platform abstractions.
All public types live in the `crutil` namespace.

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
9. [Thread-safe List (clist.h)](#thread-safe-list)
10. [Concurrent Unordered Set (cuset.h)](#concurrent-unordered-set)
11. [Lock-Free Lists (ilflist.h, lflist.h, rqlist.h)](#lock-free-lists)
12. [Safe Shared Pointer (sharedptr.h)](#safe-shared-pointer)
13. [Exception (crexception.h)](#exception)
14. [Platform & Types (crtypes.h, endian.h, crlikely.h, needs.h, crtp.h)](#platform--types)
15. [Building](#building)
16. [Testing](#testing)

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

Compile-time string obfuscation using a chained XOR/add/rotate encoding. Plaintext
string literals never appear in the compiled binary.

```cpp
#include "lstring.h"
using namespace crutil;

// Encode at compile time
constexpr auto secret = LString::encode("my secret string");

// Decode at runtime — returns std::string by value
std::string plain = LString::decode(secret);
// plain == "my secret string"
```

Encoded strings can be stored in `constexpr` containers:

```cpp
constexpr auto passwords = std::array{
   LString::encode("password1"),
   LString::encode("password2"),
};
```

### Encoding Algorithm

Each byte is transformed by a four-step pipeline before being stored:

**Encode:** `XOR(position key)` → `XOR(prev encoded byte)` → `ADD(key2)` → `ROTL(3)`

**Decode:** `ROTR(3)` → `SUB(key2)` → `XOR(prev encoded byte)` → `XOR(position key)`

Chaining each byte's output into the next means identical plaintext characters
produce different encoded values, preventing partial byte recovery. The combination
of XOR, addition, and bit rotation makes the encoding non-trivial to identify in
disassembly.

### API

| Function | Description |
|----------|-------------|
| `LString::encode(s)` | Encodes string literal `s` at compile time; returns `lstring<N>` |
| `LString::decode(enc)` | Decodes `lstring<N>` at runtime; returns `std::string` |
| `LString::encode_byte(c, i, prev)` | `constexpr` single-byte encode step |
| `LString::decode_byte(x, i, prev)` | `constexpr` single-byte decode step |

The `lstring<N>` type is the encoded container and has a single member `char data[N]`.

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
**Implementation:** `Source/semaphore.cpp`

A pthread_mutex wrapper providing classical P() and V() semaphore operations.
Supports recursive and non-recursive modes.

```cpp
#include "semaphore.h"
using namespace crutil;

Semaphore sem;                // non-recursive
Semaphore sem_r(true);        // recursive

sem.PP;   // lock   (P operation)
sem.VV;   // unlock (V operation)
```

Try-lock:

```cpp
sem.TRYPP;   // returns immediately, does not block if already locked
```

### DEBUG vs Release

In **release** builds, `PP` and `VV` expand to plain `P()` / `V()` calls with no
overhead.

In **DEBUG** builds (`-DDEBUG`), `PP` and `VV` automatically capture `__FILE__`,
`__METHOD_NAME__`, and `__LINE__`.  This enables:

- **Call-site reporting** -- error messages and stack traces include the exact
  file, function, and line of each lock/unlock.
- **Use-after-destroy detection** -- attempting to lock or unlock a destroyed
  semaphore throws with a diagnostic message.
- **Acquisition history** -- a per-semaphore string (`m_where`) records the full
  lock/unlock history, included in exception messages for debugging ordering
  issues.

### SEMTRACE

When compiled with both `-DDEBUG` and `-DSEMTRACE`, every `PP`/`VV` call is
recorded in a global trace ring via `semtraceadd()`.  Each entry stores:

| Field | Description |
|-------|-------------|
| `now` | Timestamp (nanosecond precision) |
| `tid` | Thread id |
| `sem` | Semaphore pointer |
| `porv` | true = P (lock), false = V (unlock) |
| `cost` | Time spent waiting (nanoseconds) |
| `where` | Call-site string (file::method:line) |

Call `semtracedump(FILE*)` to dump the collected ring to a file descriptor:

```cpp
semtracedump(stderr);   // dump to stderr (default)
```

### Verbose Mode

Each semaphore has a verbose tag (default `Semaphore::VERBTAG`).  When set to a
non-NULL string, lock/unlock operations emit additional `fprintf(stderr, ...)`
traces showing the owner thread, mutex address, and depth.

```cpp
sem.setVerbose("MySem");    // enable verbose output
sem.setVerbose(NULL);       // disable (NULL suppresses output)
```

---

## Condition Variable

**Header:** `Source/include/condition.h`
**Implementation:** `Source/condition.cpp`

A pthread_cond wrapper supporting nanosecond-resolution timeouts, built on top of
the Semaphore class.  Supports both broadcast and single-event modes.

```cpp
#include "condition.h"
using namespace crutil;

Condition cond;                       // single-event mode
Condition cond_bc(true);              // broadcast mode
Condition cond_dbg(false, true);      // single-event with debug tracing

// Wait (blocking)
cond.waitFor();

// Wait with timeout (nanoseconds)
int result = cond.waitFor(NS_IN_ONE_SEC);  // 1-second timeout

// Signal one waiter
cond.raise();

// Broadcast to all waiters (broadcast mode)
cond_bc.raise();
```

### COND_DEBUG

`COND_DEBUG` is defined by default in the header.  Each `Condition` instance has a
debug flag (off by default) controllable via the constructor's `dflag` parameter or
at runtime:

```cpp
Condition cond(false, true);   // dflag=true enables tracing at construction
cond.setDebug(true);           // or enable at runtime
cond.setDebug(false);          // disable
```

When active, `CO_DEBUG` traces are emitted to stderr showing:

| Field | Example |
|-------|---------|
| Thread id | `COND(12345)` |
| Semaphore tag | `[MySem,B]` or `[MySem,N]` (broadcast/normal) |
| Operation | `Wait`, `Raise`, `wait enter`, `wait exit` |
| State | `m_fired=1,m_waiters=2,to=1000000000` |

This makes it straightforward to diagnose missed signals, spurious wakeups, and
ordering problems in multi-threaded code.

---

## Thread-safe List

**Header:** `Source/include/clist.h` (header-only)

`CList<T>` is a thread-safe concurrent list/queue backed by `std::deque<T>`. All
operations are O(1) and fully synchronized via an internal recursive mutex. A built-in
`Condition` supports efficient producer-consumer blocking.

```cpp
#include "clist.h"
using namespace crutil;

CList<int> list;                    // empty list
CList<int> list(42);                // single-item list
CList<int> list(std::move(other));  // move construct
```

An optional `tag` string names the internal condition variable for debugging.

### Size and Inspection

```cpp
size_t n  = list.size();    // current element count
bool   e  = list.empty();   // true if empty
int    f  = list.front();   // copy of first element (throws if empty)
int    b  = list.back();    // copy of last element (throws if empty)
```

`front()` and `back()` return by **copy** to ensure thread safety.
Iterators and `operator[]` are intentionally absent.

### Adding Elements

```cpp
list.push_front(item);             // prepend (signals waiters by default)
list.push_back(item);              // append  (signals waiters by default)
list.push_back(item, false);       // append without signaling
list.add(item);                    // alias for push_back
list.add(std::move(other_list));   // splice all elements from another CList
list.splice(std::move(other));     // move-splice; returns new size
```

### Removing Elements

```cpp
list.pop_front();                        // discard first element (throws if empty)
list.pop_back();                         // discard last element (throws if empty)

int v = list.remove_front();             // remove and return first (throws if empty)
int v = list.remove_front(false);        // remove and return first (returns T{} if empty, sets errno=-1)
int v = list.remove_back();             // remove and return last (throws if empty)
int v = list.remove_back(false);         // no-throw variant

int v = list.pfpb();                     // pop-front-push-back: rotate first to end, return it

unsigned int n = list.remove(value);     // erase all matching elements; returns new size
```

### Waiting and Signaling

```cpp
bool ok = list.waitFor();              // block until non-empty (indefinite)
bool ok = list.waitFor(NS_IN_ONE_SEC); // block with nanosecond timeout; false = timed out
list.raise();                          // wake waiters even if list is empty (e.g. on shutdown)
```

### Bulk Operations

```cpp
list.clear();    // empty the list and wake any waiters
list.fit();      // release excess deque capacity (shrink_to_fit)
```

### Atomic Multi-step Access

```cpp
size_t n = list.freeze();   // lock and return size (recursive mutex)
// ... inspect m_data safely ...
list.thaw();                // unlock
```

---

## Concurrent Unordered Set

**Header:** `Source/include/cuset.h` (header-only)

`CUSet<T>` is a thread-safe wrapper around `std::unordered_set<T>`. All operations
are internally synchronized via a recursive mutex. Set semantics are enforced:
duplicate insertions are idempotent. A built-in `Condition` supports blocking on
non-empty.

```cpp
#include "cuset.h"
using namespace crutil;

CUSet<int> set;                      // empty set
CUSet<int> set(item);                // single-element set
CUSet<int> set(std::move(other));    // move construct
```

### Query

```cpp
size_t n  = set.size();           // point-in-time element count (may be stale immediately)
bool   e  = set.empty();          // point-in-time emptiness check
bool   c  = set.contains(value);  // membership test
```

> **Note:** Results may be stale by the time the caller acts on them. Use
> `freeze()`/`thaw()` for consistent multi-step reads.

### Adding Elements

```cpp
set.add(item);                     // insert (idempotent; signals waiters)
set.push_back(item);               // alias for add
set.push_front(item);              // alias for add
set.add(std::move(other_set));     // splice all elements from another CUSet
set.splice(std::move(other));      // move-splice; returns new size
```

### Removing Elements

```cpp
T v    = set.remove_front();       // remove and return an arbitrary element (throws if empty)
int n  = set.remove(value);        // erase by value; returns 1 if found, 0 if not (throws if empty)
```

### Waiting and Signaling

```cpp
bool ok = set.waitFor();               // block until non-empty (indefinite)
bool ok = set.waitFor(NS_IN_ONE_SEC);  // block with nanosecond timeout; false = timed out
set.raise();                           // wake waiters unconditionally
```

### Bulk Operations

```cpp
set.clear();   // empty the set and wake any waiters
```

### Atomic Multi-step Access

```cpp
size_t n = set.freeze();   // lock and return size
// ... perform multiple consistent reads ...
set.thaw();                // unlock
```

---

## Lock-Free Lists

CRUtil provides three related types for lock-free concurrent queues, built around a
common abstract interface.

### ILFList — Interface

**Header:** `Source/include/ilflist.h` (header-only)

Abstract base class defining the interface shared by all lock-free list implementations.

```cpp
virtual size_t size()                                  = 0;
virtual bool   empty()                                 = 0;
virtual T      remove_front(bool* success, bool throwe = true) = 0;
virtual bool   push_back(const T& item, bool raise = true)     = 0;
virtual bool   waitFor(uint64_t timeout = 0)           = 0;
virtual void   raise()                                 = 0;
virtual void   clear()                                 = 0;
```

Use `ILFList<T>*` to write code that is agnostic between `LFList` and `RQList`.

---

### LFList — Lock-Free List (oneTBB)

**Header:** `Source/include/lflist.h` (header-only)
**Requires:** Intel oneTBB (`tbb/concurrent_queue.h`)

`LFList<T>` wraps `tbb::concurrent_queue<T>` with a `Condition` for blocking
wait/signal patterns. All operations are thread-safe for any number of producers
and consumers.

```cpp
#include "lflist.h"
using namespace crutil;

LFList<int*> queue;            // empty queue
LFList<int*> queue(item);      // single-item queue
LFList<int*> queue(std::move(other));  // move construct
```

#### Push

```cpp
queue.push(item);              // enqueue (signals waiters by default)
queue.push(item, false);       // enqueue without signaling
queue.push_back(item);         // alias for push
```

#### Pop

```cpp
bool ok;
T item = queue.remove(&ok);            // dequeue; ok=true if item was available
T item = queue.remove(&ok, false);     // no-throw variant
T item = queue.remove_front(&ok);      // alias for remove
T item = queue.pfpb();                 // pop-front-push-back: rotate, return item
```

#### Size and State

```cpp
size_t n = queue.size();    // approximate element count (tbb::unsafe_size)
bool   e = queue.empty();   // true if empty
```

#### Waiting and Signaling

```cpp
bool ok = queue.waitFor();              // block until non-empty (indefinite)
bool ok = queue.waitFor(NS_IN_ONE_SEC); // block with nanosecond timeout
queue.raise();                          // wake waiters unconditionally (e.g. on shutdown)
queue.clear();                          // empty the queue and wake waiters
```

> **Dependency:** `LFList` requires Intel oneTBB. The library is installed locally
> at `.d/oneTBB/`. See `Documents/ONETBB_INTEGRATION.md` for build details.

---

### RQList — Lock-Free Ring Queue (MPSC)

**Header:** `Source/include/rqlist.h` (header-only)

`RQList<T, TSIZE>` is a lock-free ring queue designed for **multi-producer,
single-consumer** (MPSC) patterns. It uses atomic slot reservation to provide
thread-safe concurrent writes without locks or oneTBB.

| Template Parameter | Default | Description |
|-------------------|---------|-------------|
| `T` | — | Element type (must be copyable) |
| `TSIZE` | `2048` | Ring buffer capacity |

```cpp
#include "rqlist.h"
using namespace crutil;

RQList<MyEvent*> queue;           // capacity 2048
RQList<MyEvent*, 4096> queue;     // custom capacity
RQList<MyEvent*> queue("tag");    // named condition variable
```

#### Push (multiple producers safe)

```cpp
bool ok = queue.push(item);           // enqueue; false if queue is full
bool ok = queue.push(item, false);    // enqueue without signaling waiters
bool ok = queue.push_back(item);      // alias for push
```

#### Pop (single consumer only)

```cpp
bool ok;
T item = queue.remove(&ok);           // dequeue; ok=true if item was available
T item = queue.remove_front(&ok);     // alias for remove
```

#### State

```cpp
size_t n = queue.size();    // approximate (write_pos - read_pos)
bool   e = queue.empty();   // true if read slot is EMPTY
```

#### Waiting, Signaling, and Reset

```cpp
bool ok = queue.waitFor();              // block until non-empty
bool ok = queue.waitFor(timeout_ns);    // block with nanosecond timeout
queue.raise();                          // wake waiters
queue.clear();                          // reset all slots to EMPTY — NOT thread-safe
```

> **Important:** `clear()` must only be called when no other threads are accessing
> the queue.

#### Slot State Machine

Each ring slot progresses through three states:

```
EMPTY → RESERVED (writer claims slot) → FILLED (data written) → EMPTY (reader consumed)
```

The `RESERVED` state prevents a reader from consuming a slot before the writer
finishes writing, eliminating ABA hazards.

---

## Safe Shared Pointer

**Header:** `Source/include/sharedptr.h` (header-only)

`SharedPtr<T>` extends `std::shared_ptr<T>` with null-dereference protection.
Dereferencing a null `SharedPtr` throws a `CRException` instead of causing undefined
behavior.

```cpp
#include "sharedptr.h"
using namespace crutil;

// Always construct via make_shared
auto ptr = crutil::make_shared<MyClass>(arg1, arg2);

// Safe to use — throws CRException on null dereference
ptr->method();    // throws if ptr is null
*ptr = value;     // throws if ptr is null

// Construct from std::shared_ptr
std::shared_ptr<MyClass> std_ptr = std::make_shared<MyClass>();
SharedPtr<MyClass> safe_ptr(std_ptr);
```

### Construction Rules

- **No default constructor** — prevents uninitialized shared pointers.
- **No constructor from raw `T*`** — use `crutil::make_shared<T>()` instead.
- Copy, move, and construction from `std::shared_ptr<T>` are all supported.

```cpp
SharedPtr<T> a(other_shared_ptr);       // copy from SharedPtr
SharedPtr<T> b(std_shared_ptr);         // wrap existing std::shared_ptr
SharedPtr<T> c(std::move(a));           // move construct
```

### Null Behavior

A `SharedPtr` constructed from a null `std::shared_ptr` is valid to hold but throws
on dereference:

```cpp
SharedPtr<MyClass> null_ptr(std::shared_ptr<MyClass>());  // wraps null
null_ptr->method();  // throws CRException: "ptr is NULL"
```

### Inheritance

`SharedPtr<T>` inherits from `std::shared_ptr<T>`, so it is compatible with
`std::shared_ptr` APIs (`use_count()`, `reset()`, `get()`, boolean test, etc.).

---

## Exception

**Header:** `Source/include/crexception.h`
**Implementation:** `Source/crexception.cpp`

`CRException` extends `std::runtime_error` with file/line/function tracking,
demangled stack traces, and a rich macro API for throwing, catching, and reporting.

### Compile Flags

Stack traces require `_GNU_SOURCE` and debug symbols.  On Linux, link with
`-rdynamic` so `backtrace_symbols` can resolve names.  On macOS, no special linker
flag is needed.

```bash
# Linux
g++ -std=gnu++17 -D_GNU_SOURCE -DDEBUG=1 -Og t.cpp -rdynamic

# macOS
g++ -std=gnu++17 -D_GNU_SOURCE -DDEBUG=1 -Og t.cpp
```

### Throwing Macros

| Macro | Description |
|-------|-------------|
| `CRX_THROW(MSG, ...)` | Throw with errno=-1 |
| `CRX_THROW_ERR(ERR, MSG, ...)` | Throw with explicit errno |
| `CRX_THROW_CHK(ERR, MSG, ...)` | Throw unless current thread is canceled |
| `CRX_TIF(EXPR, MSG, ...)` | Throw if EXPR is true |
| `CRX_TUNLESS(EXPR, MSG, ...)` | Throw if EXPR is false |
| `CRX_TIF_ERR(EXPR, ERR, MSG, ...)` | Throw with errno if EXPR is true |
| `CRX_TIFNULL(PTR)` | Throw on NULL, otherwise return PTR |

All MSG arguments are printf-style format strings.

```cpp
CRX_THROW("something went wrong: %s", detail);
CRX_THROW_ERR(ENOENT, "file %s not found", path);
CRX_TIF(ptr == nullptr, "unexpected null");
CRX_TUNLESS(count > 0, "count must be positive");
CRX_TIF_ERR(fd < 0, errno, "open failed: %s", path);
int* p = CRX_TIFNULL(getPointer());   // returns the pointer if non-NULL
```

### Catching and Reporting Macros

| Macro | Description |
|-------|-------------|
| `CRX_CAPTURE_CATCH(STR, CRX)` | Append full catch summary (pid, tid, file:line, raised detail) to a `std::string` |
| `CRX_REPORT_CATCH(FD, CRX)` | Write catch summary to a `FILE*`; in DEBUG builds adds separator lines |

```cpp
try {
   doWork();
} catch (CRException& e) {
   // Append to a string for later use
   string report;
   CRX_CAPTURE_CATCH(report, e);

   // Or write directly to a file descriptor
   CRX_REPORT_CATCH(stderr, e);
}
```

### Stack Trace Macros

| Macro | Description |
|-------|-------------|
| `CRX_STACKTRACE(FD, ERR, RETHROW, MSG, ...)` | Capture a stack trace to FILE*; optionally rethrow |
| `CRX_REPORT_TRACE(FD, ERR, RETHROW, MSG, ...)` | Alias for CRX_STACKTRACE |

The RETHROW parameter controls whether the exception is re-thrown after reporting.

```cpp
// Capture a stack trace to stderr, do not rethrow
CRX_STACKTRACE(stderr, -1, false, "checkpoint: state=%d", state);

// Capture and rethrow
CRX_STACKTRACE(stderr, errno, true, "operation failed: %s", msg);
```

Stack traces are automatically demangled using `abi::__cxa_demangle()` so that
function names appear in readable C++ form rather than raw mangled symbols.

### Catching via operator<< and operator+=

```cpp
try {
   do_work();
} catch (CRException& e) {
   std::cerr << e;          // full output: file, line, function, message, errno, calltrace
   std::cerr << e.what();   // just the formatted message
   std::cerr << e.errmsg(); // system error string if errno > 0, else what()

   string out;
   out += e;                // append full output to string
}
```

### Accessors

| Method | Returns |
|--------|---------|
| `what()` | Formatted message (const char*) |
| `what(string&)` | Formatted message via reference |
| `errmsg()` | System error string for positive errno, else what() |
| `errmsg(string&)` | System error string via reference |
| `file()` | Source filename |
| `line()` | Source line number |
| `geterrno()` | Error number passed at throw time |
| `toString(string&)` | Full exception output via reference |

### Thread Cancellation

`CRX_THROW_CHK` and the rethrow path of `CRX_STACKTRACE` consult an internal
cancel map.  When a thread is marked as canceled, throws from **other** threads
are suppressed while the canceled thread shuts down.

```cpp
pid_t tid = CRX_GETTID();
CRException::notifyCancel(tid);    // mark thread as canceled
// ... shutdown work ...
CRException::clearCancel(tid);     // clear when the thread is joined
```

### Static-Only Mode

Setting static-only mode suppresses pid/tid and calltrace from the output, useful
for contexts where only the message matters:

```cpp
e.setStaticOnly(true);
std::cerr << e;   // omits pid/tid and calltrace
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
`__BIG_ENDIAN` on all platforms. Courtesy of [Mathias Panzenböck](https://gist.github.com/panzi/6856583).

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

### crtp.h

CRTP (Curiously Recurring Template Pattern) helper in the `crunnable` namespace:

```cpp
#include "crtp.h"

template <typename Derived>
class Base : public crunnable::crtp<Derived> {
   void foo() {
      this->tcast().derived_method();   // safe downcast, no manual static_cast
   }
};
```

`crtp<T>` provides two `tcast()` overloads — const and non-const — that perform
`static_cast<T const&>(*this)` and `static_cast<T&>(*this)` respectively. This
eliminates repetitive casting boilerplate in CRTP base classes.

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

### Requirements

- C++17 compiler (GCC or Clang)
- POSIX system (Linux, macOS)
- `__int128` support for intrinsic uint128_t (GCC/Clang on 64-bit)
- Intel oneTBB — required only for `LFList`. Installed locally at `.d/oneTBB/`.
  See `Documents/ONETBB_INTEGRATION.md` for installation details.

---

## Testing

Tests use the [Catch2](https://github.com/catchorg/Catch2) framework (v2.13.0,
single-header, included in `Test/catch2.hpp`).

| Test File | Component |
|-----------|-----------|
| test_bigint.cpp | bigint128.h (uint128p_t and uint128_t) |
| test_bigint256.cpp | bigint256.h (uint256_t) |
| test_crexception.cpp | crexception.h (macros, catch/report, stack traces, thread cancellation) |
| test_crstring.cpp | crstring.h |
| test_ainteger.cpp | ainteger.h |
| test_lstring.cpp | lstring.h |
| test_condition.cpp | condition.h |
| test_crtimer.cpp | crtimer.h |
| test_semaphore.cpp | semaphore.h |
| test_clist.cpp | clist.h |
| test_cuset.cpp | cuset.h |
| test_rqlist.cpp | rqlist.h |
| test_lflist.cpp | lflist.h — requires oneTBB |

The full suite (including LFList) has **219 test cases** and **3111 assertions**.
The non-oneTBB suite has **182 test cases**.

### Standard Test Build

```bash
bif do 0    # build the debug library first

cd Test && g++ -std=gnu++17 -D_GNU_SOURCE \
  -I ../Source/include \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_lstring.cpp test_semaphore.cpp test_clist.cpp test_rqlist.cpp test_cuset.cpp \
  ../Source/crstring.cpp ../Source/condition.cpp ../Source/crexception.cpp \
  ../Source/semaphore.cpp \
  -lpthread -o cl_test_runner

./cl_test_runner                    # run all non-TBB tests
./cl_test_runner "[bigint256]"      # run a specific tag
```

### LFList Test Build (requires oneTBB)

```bash
cd Test && g++ -std=gnu++17 -D_GNU_SOURCE \
  -I ../Source/include -I ../.d/oneTBB/include \
  test_main.cpp test_ainteger.cpp test_bigint.cpp test_bigint256.cpp \
  test_condition.cpp test_crexception.cpp test_crstring.cpp test_crtimer.cpp \
  test_lstring.cpp test_semaphore.cpp test_clist.cpp test_rqlist.cpp test_cuset.cpp \
  test_lflist.cpp \
  ../Source/crstring.cpp ../Source/condition.cpp ../Source/crexception.cpp \
  ../Source/semaphore.cpp \
  -L../.d/oneTBB/lib -ltbb -lpthread -o tbb+cl_test_runner

export DYLD_LIBRARY_PATH=/Volumes/Development/DV/Live/CRUtil/.d/oneTBB/lib:$DYLD_LIBRARY_PATH
./tbb+cl_test_runner                # run all tests including LFList
./tbb+cl_test_runner "[lflist]"     # run only LFList tests
```
