#include "catch2.hpp"
#include "ainteger.h"

using namespace std;
using namespace crutil;

// --- comparison ---

TEST_CASE("AInteger comparison operations", "[ainteger]") {
   AInteger x;
   AInteger y(10);

   REQUIRE(x == x);
   REQUIRE(x != y);
   REQUIRE(x < y);
   REQUIRE(y > x);

   x = 10;
   REQUIRE(x.value() == 10);

   REQUIRE(x <= y);
   REQUIRE(y >= x);
}

// --- arithmetic ---

TEST_CASE("AInteger arithmetic operations", "[ainteger]") {
   AInteger x(10);

   x += 1;
   REQUIRE(x == 11);
   x -= 6;
   REQUIRE(x == 5);
   --x;
   REQUIRE(x == 4);
   ++x;
   REQUIRE(x == 5);
   REQUIRE(x++ == 5);
   REQUIRE(x-- == 6);
   REQUIRE(x == 5);

   // postfix is applied first
   ++x--;
   REQUIRE(x == 4);

   // postfix is applied first
   --x++;
   REQUIRE(x == 5);
}

// --- CAS ---

TEST_CASE("AInteger CAS operations", "[ainteger]") {
   AInteger x(10);

   x.cas(11, 12);
   REQUIRE(x == 10);
   x.cas(10, 12);
   REQUIRE(x == 12);
}

TEST_CASE("AInteger test_and_set operations", "[ainteger]") {
   AInteger x(10);

   REQUIRE(x.test_and_set(11, 12) == false);  // Expected doesn't match
   REQUIRE(x == 10);                          // Value unchanged
   REQUIRE(x.test_and_set(10, 12) == true);   // Expected matches
   REQUIRE(x == 12);                          // Value updated
}

// --- floor/ceiling ---

TEST_CASE("AInteger floor operations", "[ainteger]") {
   AInteger x(10);

   x.floor(5);
   REQUIRE(x == 10);  // No change, already above floor

   x.floor(15);
   REQUIRE(x == 15);  // Clamped up to floor

   x.floor(12);
   REQUIRE(x == 15);  // No change, already above floor
}

TEST_CASE("AInteger ceiling operations", "[ainteger]") {
   AInteger x(10);

   x.ceiling(15);
   REQUIRE(x == 10);  // No change, already below ceiling

   x.ceiling(5);
   REQUIRE(x == 5);   // Clamped down to ceiling

   x.ceiling(8);
   REQUIRE(x == 5);   // No change, already below ceiling
}

// --- fetch operations ---

TEST_CASE("AInteger pre_add operations", "[ainteger]") {
   AInteger x(10);
   AInteger delta(5);

   int64_t old = x.pre_add(delta);
   REQUIRE(old == 10);   // Returns old value
   REQUIRE(x == 15);     // Value now updated

   old = x.pre_add(delta);
   REQUIRE(old == 15);   // Returns previous value
   REQUIRE(x == 20);     // Value incremented again
}

TEST_CASE("AInteger pre_sub operations", "[ainteger]") {
   AInteger x(20);
   AInteger delta(5);

   int64_t old = x.pre_sub(delta);
   REQUIRE(old == 20);   // Returns old value
   REQUIRE(x == 15);     // Value now updated

   old = x.pre_sub(delta);
   REQUIRE(old == 15);   // Returns previous value
   REQUIRE(x == 10);     // Value decremented again
}

// --- exchange ---

TEST_CASE("AInteger gas (get-and-set) operations", "[ainteger]") {
   AInteger x(10);

   int64_t old = x.gas(20);
   REQUIRE(old == 10);   // Returns old value
   REQUIRE(x == 20);     // Value now updated

   old = x.gas(30);
   REQUIRE(old == 20);   // Returns previous value
   REQUIRE(x == 30);     // Value updated again
}

// --- type conversion ---

TEST_CASE("AInteger uint64 conversion", "[ainteger]") {
   AInteger x(42);
   REQUIRE(x.value() == 42);
   REQUIRE(x.uint64() == 42u);

   x = -5;
   REQUIRE(x.value() == -5);
   // Negative value wraps to large unsigned value
   REQUIRE(x.uint64() == (uint64_t)-5);
   REQUIRE(x.uint64() == 18446744073709551611ULL);
}

// --- set method ---

TEST_CASE("AInteger set method", "[ainteger]") {
   AInteger x(10);

   x.set(20);
   REQUIRE(x == 20);

   x.set(5).set(15);  // Chaining
   REQUIRE(x == 15);
}
