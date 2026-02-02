#include "catch2.hpp"
#include "bigint256.h"
#include <sstream>
#include <unordered_map>

using namespace std;
using namespace crutil;

// 2^256 - 1 as decimal string
static const char *MAX256 = "115792089237316195423570985008687907853269984665640564039457584007913129639935";

// =============================================================================
// Construction tests
// =============================================================================

TEST_CASE("256-bit construction operations", "[bigint256][construction]") {
   uint256_t b(42UL);
   uint256_t c(uint128_t(1UL), uint128_t(0UL));  // 1 << 128
   uint256_t d(MAX256);
   uint256_t e(d);
   uint256_t f("0");

   REQUIRE(b == 42UL);
   REQUIRE(c.hi128() == uint128_t(1UL));
   REQUIRE(c.lo128() == uint128_t(0UL));
   REQUIRE(e == d);
   REQUIRE(f.isZero());

   // Copy from pointer
   uint256_t g(&b);
   REQUIRE(g == 42UL);

   // From string object
   string s("100");
   uint256_t h(s);
   REQUIRE(h == 100UL);

   // From char* (non-const)
   char buf[] = "999";
   uint256_t i(buf);
   REQUIRE(i == 999UL);
}


// =============================================================================
// Assignment tests
// =============================================================================

TEST_CASE("256-bit assignment operations", "[bigint256][assignment]") {
   uint256_t a;
   a = 42UL;
   REQUIRE(a == 42UL);

   a = uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL);
   REQUIRE(a.hi128().isZero());
   REQUIRE(a.lo128() == uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL));

   uint256_t b;
   b = MAX256;
   REQUIRE(b == uint256_t(MAX256));

   string s("12345678901234567890");
   uint256_t c;
   c = s;
   REQUIRE(c == uint256_t("12345678901234567890"));
}


// =============================================================================
// Conversion tests
// =============================================================================

TEST_CASE("256-bit conversion operations", "[bigint256][conversion]") {
   uint256_t a(0xDEADBEEFUL);

   REQUIRE((bool)a == true);
   REQUIRE((bool)uint256_t(0UL) == false);
   REQUIRE((uint64_t)a == 0xDEADBEEFUL);
   REQUIRE(static_cast<uint32_t>(a) == 0xDEADBEEFU);
   REQUIRE(static_cast<uint16_t>(a) == 0xBEEF);
   REQUIRE(static_cast<uint8_t>(a) == 0xEF);
   REQUIRE((uint128_t)a == uint128_t(0xDEADBEEFUL));
}


// =============================================================================
// Logical operator tests
// =============================================================================

TEST_CASE("256-bit logical operations", "[bigint256][logical]") {
   uint256_t zero(0UL);
   uint256_t one(1UL);

   REQUIRE(!zero == true);
   REQUIRE(!one == false);
   REQUIRE((one && one) == true);
   REQUIRE((one && zero) == false);
   REQUIRE((zero || one) == true);
   REQUIRE((zero || zero) == false);
}


// =============================================================================
// Comparison tests
// =============================================================================

TEST_CASE("256-bit comparison operations", "[bigint256][comparison]") {
   uint256_t a(100UL);
   uint256_t b(200UL);
   uint256_t c(100UL);

   // pair_t op pair_t
   REQUIRE(a == c);
   REQUIRE(a != b);
   REQUIRE(a < b);
   REQUIRE(b > a);
   REQUIRE(a <= c);
   REQUIRE(a >= c);
   REQUIRE(a <= b);
   REQUIRE(b >= a);

   // pair_t op T
   REQUIRE(a == 100UL);
   REQUIRE(a != 200UL);
   REQUIRE(a < 200UL);
   REQUIRE(a > 50UL);
   REQUIRE(a >= 100UL);
   REQUIRE(a <= 100UL);

   // Cross-128 boundary comparison
   uint256_t hi(uint128_t(1UL), uint128_t(0UL));
   uint256_t lo(uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL));
   REQUIRE(hi > lo);
   REQUIRE(lo < hi);

   // Comparison with uint128_t
   uint128_t v128(42UL);
   uint256_t v256(42UL);
   REQUIRE(v256 == v128);
   REQUIRE(v256 <= v128);
   REQUIRE(v256 >= v128);
}


// =============================================================================
// Bitwise operator tests
// =============================================================================

TEST_CASE("256-bit bitwise operations", "[bigint256][bitwise]") {
   uint256_t a(uint128_t(0xFF00FF00UL), uint128_t(0x00FF00FFUL));
   uint256_t b(uint128_t(0x0F0F0F0FUL), uint128_t(0xF0F0F0F0UL));

   uint256_t c = a & b;
   REQUIRE(c.hi128() == (uint128_t(0xFF00FF00UL) & uint128_t(0x0F0F0F0FUL)));
   REQUIRE(c.lo128() == (uint128_t(0x00FF00FFUL) & uint128_t(0xF0F0F0F0UL)));

   c = a | b;
   REQUIRE(c.hi128() == (uint128_t(0xFF00FF00UL) | uint128_t(0x0F0F0F0FUL)));
   REQUIRE(c.lo128() == (uint128_t(0x00FF00FFUL) | uint128_t(0xF0F0F0F0UL)));

   c = a ^ b;
   REQUIRE(c.hi128() == (uint128_t(0xFF00FF00UL) ^ uint128_t(0x0F0F0F0FUL)));
   REQUIRE(c.lo128() == (uint128_t(0x00FF00FFUL) ^ uint128_t(0xF0F0F0F0UL)));

   c = ~a;
   REQUIRE(c.hi128() == ~uint128_t(0xFF00FF00UL));
   REQUIRE(c.lo128() == ~uint128_t(0x00FF00FFUL));

   // Compound assignment
   c = a;
   c |= b;
   REQUIRE(c == (a | b));

   c = a;
   c &= b;
   REQUIRE(c == (a & b));

   c = a;
   c ^= b;
   REQUIRE(c == (a ^ b));

   // With integral
   uint256_t d(0xFFUL);
   REQUIRE((d & 0x0FUL) == 0x0FUL);
   REQUIRE((d | 0x100UL) == 0x1FFUL);
   REQUIRE((d ^ 0xFFUL) == 0UL);
}


// =============================================================================
// Shift operator tests
// =============================================================================

TEST_CASE("256-bit shift operations", "[bigint256][shift]") {
   uint256_t one(1UL);

   // Basic shifts
   REQUIRE((one << 0) == one);
   REQUIRE((one << 1) == 2UL);
   REQUIRE((one << 64) == uint256_t(uint128_t(1UL, 0UL)));

   // Shift across 128-bit boundary
   uint256_t shifted = one << 128;
   REQUIRE(shifted.hi128() == uint128_t(1UL));
   REQUIRE(shifted.lo128().isZero());

   shifted = one << 255;
   REQUIRE(shifted.hi128() == (uint128_t(1UL) << 127));

   // Shift >= 256 zeroes out
   REQUIRE((one << 256).isZero());
   REQUIRE((one << 300).isZero());

   // Right shift
   uint256_t big(uint128_t(1UL), uint128_t(0UL)); // 1 << 128
   REQUIRE((big >> 128) == 1UL);
   REQUIRE((big >> 64) == uint256_t(uint128_t(0UL, 1UL) << 64));

   // Right shift >= 256 zeroes out
   REQUIRE((big >> 256).isZero());

   // Compound assignment
   uint256_t x(0x100UL);
   x <<= 4;
   REQUIRE(x == 0x1000UL);
   x >>= 4;
   REQUIRE(x == 0x100UL);

   // Shift within hi half
   uint256_t hi_val(uint128_t(0xABCDUL), uint128_t(0UL));
   hi_val >>= 4;
   REQUIRE(hi_val.hi128() == uint128_t(0xABCUL));
}


// =============================================================================
// Addition tests
// =============================================================================

TEST_CASE("256-bit addition operations", "[bigint256][addition]") {
   uint256_t a(100UL);
   uint256_t b(200UL);

   REQUIRE((a + b) == 300UL);
   REQUIRE((a + 50UL) == 150UL);

   // Carry across 128-bit boundary
   uint256_t max128(uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL));
   uint256_t result = max128 + 1UL;
   REQUIRE(result.hi128() == uint128_t(1UL));
   REQUIRE(result.lo128().isZero());

   // Compound assignment
   uint256_t c(10UL);
   c += 5UL;
   REQUIRE(c == 15UL);

   c += uint256_t(10UL);
   REQUIRE(c == 25UL);

   // With uint128_t
   c += uint128_t(5UL);
   REQUIRE(c == 30UL);

   // Increment
   uint256_t d(0UL);
   ++d;
   REQUIRE(d == 1UL);
   d++;
   REQUIRE(d == 2UL);

   // Overflow wraps
   uint256_t maxval(MAX256);
   uint256_t wrapped = maxval + 1UL;
   REQUIRE(wrapped.isZero());
}


// =============================================================================
// Subtraction tests
// =============================================================================

TEST_CASE("256-bit subtraction operations", "[bigint256][subtraction]") {
   uint256_t a(300UL);
   uint256_t b(100UL);

   REQUIRE((a - b) == 200UL);
   REQUIRE((a - 50UL) == 250UL);

   // Borrow across 128-bit boundary
   uint256_t hi_one(uint128_t(1UL), uint128_t(0UL));
   uint256_t result = hi_one - 1UL;
   REQUIRE(result.hi128().isZero());
   REQUIRE(result.lo128() == uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL));

   // Compound assignment
   uint256_t c(100UL);
   c -= 25UL;
   REQUIRE(c == 75UL);
   c -= uint256_t(25UL);
   REQUIRE(c == 50UL);
   c -= uint128_t(10UL);
   REQUIRE(c == 40UL);

   // Decrement
   uint256_t d(2UL);
   --d;
   REQUIRE(d == 1UL);
   d--;
   REQUIRE(d.isZero());

   // Unary
   uint256_t e(100UL);
   REQUIRE((+e) == 100UL);
   uint256_t neg = -uint256_t(1UL);
   REQUIRE(neg == uint256_t(MAX256));  // -1 == 2^256-1 in unsigned
}


// =============================================================================
// Multiplication tests
// =============================================================================

TEST_CASE("256-bit multiplication operations", "[bigint256][multiplication]") {
   uint256_t a(1000UL);
   uint256_t b(2000UL);

   REQUIRE((a * b) == 2000000UL);
   REQUIRE((a * 0UL).isZero());
   REQUIRE((uint256_t(0UL) * b).isZero());
   REQUIRE((uint256_t(1UL) * b) == b);
   REQUIRE((a * 1UL) == a);

   // Cross-boundary multiplication
   uint256_t large(uint128_t(1UL, 0UL));  // 2^64
   uint256_t result = large * large;       // 2^128
   REQUIRE(result.hi128() == uint128_t(1UL));
   REQUIRE(result.lo128().isZero());

   // Larger multiplication
   uint256_t x("1000000000000000000");   // 10^18
   uint256_t y("1000000000000000000");   // 10^18
   uint256_t expected("1000000000000000000000000000000000000"); // 10^36
   REQUIRE((x * y) == expected);

   // Compound assignment
   uint256_t c(100UL);
   c *= 10UL;
   REQUIRE(c == 1000UL);

   c *= uint256_t(5UL);
   REQUIRE(c == 5000UL);

   c *= uint128_t(2UL);
   REQUIRE(c == 10000UL);
}


// =============================================================================
// Division and modulus tests
// =============================================================================

TEST_CASE("256-bit division operations", "[bigint256][division]") {
   uint256_t a(1000UL);
   uint256_t b(7UL);

   uint256_t q = a / b;
   uint256_t r = a % b;
   REQUIRE(q * b + r == a);
   REQUIRE(q == 142UL);
   REQUIRE(r == 6UL);

   // Divide by 1
   REQUIRE(a / 1UL == a);

   // Divide by self
   REQUIRE(a / a == 1UL);
   REQUIRE(a % a == 0UL);

   // Dividend < divisor
   REQUIRE(uint256_t(5UL) / uint256_t(10UL) == 0UL);
   REQUIRE(uint256_t(5UL) % uint256_t(10UL) == 5UL);

   // Divide by zero throws
   REQUIRE_THROWS(a / uint256_t(0UL));
   REQUIRE_THROWS(a % uint256_t(0UL));

   // Cross-boundary division
   uint256_t big("100000000000000000000000000000000000000");
   uint256_t div("1000000000000000000");
   q = big / div;
   r = big % div;
   REQUIRE(q * div + r == big);

   // Large / small — fast path (divisor fits 128 bits, dividend > 128)
   uint256_t large(uint128_t(10UL), uint128_t(0UL)); // 10 * 2^128
   q = large / uint256_t(10UL);
   REQUIRE(q == uint256_t(uint128_t(1UL), uint128_t(0UL)));

   // Compound assignment
   uint256_t c(100UL);
   c /= 10UL;
   REQUIRE(c == 10UL);

   c %= 3UL;
   REQUIRE(c == 1UL);
}


TEST_CASE("256-bit division round-trip", "[bigint256][division]") {
   // Various round-trip checks: q*b + r == a
   struct { const char *a; const char *b; } cases[] = {
      { MAX256, "340282366920938463463374607431768211456" },   // 2^256-1 / 2^128
      { "115792089237316195423570985008687907853269984665640564039457584007913129639935",
        "99999999999999999999999999999999999999" },
      { "57896044618658097711785492504343953926634992332820282019728792003956564819968",
        "7" },
      { "1", "2" },
      { MAX256, MAX256 },
      { "100000000000000000000000000000000000000", "9999999999999999999" },  // 10^38 / (10^19 - 1)
   };

   for (auto& tc : cases) {
      uint256_t a(tc.a);
      uint256_t b(tc.b);
      uint256_t q = a / b;
      uint256_t r = a % b;
      REQUIRE(q * b + r == a);
      REQUIRE(r < b);
   }
}


// =============================================================================
// Convenience function tests
// =============================================================================

TEST_CASE("256-bit convenience functions", "[bigint256][utility]") {
   REQUIRE(uint256_t(0UL).isZero());
   REQUIRE_FALSE(uint256_t(1UL).isZero());

   REQUIRE(uint256_t(1UL).isOne());
   REQUIRE_FALSE(uint256_t(2UL).isOne());

   REQUIRE(uint256_t(100UL).isLow());
   REQUIRE_FALSE(uint256_t(uint128_t(1UL), uint128_t(0UL)).isLow());

   // isPow2 / getPow2
   REQUIRE(uint256_t(1UL).isPow2());
   REQUIRE(uint256_t(1UL).getPow2() == 0);

   REQUIRE((uint256_t(1UL) << 128).isPow2());
   REQUIRE((uint256_t(1UL) << 128).getPow2() == 128);

   REQUIRE((uint256_t(1UL) << 255).isPow2());
   REQUIRE((uint256_t(1UL) << 255).getPow2() == 255);

   REQUIRE_FALSE(uint256_t(3UL).isPow2());
   REQUIRE_THROWS(uint256_t(3UL).getPow2());
}


TEST_CASE("256-bit bit utilities", "[bigint256][bits]") {
   REQUIRE(uint256_t(0UL).popcount() == 0);
   REQUIRE(uint256_t(0xFFUL).popcount() == 8);
   REQUIRE(uint256_t(MAX256).popcount() == 256);

   REQUIRE(uint256_t(0UL).countl_zero() == 256);
   REQUIRE(uint256_t(1UL).countl_zero() == 255);
   REQUIRE(uint256_t(MAX256).countl_zero() == 0);

   REQUIRE(uint256_t(0UL).countr_zero() == 256);
   REQUIRE(uint256_t(1UL).countr_zero() == 0);
   REQUIRE((uint256_t(1UL) << 128).countr_zero() == 128);
}


// =============================================================================
// String output tests
// =============================================================================

TEST_CASE("256-bit toString", "[bigint256][string]") {
   string s;

   uint256_t(0UL).toString(s);
   REQUIRE(s == "0");

   uint256_t(12345UL).toString(s);
   REQUIRE(s == "12345");

   uint256_t(MAX256).toString(s);
   REQUIRE(s == MAX256);

   // Large value round-trip
   const char *val = "100000000000000000000000000000000000000";
   uint256_t a(val);
   a.toString(s);
   REQUIRE(s == val);
}


TEST_CASE("256-bit toHexString", "[bigint256][string]") {
   string s;

   uint256_t(0UL).toHexString(s);
   REQUIRE(s == "0");

   uint256_t(255UL).toHexString(s);
   REQUIRE(s == "ff");

   uint256_t(MAX256).toHexString(s);
   REQUIRE(s == "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

   // Value in hi128 only
   uint256_t hi(uint128_t(0xABCDUL), uint128_t(0UL));
   hi.toHexString(s);
   REQUIRE(s == "abcd00000000000000000000000000000000");
}


TEST_CASE("256-bit toOctString", "[bigint256][string]") {
   string s;

   uint256_t(0UL).toOctString(s);
   REQUIRE(s == "0");

   uint256_t(8UL).toOctString(s);
   REQUIRE(s == "10");

   uint256_t(0777UL).toOctString(s);
   REQUIRE(s == "777");
}


TEST_CASE("256-bit ostream formatting", "[bigint256][string]") {
   ostringstream oss;

   oss << uint256_t(255UL);
   REQUIRE(oss.str() == "255");

   oss.str("");
   oss << hex << uint256_t(255UL);
   REQUIRE(oss.str() == "ff");

   oss.str("");
   oss << hex << showbase << uint256_t(255UL);
   REQUIRE(oss.str() == "0xff");

   oss.str("");
   oss.clear();
   oss << noshowbase << oct << uint256_t(8UL);
   REQUIRE(oss.str() == "10");

   oss.str("");
   oss << oct << showbase << uint256_t(8UL);
   REQUIRE(oss.str() == "010");

   oss.str("");
   oss << oct << showbase << uint256_t(0UL);
   REQUIRE(oss.str() == "0");
}


// =============================================================================
// Free-standing operator tests
// =============================================================================

TEST_CASE("256-bit free-standing operators", "[bigint256][freestanding]") {
   uint256_t a(100UL);

   // T op uint256_t (comparison)
   REQUIRE(100UL == a);
   REQUIRE(200UL != a);
   REQUIRE(200UL > a);
   REQUIRE(50UL < a);
   REQUIRE(100UL >= a);
   REQUIRE(100UL <= a);

   // T op uint256_t (binary)
   REQUIRE(10UL + a == 110UL);
   REQUIRE(200UL - a == 100UL);
   REQUIRE(3UL * a == 300UL);
   REQUIRE(1000UL / a == 10UL);
   REQUIRE(105UL % a == 5UL);

   // Compound assignment: T op= uint256_t
   uint64_t x = 100;
   x += uint256_t(10UL);
   REQUIRE(x == 110);

   x -= uint256_t(10UL);
   REQUIRE(x == 100);

   x *= uint256_t(2UL);
   REQUIRE(x == 200);

   x /= uint256_t(4UL);
   REQUIRE(x == 50);

   x %= uint256_t(7UL);
   REQUIRE(x == 1);
}


// =============================================================================
// std::hash test
// =============================================================================

TEST_CASE("256-bit std::hash", "[bigint256][hash]") {
   uint256_t a(42UL);
   uint256_t b(42UL);
   uint256_t c(43UL);

   hash<uint256_t> hasher;
   REQUIRE(hasher(a) == hasher(b));
   // Different values should (almost certainly) produce different hashes
   REQUIRE(hasher(a) != hasher(c));

   // Can be used in unordered_map
   unordered_map<uint256_t, int> map;
   map[a] = 1;
   map[c] = 2;
   REQUIRE(map[a] == 1);
   REQUIRE(map[c] == 2);
}


// =============================================================================
// std::numeric_limits test
// =============================================================================

TEST_CASE("256-bit std::numeric_limits", "[bigint256][limits]") {
   using lim = numeric_limits<uint256_t>;

   REQUIRE(lim::is_specialized == true);
   REQUIRE(lim::is_signed == false);
   REQUIRE(lim::is_integer == true);
   REQUIRE(lim::digits == 256);
   REQUIRE(lim::digits10 == 77);
   REQUIRE(lim::max_digits10 == 78);
   REQUIRE(lim::radix == 2);

   REQUIRE(lim::min().isZero());
   REQUIRE(lim::lowest().isZero());
   REQUIRE(lim::max() == uint256_t(MAX256));
}


// =============================================================================
// User-defined literal test
// =============================================================================

TEST_CASE("256-bit user-defined literal", "[bigint256][literal]") {
   using namespace crutil::literals;

   auto a = "115792089237316195423570985008687907853269984665640564039457584007913129639935"_u256;
   REQUIRE(a == uint256_t(MAX256));

   auto b = "42"_u256;
   REQUIRE(b == 42UL);
}
