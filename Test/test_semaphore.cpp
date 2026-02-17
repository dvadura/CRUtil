#include "catch2.hpp"
#include "semaphore.h"

using namespace std;
using namespace crutil;

// --- non-recursive semaphore P() behavior ---

#ifdef DEBUG
// This test only works in DEBUG mode because non-DEBUG builds don't use trylock
// before lock, so double P() will deadlock instead of throwing
TEST_CASE("Semaphore non-recursive double P() throws", "[semaphore]") {
   Semaphore sem(false);  // non-recursive

   REQUIRE_NOTHROW(sem.PP);

   // Second P() on non-recursive semaphore should throw
   REQUIRE_THROWS_AS(sem.PP, CRException);

   REQUIRE_NOTHROW(sem.VV);
}
#endif

// --- semaphore V() without matching P() ---

TEST_CASE("Semaphore V() without matching P() throws", "[semaphore]") {
   Semaphore sem(true);  // recursive

   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.VV);

   // V() without matching P() should throw
   REQUIRE_THROWS_AS(sem.VV, CRException);
}

// --- recursive semaphore over-release ---

TEST_CASE("Semaphore recursive over-release throws", "[semaphore]") {
   Semaphore sem(true);  // recursive

   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.VV);
   REQUIRE_NOTHROW(sem.VV);

   // Third V() without matching P() should throw
   REQUIRE_THROWS_AS(sem.VV, CRException);
}

// --- verbose recursive semaphore error dump ---

TEST_CASE("Semaphore recursive verbose error dump", "[semaphore]") {
   Semaphore sem(true, false, "RSEM");  // recursive, non-adaptive, verbose tag

   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.VV);
   REQUIRE_NOTHROW(sem.VV);

   // This should cause a dump of the error with call stack
   fprintf(stderr, "\n===================> Expect a FAIL for V with call stack dump here\n");
   REQUIRE_THROWS_AS(sem.VV, CRException);
}

// --- verbose non-recursive semaphore error dump ---

#ifdef DEBUG
// This test requires DEBUG mode to detect double P() via trylock
TEST_CASE("Semaphore non-recursive verbose error dump", "[semaphore]") {
   Semaphore sem(false, false, "NSEM");  // non-recursive, non-adaptive, verbose tag

   sem.PP;

   // Double P() should throw and dump error
   fprintf(stderr, "\n===================> Expect a FAIL for P with call stack dump here\n");
   REQUIRE_THROWS_AS(sem.PP, CRException);

   sem.VV;

   // V() without matching P() should throw and dump error
   fprintf(stderr, "\n===================> Expect a FAIL for V with call stack dump here\n");
   REQUIRE_THROWS_AS(sem.VV, CRException);

#ifdef SEMTRACE
   SEMTRACEDUMP(stderr);
#endif
}
#endif

// --- basic lifecycle ---

TEST_CASE("Semaphore default create and destroy", "[semaphore]") {
   REQUIRE_NOTHROW([]{ Semaphore s; }());
}

TEST_CASE("Semaphore non-recursive create and destroy", "[semaphore]") {
   REQUIRE_NOTHROW([]{ Semaphore s(false); }());
}

TEST_CASE("Semaphore recursive create and destroy", "[semaphore]") {
   REQUIRE_NOTHROW([]{ Semaphore s(true); }());
}

TEST_CASE("Semaphore verbose create and destroy", "[semaphore]") {
   REQUIRE_NOTHROW([]{ Semaphore s(true, false, "TEST"); }());
}

// --- basic P/V operations ---

TEST_CASE("Semaphore basic P and V", "[semaphore]") {
   Semaphore sem(false);

   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.VV);
}

TEST_CASE("Semaphore recursive multiple P and V", "[semaphore]") {
   Semaphore sem(true);

   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.PP);
   REQUIRE_NOTHROW(sem.VV);
   REQUIRE_NOTHROW(sem.VV);
   REQUIRE_NOTHROW(sem.VV);
}

// --- verbose tag operations ---

TEST_CASE("Semaphore verbose tag get/set", "[semaphore]") {
   // Set global VERBTAG for this test
   Semaphore::VERBTAG = "SEM";

   Semaphore sem(false, false, "INITIAL");

   REQUIRE(std::string(sem.getVerbose()) == "INITIAL");

   const char* old = sem.setVerbose("UPDATED");
   REQUIRE(std::string(old) == "INITIAL");
   REQUIRE(std::string(sem.getVerbose()) == "UPDATED");

   sem.setVerbose();  // Reset to default
   REQUIRE(std::string(sem.getVerbose()) == "SEM");

   // Restore to nullptr for other tests
   Semaphore::VERBTAG = nullptr;
}
