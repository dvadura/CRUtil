#include "catch2.hpp"
#include "crtimer.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace crunnable;

// --- comparison ---

TEST_CASE("CRTime comparison operations", "[crtimer]") {
   CRTime x((uint64_t) 0);
   CRTime y((uint64_t) 0);

   REQUIRE(x == y);
   y = x.now();
   y.usleep(100);
   y.now();
   uint64_t diff = y.usec() - x.usec();

   REQUIRE(y > x);
   REQUIRE(diff >= 100);

   CRTime z1;
   std::this_thread::sleep_for(std::chrono::microseconds(1));
   CRTime z2;

   REQUIRE(z1 != z2);
   REQUIRE(z2 > z1);
   REQUIRE(z1 < z2);
   REQUIRE(z2 >= z1);
   REQUIRE(z1 <= z2);
}

// --- conversion ---

TEST_CASE("CRTime conversion operations", "[crtimer]") {
   CRTime x;
   std::this_thread::sleep_for(std::chrono::microseconds(1));
   CRTime y;
   uint64_t diff = y.nsec() - x.nsec();

   REQUIRE(x != y);
   REQUIRE(diff > 0);

   y -= x;
   REQUIRE(y == diff);
}

// --- delay ---

TEST_CASE("CRTime delay operations", "[crtimer]") {
   CRTime x((uint64_t) 0);
   CRTime r((uint64_t) 0);

   REQUIRE(x.t2ns() == 0L);

   uint64_t start = x.now().msec();
   int delay = x.ndelay(NS_IN_ONE_MSEC * 5, &r);
   uint64_t diff = x.msec() - start;

   REQUIRE(delay == 0);
   REQUIRE(diff > 0);
   REQUIRE(diff < 1000);

   delay = x.ndelay(NS_IN_ONE_MSEC * 10, &r);
   std::this_thread::sleep_for(std::chrono::microseconds(1));
   CRTime y;
   diff = y.nsec() - x.nsec();

   REQUIRE(delay == 0);
   REQUIRE(diff > 0);

   // diff may be large on throttled systems so allow generous margin
   REQUIRE(diff < 50000);

   y.now();
   std::this_thread::sleep_for(std::chrono::microseconds(10000));
   diff = y.diff();
   REQUIRE(diff > 0);
   REQUIRE(diff > 10000000);
   REQUIRE(diff < 12000000);
}
