#include "catch2.hpp"
#include "bigint.h"
#include <sstream>
#include <unordered_map>

using namespace std;
using namespace crunnable;

// =============================================================================
// Pair-based uint128p_t tests
// =============================================================================

// --- construction ---

TEST_CASE("Pair construction operations", "[bigint][pair]") {
   uint128p_t a;
   uint128p_t b("18446744073709551617");
   uint128p_t c(9223372036854775808UL);
   uint128p_t d(1,1);
   uint128p_t e(d);

   REQUIRE_FALSE(a.isZero());
   REQUIRE(b == d);
   REQUIRE(d == e);

   c *= 2;
   ++c;
   REQUIRE(c == b);

   string s;
   REQUIRE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);

   uint128p_t f(s);
   REQUIRE(f == b);
}

// --- assignment ---

TEST_CASE("Pair assignment operations", "[bigint][pair]") {
   uint128p_t b = "18446744073709551617";
   uint128p_t c(9223372036854775808UL);

   uint128p_t d = b;
   uint128p_t e = 1;

   c <<= 1;
   c += e;

   REQUIRE(b == d);
   REQUIRE(b == c);

   string s = "18446744073709551617";
   c = s;
   REQUIRE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);
}

// --- conversion ---

TEST_CASE("Pair conversion operations", "[bigint][pair]") {
   uint128p_t a("18446744073709551617");

   bool v1 = a;
   uint8_t v2 = static_cast<uint8_t>(a);
   uint16_t v3 = static_cast<uint16_t>(a);
   uint32_t v4 = static_cast<uint32_t>(a);
   uint64_t v5 = a;

   REQUIRE(v1 == true);
   REQUIRE(v2 == 1);
   REQUIRE(v3 == 1);
   REQUIRE(v4 == 1);
   REQUIRE(v5 == 1);
}

// --- logical ---

TEST_CASE("Pair logical operations", "[bigint][pair]") {
   uint128p_t a("18446744073709551617");
   uint128p_t b(0);

   REQUIRE(!a == false);
   REQUIRE((a && true) == true);
   REQUIRE(!b == true);
   REQUIRE((a && b) == false);
   REQUIRE((b && a) == false);
   REQUIRE((a || b) == true);
   REQUIRE((b || a) == true);
}

// --- comparison ---

TEST_CASE("Pair comparison operations", "[bigint][pair]") {
   uint128p_t a(1,1);
   uint128p_t b(1,0);
   uint128p_t c(10);

   REQUIRE(a != b);
   REQUIRE(a == b + 1);
   REQUIRE(a - 1 == b);

   REQUIRE(a > b);
   REQUIRE(b < a);

   REQUIRE(a >= b);
   REQUIRE(b <= a);

   REQUIRE(a >= (b + 1));
   REQUIRE(b <= (a - 1));

   REQUIRE(c != 11);
   REQUIRE(c == 10);

   REQUIRE(c > 9);
   REQUIRE(9 < c);

   REQUIRE(c >= 10);
}

// --- boolean / bitwise ---

TEST_CASE("Pair boolean operations", "[bigint][pair]") {
   uint128p_t a(0);
   uint128p_t b(0x10101010UL, 0x10101010UL);
   uint128p_t c(a);

   a |= b;
   REQUIRE(a == b);

   a ^= b;
   REQUIRE(a == c);

   b &= c;
   REQUIRE(b == c);

   b |= 0xf0f0f0f0UL;
   a |= b;
   REQUIRE(a == 0xf0f0f0f0UL);

   a = ~b;
   a |= 0xf0f0f0f0;
   REQUIRE((~a).isZero());
}

// --- bit shift ---

TEST_CASE("Pair bit shift operations", "[bigint][pair]") {
   uint128p_t a(1);
   uint128p_t b(a);
   uint128p_t c(0x8000000000000000UL, 0UL);

   b <<= 127;
   REQUIRE(b == c);
   b <<= 1;
   REQUIRE(b == 0);

   b = a << 63;
   REQUIRE(b.isLow());
   b <<= 1;
   REQUIRE_FALSE(b.isLow());

   a >>= 1;
   REQUIRE(a == 0);

   c >>= 200;
   REQUIRE(c == 0);
}

// --- addition ---

TEST_CASE("Pair addition operations", "[bigint][pair]") {
   uint128p_t a(0);
   uint128p_t b("18446744073709551617");
   uint128p_t c;

   ++a;
   REQUIRE(a == 1);
   c = a++;
   REQUIRE(a == 2);
   REQUIRE(c == 1);

   a += 1;
   REQUIRE(a == 3);
   REQUIRE(a + 1 == 4);
   REQUIRE(a == 3);

   a += 0xfffffffffffffffc;
   REQUIRE(a.isLow());
   ++a;
   REQUIRE_FALSE(a.isLow());
   REQUIRE(a + 1 == b);

   a = "340282366920938463463374607431768211455";
   REQUIRE(a + 1 == 0);
}

// --- subtraction ---

TEST_CASE("Pair subtraction operations", "[bigint][pair]") {
   uint128p_t a(0);
   uint128p_t b("340282366920938463463374607431768211455");
   uint128p_t c(1, 0);

   --a;
   REQUIRE(a == b);
   a = 0;
   a--;
   REQUIRE(a == b);

   REQUIRE_FALSE(c.isLow());
   c -= 1;
   REQUIRE(c.isLow());
   REQUIRE(c == 0xffffffffffffffffUL);
   REQUIRE(c - 0xffffffffffffffffUL == 0);
}

// --- multiplication ---

TEST_CASE("Pair multiplication operations", "[bigint][pair]") {
   uint128p_t a(0);
   uint128p_t b(1);
   uint128p_t c(10);

   a *= 10;
   a *= 1;
   REQUIRE(a == 0);

   b *= 2*2*2*2*2*2*2*2;
   REQUIRE(b == 256);

   b = "10000000000000000000000000000000000000";
   c *= 1000000;
   c *= 1000000;
   c *= 1000000;
   c *= 1000000;
   c *= 1000000;
   c *= 1000000;

   REQUIRE(c == b);
   c *= 10;
   b = "100000000000000000000000000000000000000";
   REQUIRE(c == b);

   c *= 10;
   b = "319435266158123073073250785136463577088";
   REQUIRE(c == b);
}

// --- division ---

TEST_CASE("Pair division operations", "[bigint][pair]") {
   uint128p_t a("340282366920938463463374607431768211455");
   uint128p_t b("10000000000000000000000");
   uint128p_t c;
   string s;

   c = a / b;
   REQUIRE(c == 34028236692093846UL);

   c = a % b;
   c.toString(s);
   REQUIRE(c == "3463374607431768211455");

   c = a / 2 / 2 / 2 / 2;
   c.toString(s);
   REQUIRE(c == "21267647932558653966460912964485513215");

   REQUIRE_THROWS_AS(a / 0, CRException);
}


// =============================================================================
// Intrinsic uint128_t tests (when available)
// =============================================================================

#ifdef INT128_INTRINSIC

// --- construction ---

TEST_CASE("Intrinsic construction operations", "[bigint][intrinsic]") {
   uint128_t a;
   uint128_t b("18446744073709551617");
   uint128_t c(9223372036854775808UL);
   uint128_t d(1,1);
   uint128_t e(d);

   REQUIRE_FALSE(a.isZero());
   REQUIRE(b == d);
   REQUIRE(d == e);

   c <<= 1;
   ++c;
   REQUIRE(c == b);

   string s;
   REQUIRE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);

   uint128_t f(s);
   REQUIRE(f == b);
}

// --- assignment ---

TEST_CASE("Intrinsic assignment operations", "[bigint][intrinsic]") {
   uint128_t b = "18446744073709551617";
   uint128_t c(9223372036854775808UL);

   uint128_t d = b;
   uint128_t e = 1;

   c <<= 1;
   c += e;

   REQUIRE(b == d);
   REQUIRE(b == c);

   string s = "18446744073709551617";
   c = s;
   REQUIRE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);
}

// --- conversion ---

TEST_CASE("Intrinsic conversion operations", "[bigint][intrinsic]") {
   uint128_t a("18446744073709551617");

   bool v1 = a;
   uint8_t v2 = static_cast<uint8_t>(a);
   uint16_t v3 = static_cast<uint16_t>(a);
   uint32_t v4 = static_cast<uint32_t>(a);
   uint64_t v5 = a;

   REQUIRE(v1 == true);
   REQUIRE(v2 == 1);
   REQUIRE(v3 == 1);
   REQUIRE(v4 == 1);
   REQUIRE(v5 == 1);
}

// --- logical ---

TEST_CASE("Intrinsic logical operations", "[bigint][intrinsic]") {
   uint128_t a("18446744073709551617");
   uint128_t b(0);

   REQUIRE(!a == false);
   REQUIRE((a && true) == true);
   REQUIRE(!b == true);
   REQUIRE((a && b) == false);
   REQUIRE((b && a) == false);
   REQUIRE((a || b) == true);
   REQUIRE((b || a) == true);
}

// --- comparison ---

TEST_CASE("Intrinsic comparison operations", "[bigint][intrinsic]") {
   uint128_t a(1,1);
   uint128_t b(1,0);
   uint128_t c(10);

   REQUIRE(a != b);
   REQUIRE(a == b + 1);
   REQUIRE(a - 1 == b);

   REQUIRE(a > b);
   REQUIRE(b < a);

   REQUIRE(a >= b);
   REQUIRE(b <= a);

   REQUIRE(a >= (b + 1));
   REQUIRE(b <= (a - 1));

   REQUIRE(c != 11);
   REQUIRE(c == 10);

   REQUIRE(c > 9);
   REQUIRE(c >= 10);
}

// --- boolean / bitwise ---

TEST_CASE("Intrinsic boolean operations", "[bigint][intrinsic]") {
   uint128_t a(0);
   uint128_t b(0x10101010UL, 0x10101010UL);
   uint128_t c(a);

   a |= b;
   REQUIRE(a == b);

   a ^= b;
   REQUIRE(a == c);

   b &= c;
   REQUIRE(b == c);

   b |= 0xf0f0f0f0UL;
   a |= b;
   REQUIRE(a == 0xf0f0f0f0UL);

   a = ~b;
   a |= 0xf0f0f0f0;
   REQUIRE((~a).isZero());
}

// --- bit shift ---

TEST_CASE("Intrinsic bit shift operations", "[bigint][intrinsic]") {
   uint128_t a(1);
   uint128_t b(a);
   uint128_t c(0x8000000000000000UL, 0UL);

   b <<= 127;
   REQUIRE(b == c);
   b <<= 1;
   REQUIRE(b == 0);

   b = a << 63;
   REQUIRE(b.isLow());
   b <<= 1;
   REQUIRE_FALSE(b.isLow());

   a >>= 1;
   REQUIRE(a == 0);

   c >>= 200;
   REQUIRE(c == 0);
}

// --- addition ---

TEST_CASE("Intrinsic addition operations", "[bigint][intrinsic]") {
   uint128_t a(0);
   uint128_t b("18446744073709551617");
   uint128_t c;

   ++a;
   REQUIRE(a == 1);
   c = a++;
   REQUIRE(a == 2);
   REQUIRE(c == 1);

   a += 1;
   REQUIRE(a == 3);
   REQUIRE(a + 1 == 4);
   REQUIRE(a == 3);

   a += 0xfffffffffffffffc;
   REQUIRE(a.isLow());
   ++a;
   REQUIRE_FALSE(a.isLow());
   REQUIRE(a + 1 == b);

   a = "340282366920938463463374607431768211455";
   REQUIRE(a + 1 == 0);
}

// --- subtraction ---

TEST_CASE("Intrinsic subtraction operations", "[bigint][intrinsic]") {
   uint128_t a(0);
   uint128_t b("340282366920938463463374607431768211455");
   uint128_t c(1, 0);

   --a;
   REQUIRE(a == b);
   a = 0;
   a--;
   REQUIRE(a == b);

   REQUIRE_FALSE(c.isLow());
   c -= 1;
   REQUIRE(c.isLow());
   REQUIRE(c == 0xffffffffffffffffUL);
   REQUIRE(c - 0xffffffffffffffffUL == 0);
}

// --- multiplication ---

TEST_CASE("Intrinsic multiplication operations", "[bigint][intrinsic]") {
   uint128_t a(0);
   uint128_t b(1);
   uint128_t c(10);

   a *= 10;
   a *= 1;
   REQUIRE(a == 0);

   b *= 2*2*2*2*2*2*2*2;
   REQUIRE(b == 256);

   b = "10000000000000000000000000000000000000";
   c *= 1000000;
   c *= 1000000;
   c *= 1000000;

   a = 1000000;
   c *= a;
   c *= a;
   c *= a;

   REQUIRE(c == b);
   c *= 10;
   b = "100000000000000000000000000000000000000";
   REQUIRE(c == b);

   c *= 10;
   b = "319435266158123073073250785136463577088";
   REQUIRE(c == b);
}

// --- division ---

TEST_CASE("Intrinsic division operations", "[bigint][intrinsic]") {
   uint128_t a("340282366920938463463374607431768211455");
   uint128_t b("10000000000000000000000");
   uint128_t c;
   string s;

   c = a / b;
   REQUIRE(c == 34028236692093846UL);

   c = a % b;
   c.toString(s);
   REQUIRE(c == "3463374607431768211455");

   c = a / 2 / 2 / 2 / 2;
   c.toString(s);
   REQUIRE(c == "21267647932558653966460912964485513215");

   REQUIRE_THROWS_AS(a / 0, CRException);
}

#endif /* INT128_INTRINSIC */


// =============================================================================
// Unary and free-standing operator tests
// =============================================================================

TEST_CASE("Pair unary plus and minus", "[bigint][pair][unary]") {
   uint128p_t a(42);
   uint128p_t b(1, 0);  // 2^64

   // Unary plus is identity
   REQUIRE(+a == a);
   REQUIRE(+b == b);

   // Unary minus: -x + x == 0
   uint128p_t neg_a = -a;
   REQUIRE(neg_a + a == 0);

   uint128p_t neg_b = -b;
   REQUIRE(neg_b + b == 0);

   // -0 == 0
   uint128p_t z(0);
   REQUIRE(-z == 0);

   // -1 == max value (two's complement wrap)
   uint128p_t one(1);
   uint128p_t max_val("340282366920938463463374607431768211455");
   REQUIRE(-one == max_val);
}

TEST_CASE("Pair free-standing binary operators (T op uint128p_t)", "[bigint][pair][freestanding]") {
   uint128p_t rhs(10);

   // T + uint128p_t
   REQUIRE(5UL + rhs == 15);

   // T - uint128p_t
   REQUIRE(20UL - rhs == 10);

   // T * uint128p_t
   REQUIRE(3UL * rhs == 30);

   // T / uint128p_t
   REQUIRE(100UL / rhs == 10);

   // T % uint128p_t
   REQUIRE(23UL % rhs == 3);

   // T & uint128p_t
   REQUIRE((0xFFUL & uint128p_t(0x0F)) == 0x0F);

   // T | uint128p_t
   REQUIRE((0xF0UL | uint128p_t(0x0F)) == 0xFF);

   // T ^ uint128p_t
   REQUIRE((0xFFUL ^ uint128p_t(0x0F)) == 0xF0);

   // T << uint128p_t
   REQUIRE((1UL << uint128p_t(4)) == 16);

   // T >> uint128p_t
   REQUIRE((16UL >> uint128p_t(4)) == 1);
}

TEST_CASE("Pair free-standing compound assignments (T op= uint128p_t)", "[bigint][pair][freestanding]") {
   uint64_t v;

   v = 5;    v += uint128p_t(10);   REQUIRE(v == 15);
   v = 20;   v -= uint128p_t(10);   REQUIRE(v == 10);
   v = 3;    v *= uint128p_t(10);   REQUIRE(v == 30);
   v = 100;  v /= uint128p_t(10);   REQUIRE(v == 10);
   v = 23;   v %= uint128p_t(10);   REQUIRE(v == 3);
   v = 0xFF; v &= uint128p_t(0x0F); REQUIRE(v == 0x0F);
   v = 0xF0; v |= uint128p_t(0x0F); REQUIRE(v == 0xFF);
   v = 0xFF; v ^= uint128p_t(0x0F); REQUIRE(v == 0xF0);
   v = 1;    v <<= uint128p_t(4);   REQUIRE(v == 16);
   v = 16;   v >>= uint128p_t(4);   REQUIRE(v == 1);
}

#ifdef INT128_INTRINSIC

TEST_CASE("Intrinsic unary plus and minus", "[bigint][intrinsic][unary]") {
   uint128_t a(42);
   uint128_t b(1, 0);  // 2^64

   // Unary plus is identity
   REQUIRE(+a == a);
   REQUIRE(+b == b);

   // Unary minus: -x + x == 0
   uint128_t neg_a = -a;
   REQUIRE(neg_a + a == 0);

   uint128_t neg_b = -b;
   REQUIRE(neg_b + b == 0);

   // -0 == 0
   uint128_t z(0);
   REQUIRE(-z == 0);

   // -1 == max value (two's complement wrap)
   uint128_t one(1);
   uint128_t max_val("340282366920938463463374607431768211455");
   REQUIRE(-one == max_val);
}

TEST_CASE("Intrinsic free-standing binary operators (T op uint128_t)", "[bigint][intrinsic][freestanding]") {
   uint128_t rhs(10);

   REQUIRE(5UL + rhs == 15);
   REQUIRE(20UL - rhs == 10);
   REQUIRE(3UL * rhs == 30);
   REQUIRE(100UL / rhs == 10);
   REQUIRE(23UL % rhs == 3);
   REQUIRE((0xFFUL & uint128_t(0x0F)) == 0x0F);
   REQUIRE((0xF0UL | uint128_t(0x0F)) == 0xFF);
   REQUIRE((0xFFUL ^ uint128_t(0x0F)) == 0xF0);
   REQUIRE((1UL << uint128_t(4)) == 16);
   REQUIRE((16UL >> uint128_t(4)) == 1);
}

TEST_CASE("Intrinsic free-standing compound assignments (T op= uint128_t)", "[bigint][intrinsic][freestanding]") {
   uint64_t v;

   v = 5;    v += uint128_t(10);   REQUIRE(v == 15);
   v = 20;   v -= uint128_t(10);   REQUIRE(v == 10);
   v = 3;    v *= uint128_t(10);   REQUIRE(v == 30);
   v = 100;  v /= uint128_t(10);   REQUIRE(v == 10);
   v = 23;   v %= uint128_t(10);   REQUIRE(v == 3);
   v = 0xFF; v &= uint128_t(0x0F); REQUIRE(v == 0x0F);
   v = 0xF0; v |= uint128_t(0x0F); REQUIRE(v == 0xFF);
   v = 0xFF; v ^= uint128_t(0x0F); REQUIRE(v == 0xF0);
   v = 1;    v <<= uint128_t(4);   REQUIRE(v == 16);
   v = 16;   v >>= uint128_t(4);   REQUIRE(v == 1);
}

#endif /* INT128_INTRINSIC */


// =============================================================================
// Bug-exposing tests — these target specific bugs found during code review.
// Each test is tagged [bugN] so failures can be correlated to the analysis.
// =============================================================================

// ---------------------------------------------------------------------------
// Bug 1: pair operator>(T) returns false when BI_HI != 0
//        A 128-bit value with a non-zero high word is always greater than
//        any 64-bit integral, but the current code requires BI_HI == 0.
//        Also affects operator>= by delegation.
// ---------------------------------------------------------------------------
TEST_CASE("Bug1: pair operator>(T) with non-zero high word", "[bigint][pair][bug1]") {
   uint128p_t big(1, 0);   // 2^64, definitely > any uint64_t

   REQUIRE(big > 0);
   REQUIRE(big > 1);
   REQUIRE(big > 0xffffffffffffffffUL);

   REQUIRE(big >= 0);
   REQUIRE(big >= 1);
   REQUIRE(big >= 0xffffffffffffffffUL);
}

// ---------------------------------------------------------------------------
// Bug 2: pair rmdiv correction step adds tr instead of subtracting it.
//        Exercise the base-2^16 long division path with a divisor that is
//        large enough to avoid the rmdiv32 and power-of-2 fast paths, and
//        that triggers the q-estimate correction (ty > x).
//
//        We verify via round-trip: (a / b) * b + (a % b) == a
// ---------------------------------------------------------------------------
TEST_CASE("Bug2: pair rmdiv correction step (ty > x)", "[bigint][pair][bug2]") {
   // Large dividend and a divisor that forces the full base-2^16 path
   // and is likely to trigger the correction branch.
   uint128p_t a("340282366920938463463374607431768211455");  // 2^128 - 1
   uint128p_t b("98765432109876543210");

   uint128p_t q = a / b;
   uint128p_t r = a % b;

   // Round-trip check: q * b + r must equal a
   uint128p_t reconstructed = q * b + r;
   REQUIRE(reconstructed == a);
   REQUIRE(r < b);

   // A second case with a different pattern
   uint128p_t c("270000000000000000000000000000000000000");
   uint128p_t d("99999999999999999999");

   q = c / d;
   r = c % d;
   reconstructed = q * d + r;
   REQUIRE(reconstructed == c);
   REQUIRE(r < d);
}

// ---------------------------------------------------------------------------
// Bug 3: pair rmdiv32 final remainder uses q.BI_UB32[1] instead of [0].
//        Exercise the rmdiv32 path: divisor fits in 32 bits and dividend
//        uses the high word.  Verify remainder via round-trip.
// ---------------------------------------------------------------------------
TEST_CASE("Bug3: pair rmdiv32 remainder uses wrong index", "[bigint][pair][bug3]") {
   uint128p_t a("100000000000000000000");   // > 2^64, forces rmdiv32 path
   uint128p_t b(7);                         // small 32-bit divisor

   uint128p_t q = a / b;
   uint128p_t r = a % b;

   uint128p_t reconstructed = q * b + r;
   REQUIRE(reconstructed == a);
   REQUIRE(r < b);

   // Another case: max value divided by a 32-bit prime
   uint128p_t c("340282366920938463463374607431768211455");
   uint128p_t d(65521);   // largest 16-bit prime, fits in 32 bits

   q = c / d;
   r = c % d;
   reconstructed = q * d + r;
   REQUIRE(reconstructed == c);
   REQUIRE(r < d);
}

// ---------------------------------------------------------------------------
// Bug 4: free-standing operator^=(T, uint128p_t) uses & instead of ^.
// ---------------------------------------------------------------------------
TEST_CASE("Bug4: free-standing operator^= uses & instead of ^", "[bigint][pair][bug4]") {
   uint128p_t rhs(0xFF);
   uint64_t val = 0xAA;

   // 0xAA ^ 0xFF should be 0x55
   uint64_t result = (val ^= rhs);
   REQUIRE(result == 0x55);
}

// ---------------------------------------------------------------------------
// Bug 5: free-standing operator<(T, uint128p_t) returns false when BI_HI != 0
//        If the uint128p_t has a non-zero high word it is larger than any
//        integral T, so T < uint128p_t should be true.
//        Same bug in operator<= and operator!=.
// ---------------------------------------------------------------------------
TEST_CASE("Bug5: free-standing operator<(T, uint128p_t) with high word", "[bigint][pair][bug5]") {
   uint128p_t big(1, 0);   // 2^64

   REQUIRE(0 < big);
   REQUIRE(1 < big);
   REQUIRE(0xffffffffffffffffUL < big);

   REQUIRE(0UL <= big);
   REQUIRE(1UL <= big);
   REQUIRE(0xffffffffffffffffUL <= big);

   REQUIRE(0UL != big);
   REQUIRE(1UL != big);
   REQUIRE(0xffffffffffffffffUL != big);
}


// =============================================================================
// New feature tests — utility, portability, modern C++
// =============================================================================

// --- named accessors ---

TEST_CASE("Pair named accessors lo64/hi64", "[bigint][pair][accessor]") {
   uint128p_t a(0x42, 0xDEADBEEF);

   REQUIRE(a.hi64() == 0x42);
   REQUIRE(a.lo64() == 0xDEADBEEF);

   uint128p_t z(0);
   REQUIRE(z.hi64() == 0);
   REQUIRE(z.lo64() == 0);
}

// --- explicit narrowing conversions ---

TEST_CASE("Pair explicit narrowing conversions", "[bigint][pair][conversion]") {
   uint128p_t a(0x12345678AABBCCDDULL);

   REQUIRE(static_cast<uint8_t>(a)  == 0xDD);
   REQUIRE(static_cast<uint16_t>(a) == 0xCCDD);
   REQUIRE(static_cast<uint32_t>(a) == 0xAABBCCDD);

   // bool and uint64_t remain implicit
   bool b = a;
   uint64_t v = a;
   REQUIRE(b == true);
   REQUIRE(v == 0x12345678AABBCCDDULL);
}

// --- hex string output ---

TEST_CASE("Pair toHexString", "[bigint][pair][hex]") {
   string s;

   uint128p_t z(0);
   z.toHexString(s);
   REQUIRE(s == "0");

   uint128p_t small(255);
   small.toHexString(s);
   REQUIRE(s == "ff");

   uint128p_t big(0xABCD, 0x1234567890ABCDEFULL);
   big.toHexString(s);
   REQUIRE(s == "abcd1234567890abcdef");

   uint128p_t max_val("340282366920938463463374607431768211455");
   max_val.toHexString(s);
   REQUIRE(s == "ffffffffffffffffffffffffffffffff");
}

// --- oct string output ---

TEST_CASE("Pair toOctString", "[bigint][pair][oct]") {
   string s;

   uint128p_t z(0);
   z.toOctString(s);
   REQUIRE(s == "0");

   uint128p_t small(8);
   small.toOctString(s);
   REQUIRE(s == "10");

   uint128p_t v(0777);
   v.toOctString(s);
   REQUIRE(s == "777");
}

// --- ostream with format flags ---

TEST_CASE("Pair ostream format flags", "[bigint][pair][ostream]") {
   uint128p_t val(255);
   ostringstream oss;

   oss << val;
   REQUIRE(oss.str() == "255");

   oss.str("");
   oss << hex << val;
   REQUIRE(oss.str() == "ff");

   oss.str("");
   oss << oct << val;
   REQUIRE(oss.str() == "377");

   oss.str("");
   oss << hex << showbase << val;
   REQUIRE(oss.str() == "0xff");

   oss.str("");
   oss << oct << showbase << val;
   REQUIRE(oss.str() == "0377");
}

// --- bit utilities ---

TEST_CASE("Pair bit utilities", "[bigint][pair][bitutil]") {
   uint128p_t z(0);
   REQUIRE(z.popcount() == 0);
   REQUIRE(z.countl_zero() == 128);
   REQUIRE(z.countr_zero() == 128);

   uint128p_t one(1);
   REQUIRE(one.popcount() == 1);
   REQUIRE(one.countl_zero() == 127);
   REQUIRE(one.countr_zero() == 0);

   uint128p_t hi_bit(1, 0);  // bit 64 set
   REQUIRE(hi_bit.popcount() == 1);
   REQUIRE(hi_bit.countl_zero() == 63);
   REQUIRE(hi_bit.countr_zero() == 64);

   uint128p_t max_val("340282366920938463463374607431768211455");
   REQUIRE(max_val.popcount() == 128);
   REQUIRE(max_val.countl_zero() == 0);
   REQUIRE(max_val.countr_zero() == 0);

   // Value with specific pattern: 0x80 in low word
   uint128p_t v(0x80);
   REQUIRE(v.popcount() == 1);
   REQUIRE(v.countr_zero() == 7);
}

// --- std::hash ---

TEST_CASE("Pair std::hash", "[bigint][pair][hash]") {
   hash<uint128p_t> hasher;

   uint128p_t a(42);
   uint128p_t b(42);
   uint128p_t c(43);

   // Equal values must produce equal hashes
   REQUIRE(hasher(a) == hasher(b));

   // Different values should (very likely) produce different hashes
   REQUIRE(hasher(a) != hasher(c));

   // Can be used in unordered_map
   unordered_map<uint128p_t, int> m;
   m[a] = 1;
   m[c] = 2;
   REQUIRE(m[a] == 1);
   REQUIRE(m[c] == 2);
}

// --- std::numeric_limits ---

TEST_CASE("Pair std::numeric_limits", "[bigint][pair][limits]") {
   using lim = numeric_limits<uint128p_t>;

   REQUIRE(lim::is_specialized == true);
   REQUIRE(lim::is_integer == true);
   REQUIRE(lim::is_signed == false);
   REQUIRE(lim::is_exact == true);
   REQUIRE(lim::is_bounded == true);
   REQUIRE(lim::is_modulo == true);
   REQUIRE(lim::digits == 128);
   REQUIRE(lim::digits10 == 38);
   REQUIRE(lim::max_digits10 == 39);

   REQUIRE(lim::min() == 0);

   string s;
   lim::max().toString(s);
   REQUIRE(s == "340282366920938463463374607431768211455");

   REQUIRE(lim::lowest() == 0);
}

// --- constexpr ---

TEST_CASE("Pair constexpr operations", "[bigint][pair][constexpr]") {
   constexpr uint128p_t a(10UL);
   constexpr uint128p_t b(3UL);

   // constexpr comparisons
   static_assert(a > b, "constexpr compare");
   static_assert(a != b, "constexpr not-equal");
   static_assert(a == uint128p_t(10UL), "constexpr equal");

   // constexpr bitwise
   constexpr uint128p_t c = a & b;
   static_assert(c == uint128p_t(2UL), "constexpr bitwise and");

   constexpr uint128p_t d = a | b;
   static_assert(d == uint128p_t(11UL), "constexpr bitwise or");

   constexpr uint128p_t e = a ^ b;
   static_assert(e == uint128p_t(9UL), "constexpr xor");

   // constexpr accessors
   static_assert(a.lo64() == 10, "constexpr lo64");
   static_assert(a.hi64() == 0, "constexpr hi64");
   static_assert(a.isZero() == false, "constexpr isZero");

   constexpr uint128p_t z(0UL);
   static_assert(z.isZero() == true, "constexpr isZero true");
}

// --- noexcept ---

TEST_CASE("Pair noexcept verification", "[bigint][pair][noexcept]") {
   uint128p_t a(10);
   uint128p_t b(3);

   // Arithmetic (non-div) should be noexcept
   static_assert(noexcept(a & b), "bitwise and noexcept");
   static_assert(noexcept(a | b), "bitwise or noexcept");
   static_assert(noexcept(a ^ b), "bitwise xor noexcept");

   // Division should NOT be noexcept (throws on zero)
   static_assert(!noexcept(a / b), "division not noexcept");
   static_assert(!noexcept(a % b), "modulus not noexcept");
}

// --- _u128 literal ---

TEST_CASE("User-defined literal _u128", "[bigint][literal]") {
   using namespace crunnable::literals;

   auto a = "18446744073709551617"_u128;
   uint128_t b("18446744073709551617");
   REQUIRE(a == b);

   auto z = "0"_u128;
   REQUIRE(z == 0);

   auto max_val = "340282366920938463463374607431768211455"_u128;
   string s;
   max_val.toString(s);
   REQUIRE(s == "340282366920938463463374607431768211455");
}

// =============================================================================
// Intrinsic versions of new feature tests
// =============================================================================

#ifdef INT128_INTRINSIC

TEST_CASE("Intrinsic named accessors lo64/hi64", "[bigint][intrinsic][accessor]") {
   uint128_t a(0x42, 0xDEADBEEF);

   REQUIRE(a.hi64() == 0x42);
   REQUIRE(a.lo64() == 0xDEADBEEF);
}

TEST_CASE("Intrinsic explicit narrowing conversions", "[bigint][intrinsic][conversion]") {
   uint128_t a(0x12345678AABBCCDDULL);

   REQUIRE(static_cast<uint8_t>(a)  == 0xDD);
   REQUIRE(static_cast<uint16_t>(a) == 0xCCDD);
   REQUIRE(static_cast<uint32_t>(a) == 0xAABBCCDD);

   bool b = a;
   uint64_t v = a;
   REQUIRE(b == true);
   REQUIRE(v == 0x12345678AABBCCDDULL);
}

TEST_CASE("Intrinsic toHexString", "[bigint][intrinsic][hex]") {
   string s;

   uint128_t z(0);
   z.toHexString(s);
   REQUIRE(s == "0");

   uint128_t small(255);
   small.toHexString(s);
   REQUIRE(s == "ff");

   uint128_t max_val("340282366920938463463374607431768211455");
   max_val.toHexString(s);
   REQUIRE(s == "ffffffffffffffffffffffffffffffff");
}

TEST_CASE("Intrinsic toOctString", "[bigint][intrinsic][oct]") {
   string s;

   uint128_t z(0);
   z.toOctString(s);
   REQUIRE(s == "0");

   uint128_t small(8);
   small.toOctString(s);
   REQUIRE(s == "10");
}

TEST_CASE("Intrinsic ostream format flags", "[bigint][intrinsic][ostream]") {
   uint128_t val(255);
   ostringstream oss;

   oss << val;
   REQUIRE(oss.str() == "255");

   oss.str("");
   oss << hex << val;
   REQUIRE(oss.str() == "ff");

   oss.str("");
   oss << oct << val;
   REQUIRE(oss.str() == "377");
}

TEST_CASE("Intrinsic bit utilities", "[bigint][intrinsic][bitutil]") {
   uint128_t z(0);
   REQUIRE(z.popcount() == 0);
   REQUIRE(z.countl_zero() == 128);
   REQUIRE(z.countr_zero() == 128);

   uint128_t one(1);
   REQUIRE(one.popcount() == 1);
   REQUIRE(one.countl_zero() == 127);
   REQUIRE(one.countr_zero() == 0);

   uint128_t hi_bit(1, 0);
   REQUIRE(hi_bit.popcount() == 1);
   REQUIRE(hi_bit.countl_zero() == 63);
   REQUIRE(hi_bit.countr_zero() == 64);
}

TEST_CASE("Intrinsic std::hash", "[bigint][intrinsic][hash]") {
   hash<uint128_t> hasher;

   uint128_t a(42);
   uint128_t b(42);
   uint128_t c(43);

   REQUIRE(hasher(a) == hasher(b));
   REQUIRE(hasher(a) != hasher(c));

   unordered_map<uint128_t, int> m;
   m[a] = 1;
   m[c] = 2;
   REQUIRE(m[a] == 1);
   REQUIRE(m[c] == 2);
}

TEST_CASE("Intrinsic std::numeric_limits", "[bigint][intrinsic][limits]") {
   using lim = numeric_limits<uint128_t>;

   REQUIRE(lim::is_specialized == true);
   REQUIRE(lim::is_integer == true);
   REQUIRE(lim::is_signed == false);
   REQUIRE(lim::digits == 128);
   REQUIRE(lim::digits10 == 38);

   REQUIRE(lim::min() == 0);

   string s;
   lim::max().toString(s);
   REQUIRE(s == "340282366920938463463374607431768211455");
}

TEST_CASE("Intrinsic constexpr operations", "[bigint][intrinsic][constexpr]") {
   constexpr uint128_t a(10UL);
   constexpr uint128_t b(3UL);

   static_assert(a > b, "constexpr compare");
   static_assert(a != b, "constexpr not-equal");

   constexpr uint128_t c = a & b;
   static_assert(c == uint128_t(2UL), "constexpr bitwise and");

   constexpr uint128_t d = a | b;
   static_assert(d == uint128_t(11UL), "constexpr bitwise or");

   static_assert(a.lo64() == 10, "constexpr lo64");
   static_assert(a.hi64() == 0, "constexpr hi64");
   static_assert(a.isZero() == false, "constexpr isZero");
}

TEST_CASE("Intrinsic noexcept verification", "[bigint][intrinsic][noexcept]") {
   uint128_t a(10);
   uint128_t b(3);

   static_assert(noexcept(a & b), "bitwise and noexcept");
   static_assert(noexcept(a | b), "bitwise or noexcept");
   static_assert(!noexcept(a / b), "division not noexcept");
   static_assert(!noexcept(a % b), "modulus not noexcept");
}

#endif /* INT128_INTRINSIC */


// =============================================================================
// Endian-safe array overlay tests
// =============================================================================

TEST_CASE("Pair endian-safe array overlay", "[bigint][pair][endian]") {
   // hi=0x0000000400000003, lo=0x0000000200000001
   uint128p_t v(0x0000000400000003ULL, 0x0000000200000001ULL);

   REQUIRE(v.m_u.ub32[W32(0)] == 1);
   REQUIRE(v.m_u.ub32[W32(1)] == 2);
   REQUIRE(v.m_u.ub32[W32(2)] == 3);
   REQUIRE(v.m_u.ub32[W32(3)] == 4);

   REQUIRE(v.m_u.ub64[W64(0)] == 0x0000000200000001ULL);
   REQUIRE(v.m_u.ub64[W64(1)] == 0x0000000400000003ULL);

   REQUIRE(v.m_u.ub16[W16(0)] == 1);
   REQUIRE(v.m_u.ub16[W16(4)] == 3);
}

#ifdef INT128_INTRINSIC
TEST_CASE("Intrinsic endian-safe array overlay", "[bigint][intrinsic][endian]") {
   uint128_t v(0x0000000400000003ULL, 0x0000000200000001ULL);

   REQUIRE(v.m_u.ub32[W32(0)] == 1);
   REQUIRE(v.m_u.ub32[W32(1)] == 2);
   REQUIRE(v.m_u.ub32[W32(2)] == 3);
   REQUIRE(v.m_u.ub32[W32(3)] == 4);

   // BI_SLO / BI_SHI should map correctly
   REQUIRE(v.BI_SLO == 0x0000000200000001ULL);
   REQUIRE(v.BI_SHI == 0x0000000400000003ULL);
}
#endif
