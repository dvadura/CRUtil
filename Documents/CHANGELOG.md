===============================================================================================
# Fix security issues: buffer overflow checks and type safety

February 14, 2026 :: 01:30 PM EST (UTC: 18:30 UTC)

Fixed several security and correctness issues identified during code audit of
include headers and string manipulation functions.

1. **crstring.cpp buffer overflow prevention** — Fixed unsigned integer underflow
   in _strncpy() by checking if size == 0 before arithmetic, preventing
   implementation-defined behavior when casting underflowed value to signed int.
   Added explicit overflow check in add() to prevent arithmetic underflow when
   calculating remaining buffer space.

2. **ainteger.h type safety** — Changed pre_add() and pre_sub() return types from
   uint64_t to int64_t to match the underlying atomic<int64_t> type, eliminating
   confusing signed-to-unsigned reinterpretation for negative values.

3. **ainteger.h const correctness** — Added const qualifier to uint64() method
   since it doesn't modify state, allowing use with const AInteger objects.

===============================================================================================
# Improve header documentation for readability and Doxygen compatibility

February 14, 2026 :: 01:00 PM EST (UTC: 18:00 UTC)

Improved header documentation quality with comprehensive additions to crstring.h,
modernized Doxygen command style, and resolved documentation gaps for better API
clarity and modern tooling compatibility.

1. **crstring.h comprehensive documentation** — Added complete documentation for
   previously undocumented header (79 lines, 30+ public methods). Includes:
   - Standard copyright header and file-level `@file` documentation
   - Class-level `@class` documentation with usage examples for CRS
   - All 4 macros documented with `@def` blocks (CRSnprintf, CRStrcpy, CRStradd, CRStrjoin)
   - Complete method documentation with `@brief`, `@param`, `@returns` for all public methods
   - Template parameter documentation with `@tparam` for throwifempty methods
   - Organized method grouping with `@name` and `@{...@}` for: Safe Formatting,
     Trimming Operations, String Manipulation, and Utility Operations

2. **crstring.cpp file documentation** — Added standard copyright header and
   `@file` block referencing crstring.h for API documentation.

3. **crtimer.h modernization and fixes** — Fixed typo ("wraper" → "wrapper" on
   line 3). Converted from backslash commands to modern `@` style (`\brief` →
   `@brief`, etc.). Moved copyright/license from Doxygen `\copy` and `\license`
   tags to standard comment header at file top. Fixed project name reference in
   crexception.h ("CRunnable" → "CRUtil").

4. **Complete style migration** — All 13 header files in Source/include/ now use
   modern Doxygen `@` command style instead of legacy backslash style:
   - Phase 1 files: crstring.h, crtimer.h (from original plan)
   - Phase 3 files: ainteger.h, bigint.h, bigint128.h, bigint256.h, condition.h,
     crexception.h, crlikely.h, crtypes.h, lstring.h, needs.h, semaphore.h
   - All files now have standard copyright headers at file top
   - All `\copy` and `\license` tags removed from Doxygen blocks
   - Consistent modern style across entire include directory

===============================================================================================
# Update header documentation URLs and copyright dates

February 14, 2026 :: 12:42 PM EST (UTC: 17:42 UTC)

Updated all header file documentation to reflect the current GitHub repository
location and extend copyright dates to 2026.

1. **\see tag URL updates** — Replaced all references to
   `http://www.vadura.eu/crutil` with `https://github.com/dvadura/CRUtil` in
   12 header files (ainteger.h, bigint.h, bigint128.h, bigint256.h,
   condition.h, crexception.h, crlikely.h, crtimer.h, crtypes.h, lstring.h,
   needs.h, semaphore.h).

2. **Copyright date updates** — Updated copyright dates to reflect continued
   maintenance through 2026:
   - Files with `2010-2013` updated to `2010-2026` (8 files): ainteger.h,
     condition.h, crexception.h, crlikely.h, crtimer.h, crtypes.h, lstring.h,
     semaphore.h
   - Files with `2016` updated to `2016-2026` (4 files): bigint.h,
     bigint128.h, bigint256.h, needs.h

3. **Excluded files** — endian.h intentionally excluded (third-party public
   domain code). crstring.h and crstring.cpp lack standard headers and were
   not updated.

===============================================================================================
# Fix git-askpass multi-remote credential resolution

February 03, 2026 :: 06:19 PM EST (UTC: 23:19 UTC)

Fixed git-askpass.sh and its inline .gitaskpass.py helper so that repos with
multiple remotes (e.g., origin + github) resolve credentials correctly.

1. **`.gitaskpass.py` multi-remote matching** — The old code passed URL strings
   from `.gituser` keys to `git remote get-url` as if they were remote names,
   which always failed, causing a fallback to the first entry (wrong credentials
   for any remote other than the first). Replaced with direct URL-key and
   hostname-based matching against the prompt URL git provides.

2. **`cmd_enable` (git-askpass.sh)** — Now sets `core.askpass` to an absolute
   path instead of relative `./`, preventing breakage when git spawns child
   processes. Disables `credential.helper` locally to stop macOS Keychain
   (`osxkeychain`) from intercepting the askpass flow. Copies `user.name` and
   `user.email` from global to local config if not already set.

3. **`cmd_add` (git-askpass.sh)** — Now accepts a git remote name (e.g.,
   `github`) in addition to raw URLs. Resolves the name to a URL via
   `git remote get-url`, embeds the username in the URL if missing (so git
   doesn't prompt for it), and stores the credential keyed by the resolved URL.

===============================================================================================
# Add CMake build support alongside bif

February 03, 2026 :: 05:32 PM EST (UTC: 22:32 UTC)

Added a CMakeLists.txt and CMakePresets.json so the project can be built with
CMake in addition to bif. Updated README and Reference docs to document both
build systems and `bif test`.

1. **CMakeLists.txt** — Static library target (`crutil`) with output directed to
   `Build/`. A `package` target stages the library and headers into
   `Artifacts/CRUtil-<version>.tar.gz`. Test suite opt-in via
   `-DCRUTIL_BUILD_TESTS=ON` builds the Catch2 runner and registers it with
   `ctest`.

2. **CMakePresets.json** — Defines `debug` and `release` configure presets with
   `binaryDir` under `Build/cmake-debug` and `Build/cmake-release`. IDEs
   (CLion, VS Code, Visual Studio) pick these up automatically so builds land
   in `Build/` without manual configuration.

3. **README.md / CRUtil-Reference.md** — Added `bif test` as the primary test
   command. Added a CMake subsection under Building with usage examples for
   library, package, and test workflows.

4. **.gitignore** — Added `Artifacts/CRUtil-*.tar.gz` so CMake package output
   does not mix with bif-produced versioned bundles.

===============================================================================================
# Fix compiler warnings in headers and tests

February 02, 2026 :: 05:55 PM EST (UTC: 17:55 UTC)

Fixed all "potentially dangerous" and "code quality" warnings reported by
`-Wall -Wextra -Wpedantic -Wshadow -Wconversion`. All 966 assertions in
164 test cases continue to pass.

1. **crexception.h** — Fixed undefined behavior: replaced the `const string&`
   variadic constructor (UB from `va_start` on a reference) with a non-variadic
   overload that sets fields directly. Removed meaningless `const` on `int`
   return types for `geterrno()` and `line()`. Changed `m_linenumber` from
   `int` to `unsigned int` to match the constructor parameter and eliminate
   sign-conversion warnings. Replaced `sprintf` with `snprintf` in
   `CRX_CAPTURE_CATCH` macro. Fixed format-security in `CRX_REPORT_CATCH`
   (`fprintf(FD, "%s", ...)` instead of `fprintf(FD, str.c_str())`).

2. **condition.h** — Fixed `%lu` format specifiers to `%llu` with explicit
   `(unsigned long long)` casts for `uint64_t` arguments in `CO_DEBUG` calls.

3. **crtimer.h** — Added explicit copy assignment operator to `crts` to
   suppress `-Wdeprecated-copy`. Commented out unused `rem` parameter names
   in `udelay`, `mdelay`, `sdelay`.

4. **semaphore.h** — Commented out unused `cond` parameter names in `cv()`
   and `cp()`. Moved `msg` declaration inside the `_GNU_SOURCE` `#if` block
   in `P()` to avoid unused-variable warning on macOS.

5. **lstring.h** — Added `static_cast<char>` to the XOR result in
   `Obfuscate::operator()` to suppress implicit int-to-char conversion warning.

6. **test_bigint256.cpp** — Removed unused variable `a` in construction test.

7. **test_crexception.cpp** — Changed `long sz` to `size_t sz` with cast
   from `ftell()` to eliminate sign-conversion warnings.

===============================================================================================
# Document CRException macros, Semaphore DEBUG/SEMTRACE, and Condition COND_DEBUG

February 02, 2026 :: 04:42 PM EST (UTC: 21:42 UTC)

Expanded header comments and README to document the debugging features of the
three core concurrency/exception headers.

1. **crexception.h**: Added a "Macro Quick Reference" section to the Doxygen
   header listing all throw, catch, report, stacktrace, and thread-cancellation
   macros with one-line descriptions.

2. **semaphore.h**: Added "DEBUG vs Release Behavior" and "SEMTRACE" sections
   explaining PP/VV call-site capture, use-after-destroy detection, acquisition
   history tracking, and the global trace ring dumped by semtracedump().

3. **condition.h**: Added "COND_DEBUG" section explaining the per-instance debug
   flag, CO_DEBUG traces, and what information they emit (tid, mode, fired/waiter
   counts, timeout).

4. **README.md**: Added three new feature sections (CRException, Semaphore,
   Condition) with usage examples and compile-flag guidance.  Updated test count
   to 164 cases / 966 assertions.  Updated compile command with -D_GNU_SOURCE
   and test_crexception.cpp.

===============================================================================================
# Add CRException test suite

February 02, 2026 :: 04:25 PM EST (UTC: 21:25 UTC)

Added `Test/test_crexception.cpp` with 32 test cases covering the CRException
catch/report/stacktrace macros and class behavior.

1. **Throw macros**: `CRX_THROW`, `CRX_THROW_ERR`, `CRX_TIF`, `CRX_TUNLESS`,
   `CRX_TIF_ERR`, `CRX_TIFNULL` — verified throw/no-throw and errno propagation.

2. **Accessor coverage**: `file()`, `line()`, `what()`, `errmsg()`, `geterrno()`,
   `setStaticOnly()`/`isStaticOnly()`.

3. **Catch macros**: `CRX_CAPTURE_CATCH` string building (including pid/tid fields
   and staticOnly suppression), `CRX_REPORT_CATCH` file output.

4. **Stacktrace macros**: `CRX_STACKTRACE` and `CRX_REPORT_TRACE` with both
   rethrow=true and rethrow=false, plus custom errno forwarding.

5. **Thread cancellation**: `CRX_THROW_CHK` and `CRX_STACKTRACE` behavior with
   `notifyCancel`/`clearCancel` — confirmed that the canceled thread itself still
   throws (suppression applies to other threads via `isThreadCanceled` semantics).

6. **Output operators**: `operator<<` (ostream ref and pointer, including NULL),
   `operator+=`, `toString()`, demangled calltrace verification.

7. **Build command updated**: Added `-D_GNU_SOURCE` flag and `test_crexception.cpp`
   to the compile line in `CLAUDE.md`.

===============================================================================================
# Add symbol demangling to CRException stack traces

February 02, 2026 :: 04:07 PM EST (UTC: 21:07 UTC)

Added automatic C++ symbol demangling to `CRException` stack traces so that
backtraces display human-readable function names instead of raw mangled symbols.

1. **Include `<cxxabi.h>`**: Added inside the existing `_GNU_SOURCE && !ANDROID`
   guard block alongside `<execinfo.h>`.

2. **Private static `demangle()` helper**: Parses a raw backtrace symbol string,
   extracts the mangled name (handles both macOS and Linux formats), calls
   `abi::__cxa_demangle()`, and replaces the mangled name in the output. Falls
   back to the original string if demangling fails.

3. **Updated `operator<<`**: The calltrace loop now passes each symbol through
   `demangle()` before writing to the output stream.

===============================================================================================
# Rename bigint.h → bigint128.h and create bigint256.h

February 02, 2026 :: 2:54 PM EST (UTC: 19:54)

Renamed `bigint.h` to `bigint128.h` and created `bigint256.h` implementing a full
256-bit unsigned integer (`uint256_t`) as `pair<uint128_t, uint128_t>`.

1. **Rename bigint.h → bigint128.h**: Renamed the header, updated include guard to
   `__BIGINT128_INC__`, removed the `uint256_dont_use_t` placeholder typedef.

2. **Backward-compatibility shim**: Created thin `bigint.h` that `#include`s
   `bigint128.h` — existing code continues to work unchanged.

3. **bigint256.h**: Full `pair<uint128_t, uint128_t>` specialization with:
   - Construction from integral, uint128_t, (hi128, lo128), decimal string, copy
   - All arithmetic: +, -, *, /, % with carry/borrow propagation across 128-bit boundary
   - Widening 128×128→256 multiply via 4 cross-products of 64-bit halves
   - Division: fast path (128/128), short path (256/128 base-2^128 long division),
     full path (256/256 binary shift-subtract)
   - Bitwise, shift, comparison, logical operators
   - String output: toString (decimal), toHexString (delegates to 128-bit halves),
     toOctString (shift-and-mask), ostream << with format flags
   - Utility: isZero, isOne, isLow, isPow2, getPow2, popcount, countl_zero, countr_zero
   - Free-standing operators for T op uint256_t
   - std::hash, std::numeric_limits (digits=256, digits10=77)
   - `_u256` user-defined literal
   - No endian-specific code (delegates to endian-safe 128-bit sub-objects)

4. **Test suite**: 22 test cases with 203 assertions in `Test/test_bigint256.cpp`
   covering construction, assignment, conversion, logical, comparison, bitwise,
   shift, addition, subtraction, multiplication, division round-trips, convenience
   functions, bit utilities, string output, ostream formatting, free-standing
   operators, std::hash, std::numeric_limits, and user-defined literal.

5. **Updated test include**: `test_bigint.cpp` now includes `bigint128.h` directly.

6. **Documentation**: Updated README.md component table and examples, added 256-bit
   section to CRUtil-Reference.md.

===============================================================================================
# Big-endian support for bigint.h

February 02, 2026 :: 1:50 PM EST (UTC: 18:50)

Added compile-time index-remapping macros (`W64`/`W32`/`W16`/`W8`) to `Source/include/bigint.h`
so that all union array overlays (`ub32[]`, `ub16[]`, `ub64[]`) are addressed by logical index
(0 = least-significant word) regardless of byte order.

1. **Index macros**: Replaced the LE-only `static_assert` with `W64`/`W32`/`W16`/`W8` macros
   that are identity on little-endian and mirror-flip on big-endian. Added sanity `static_assert`s.

2. **BI_SLO/BI_SHI**: Changed from `BI_UB64[0]`/`BI_UB64[1]` to `BI_UB64[W64(0)]`/`BI_UB64[W64(1)]`
   in the `single` class, fixing ~20 downstream use sites (casts, toString, hash, free-standing ops).

3. **toHexString**: Wrapped `BI_UB32[i]` with `BI_UB32[W32(i)]` in both `pair` and `single`.

4. **toStringDigits**: Replaced pointer-arithmetic walk with indexed access using `W32`/`W16`
   in both `pair` and `single`.

5. **operator\* (pair multiply)**: Wrapped all 7 `BI_UB32[expr]` sites with `W32(expr)`.

6. **rmdiv32 (pair)**: Wrapped all 12 hardcoded `BI_UB32[N]` indices with `W32(N)`.

7. **rmdiv (pair)**: Wrapped all ~20 `BI_UB16[expr]` and `BI_UB32[expr]` sites with `W16`/`W32`.

8. **Tests**: Added 2 new test cases (`Pair endian-safe array overlay`,
   `Intrinsic endian-safe array overlay`) verifying correct index mapping.

Zero behavior change on little-endian — macros compile to identity `(i)`.

===============================================================================================
# Utility, portability, and modern C++ enhancements to bigint.h

February 02, 2026 :: 1:19 PM EST (UTC: 18:19)

Enhanced `Source/include/bigint.h` with utility, portability, and modern C++ features:

1. **Big-endian safety guard**: Added `#include "endian.h"` and a `static_assert` rejecting
   big-endian platforms at compile time (pair multiplication/division index `ub32[]`/`ub16[]`
   assuming little-endian layout). Removed the old runtime warning comment.

2. **noexcept annotations**: Added `noexcept` to all operators and methods that cannot throw
   in both `pair` and `single` classes, plus all free-standing operators. Division/modulus
   operators and string methods intentionally left without `noexcept`.

3. **constexpr support** (C++17): Marked eligible constructors, comparisons, bitwise, shift,
   addition, subtraction, and utility methods as `constexpr`. Used designated initializer
   lists (`m_u{.m_dt=...}`) in both pair and single constructors to make union initialization
   constexpr-valid. Fixed single class accessors (`lo64()`, `hi64()`, `isZero()`, `isOne()`,
   `isLow()`) to use `BI_VAL` directly instead of union-punned `BI_SHI`/`BI_SLO` macros.
   Pair binary operators (`+`, `-`, `<<`, `>>`) that default-construct a local `pair_t` cannot
   be constexpr due to the packed union, so those are left as runtime-only.

4. **Named accessors**: Added `lo64()` and `hi64()` to both pair and single classes.
   Made `operator uint8_t()`, `operator uint16_t()`, `operator uint32_t()` `explicit` to
   prevent accidental narrowing. `operator bool()` and `operator uint64_t()` remain implicit.

5. **Hex/Octal string output + ostream format flags**: Added `toHexString()` and `toOctString()`
   methods to both pair and single. Modified `operator<<(ostream&)` to respect `std::ios::hex`,
   `std::ios::oct`, and `std::ios::showbase` format flags.

6. **Bit utilities**: Added `popcount()`, `countl_zero()`, and `countr_zero()` to both pair
   and single classes using `__builtin_popcountll`/`__builtin_clzll`/`__builtin_ctzll`.

7. **std::hash specialization**: Added `std::hash<uint128p_t>` and `std::hash<uint128_t>`
   using boost-style hash combine. Enables use in `std::unordered_map`/`std::unordered_set`.

8. **std::numeric_limits specialization**: Added for both `uint128p_t` and `uint128_t` with
   `digits=128`, `digits10=38`, `max_digits10=39`.

9. **User-defined literal `_u128`**: Added `operator""_u128` in `crunnable::literals` namespace.

10. **sprintf → snprintf**: Fixed all `sprintf` calls to use `snprintf` with proper buffer
    sizes, and corrected `%lu` format specifiers to `%llu` with explicit casts.

Added 21 new test cases to `Test/test_bigint.cpp` covering named accessors, explicit
conversions, hex/oct output, ostream format flags, bit utilities, std::hash, numeric_limits,
constexpr operations, noexcept verification, and the `_u128` literal — for both pair and
intrinsic types. Updated existing conversion tests to use `static_cast<>` for explicit
narrowing. All 54 test cases (306 assertions) pass.

===============================================================================================
# Implement missing free-standing and unary operators in bigint.h

February 02, 2026 :: 1:45 PM EST (UTC: 18:45)

Added the full suite of missing operators to `Source/include/bigint.h`:

1. **Unary `operator+()` and `operator-()`** as members of both `pair` and `single` classes.
   `operator+` returns a copy (identity), `operator-` returns two's complement negation.

2. **Free-standing pair compound assignments** (`T op= uint128p_t`): Added `|=`, `+=`,
   `-=`, `<<=`, `>>=`, `*=`, `/=`, `%=`. Also fixed existing `&=`/`^=` — all compound
   assignments now take `T&` (not `const T`) so they actually modify the LHS variable.

3. **Free-standing pair binary operators** (`T op uint128p_t`): Added `+`, `-`, `*`, `/`,
   `%`, `&`, `|`, `^`, `<<`, `>>`. Each constructs a temporary `uint128p_t` from T and
   delegates to the member operator.

4. **Free-standing intrinsic compound assignments** (`T op= uint128_t`): All 10 operators
   (`&=`, `^=`, `|=`, `+=`, `-=`, `<<=`, `>>=`, `*=`, `/=`, `%=`).

5. **Free-standing intrinsic binary operators** (`T op uint128_t`): All 10 operators.

6. **Removed `#if 0` reference block** (formerly lines 1761-1786) — all listed operators
   are now implemented.

Added 6 new test cases to `Test/test_bigint.cpp` covering unary operators and free-standing
operators for both pair and intrinsic types. All 33 test cases (217 assertions) pass.

===============================================================================================
# Fix bigint.h bugs identified in code review

February 02, 2026 :: 12:07 PM EST (UTC: 17:21)

Fixed 6 bugs and 1 minor issue in `Source/include/bigint.h`:

1. **pair `operator>(T)` wrong when `BI_HI != 0`** (line 334): A 128-bit value with
   a non-zero high word is always greater than any 64-bit integral. Changed condition
   from `(BI_HI == 0L) && (BI_LO > rhs)` to `(BI_HI != 0) || (BI_LO > rhs)`.
   Also fixes `operator>=` which delegates to `operator>`.

2. **`rmdiv` correction step inverted** (line 879): When the quotient estimate is too
   large by 1, `ty` must be reduced by one multiple of `tr`. Changed `ty += tr` to
   `ty -= tr`.

3. **`rmdiv32` final remainder uses wrong index** (line 734): The remainder was computed
   using `q.BI_UB32[1]` (previous digit) instead of `q.BI_UB32[0]` (just-computed digit).

4. **Free-standing `operator^=` uses `&` instead of `^`** (line 1848): Copy-paste error
   from the `operator&=` above it. Changed to `^`.

5. **Free-standing `operator<`, `operator<=`, `operator!=` for `(T, uint128p_t)`**
   (lines 1816-1828): All returned `false` when `rhs.BI_HI != 0`. Inverted the logic
   to `(rhs.BI_HI != 0) || (...)`.

6. **`single` shift operators UB on large shifts** (lines 1380-1398): After zeroing the
   value for shifts >= type width, execution fell through to perform the shift on the
   zeroed value, which is undefined behavior. Added `return *this` after the zero
   assignment.

7. **Minor: redundant double-assignment in `pair(const string&)` constructor** (line 178):
   `operator=(*this = str)` assigned twice. Simplified to `operator=(str)`.

Added `Test/test_bigint.cpp`: Catch2 test suite converted from the old GTest tests in
`_old_tests/bigint.cpp`, plus 5 new bug-exposing test cases that validate the fixes above.
All 27 test cases (165 assertions) pass.
