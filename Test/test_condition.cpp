// Condition tests using std::thread instead of CRunnable to avoid
// dependencies on crunnable.h, clist.h, and socket.h.

#include "catch2.hpp"
#include "condition.h"

#include <thread>
#include <atomic>
#include <chrono>

using namespace std;
using namespace crunnable;

// Static definitions needed by Condition, Semaphore, and CRException.
// These normally live in their respective .cpp files which aren't in the
// library yet, so we provide them here for the test binary.
const char*          Semaphore::VERBTAG     = "SEM";
std::map<pid_t,bool> CRException::s_tmap;
pthread_mutex_t      CRException::s_tlock   = PTHREAD_MUTEX_INITIALIZER;
pthread_key_t        Condition::CONDKEY;
bool                 Condition::CONDKEY_INIT = false;
Semaphore            Condition::SEMCONDKEY;

// --- basic lifecycle ---

TEST_CASE("Condition default create and destroy", "[condition]") {
   REQUIRE_NOTHROW([]{ Condition foo; }());
}

TEST_CASE("Condition named create and destroy", "[condition]") {
   REQUIRE_NOTHROW([]{ Condition foo("NAMED", false); }());
}

TEST_CASE("Condition broadcast create and destroy", "[condition]") {
   REQUIRE_NOTHROW([]{ Condition foo("BCAST", true); }());
}

// --- state queries ---

TEST_CASE("Condition is enabled after construction", "[condition]") {
   Condition c;
   REQUIRE(c.isEnabled() == true);
   REQUIRE(c.isDisabled() == false);
}

TEST_CASE("Condition has no waiters after construction", "[condition]") {
   Condition c;
   REQUIRE(c.hasWaiters() == false);
   REQUIRE(c.waiters() == 0);
}

TEST_CASE("Condition broadcast flag", "[condition]") {
   Condition c(false);
   REQUIRE(c.isBroadcast() == false);

   c.setBroadcast();
   REQUIRE(c.isBroadcast() == true);
}

// --- reset ---

TEST_CASE("Condition reset re-enables", "[condition]") {
   Condition c;
   c.reset();
   REQUIRE(c.isEnabled() == true);
}

// --- raise without waiters ---

TEST_CASE("Condition raise with no waiters does not throw", "[condition]") {
   Condition c;
   REQUIRE_NOTHROW(c.raise());
}

// --- timed wait expires ---

TEST_CASE("Condition timed waitFor returns ETIMEDOUT", "[condition]") {
   Condition c;
   int result = c.waitFor(NS_IN_ONE_MSEC);
   REQUIRE(result == ETIMEDOUT);
}

// --- threaded: raise before wait (signal buffered) ---

TEST_CASE("Condition signal — raise fires before waiter arrives", "[condition][threaded]") {
   Condition c("CN1", false);
   atomic<bool> started{false};
   atomic<int>  thread_result{-1};

   // worker thread: raise then loop waitFor
   thread worker([&]{
      started = true;
      c.raise();
      CRTime now((uint64_t) 0L);
      now.msleep(10);

      try {
         // wait twice to consume two subsequent raises
         c.waitFor(NS_IN_ONE_SEC);
         c.waitFor(NS_IN_ONE_SEC);
      } catch (...) {}
      thread_result = 0;
   });

   // main: wait for the worker's initial raise
   while (!started) std::this_thread::sleep_for(std::chrono::microseconds(100));
   std::this_thread::sleep_for(std::chrono::microseconds(1000));
   int result = c.waitFor(NS_IN_ONE_SEC);
   REQUIRE(result == 0);

   // send two raises so the worker's waitFors complete
   c.raise();
   c.raise();

   worker.join();
   REQUIRE(thread_result == 0);
}

// --- threaded: wait before raise ---

TEST_CASE("Condition signal — waiter blocks until raise", "[condition][threaded]") {
   Condition c("CN2", false);
   atomic<int> result{-1};

   thread waiter([&]{
      result = c.waitFor(NS_IN_ONE_SEC);
   });

   // give waiter time to block
   std::this_thread::sleep_for(std::chrono::microseconds(5000));
   REQUIRE(c.hasWaiters() == true);
   c.raise();

   waiter.join();
   REQUIRE(result == 0);
}

// --- threaded: timed wait expires ---

TEST_CASE("Condition signal — timed wait expires without raise", "[condition][threaded]") {
   Condition c("CN4", false);
   atomic<int> result{-1};

   thread waiter([&]{
      result = c.waitFor(NS_IN_ONE_MSEC * 5);
   });

   waiter.join();
   REQUIRE(result == ETIMEDOUT);
}

// --- threaded: broadcast wakes all waiters ---

TEST_CASE("Condition broadcast — raise wakes all waiters", "[condition][threaded]") {
   Condition c("CB1", true);
   atomic<int> woke{0};

   auto wait_fn = [&]{
      int r = c.waitFor(NS_IN_ONE_SEC);
      if (r == 0) ++woke;
   };

   thread w1(wait_fn);
   thread w2(wait_fn);

   // let both threads block
   std::this_thread::sleep_for(std::chrono::microseconds(10000));
   REQUIRE(c.waiters() == 2);

   c.raise();
   w1.join();
   w2.join();

   REQUIRE(woke == 2);
}
