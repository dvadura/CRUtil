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
