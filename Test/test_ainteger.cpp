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
