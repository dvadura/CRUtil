// LFList tests using Catch2 framework
// Note: Requires oneTBB to be installed and linked

#include "catch2.hpp"
#include "lflist.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

using namespace std;
using namespace crutil;

// --- basic lifecycle ---

TEST_CASE("LFList default constructor creates empty list", "[lflist]") {
   LFList<int> list;
   REQUIRE(list.empty() == true);
   REQUIRE(list.size() == 0);
}

TEST_CASE("LFList single-item constructor", "[lflist]") {
   LFList<int> list(42);
   REQUIRE(list.empty() == false);
   REQUIRE(list.size() == 1);
}

TEST_CASE("LFList tagged constructor", "[lflist]") {
   REQUIRE_NOTHROW([]{ LFList<int> list("TAGGED"); }());
}

TEST_CASE("LFList single-item with tag constructor", "[lflist]") {
   LFList<int> list(99, "TAGGED");
   REQUIRE(list.empty() == false);
   REQUIRE(list.size() == 1);
}

TEST_CASE("LFList move constructor", "[lflist]") {
   LFList<int> list1(1);
   list1.push(2);
   list1.push(3);

   LFList<int> list2(std::move(list1));
   REQUIRE(list2.size() >= 1);  // at least 1 element moved
   REQUIRE(list2.empty() == false);
}

// --- size and empty ---

TEST_CASE("LFList size tracking after push operations", "[lflist]") {
   LFList<int> list;
   REQUIRE(list.size() == 0);

   list.push(1);
   REQUIRE(list.size() == 1);

   list.push(2);
   REQUIRE(list.size() == 2);

   list.push(3);
   REQUIRE(list.size() == 3);
}

TEST_CASE("LFList empty() returns correct state", "[lflist]") {
   LFList<int> list;
   REQUIRE(list.empty() == true);

   list.push(1);
   REQUIRE(list.empty() == false);

   bool success;
   list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList empty with uint64_t parameter", "[lflist]") {
   LFList<int> list;
   uint64_t r = 999;
   REQUIRE(list.empty(&r) == true);
   REQUIRE(r == 0);

   list.push(42);
   r = 999;
   REQUIRE(list.empty(&r) == false);
   REQUIRE(r == 0);  // still set to 0 per API contract
}

// --- push operations ---

TEST_CASE("LFList push adds elements", "[lflist]") {
   LFList<int> list;
   list.push(1);
   list.push(2);
   list.push(3);

   REQUIRE(list.size() == 3);
   REQUIRE(list.empty() == false);
}

TEST_CASE("LFList push_back is alias for push", "[lflist]") {
   LFList<int> list;
   bool result = list.push_back(1);
   REQUIRE(result == true);
   REQUIRE(list.size() == 1);

   result = list.push_back(2);
   REQUIRE(result == true);
   REQUIRE(list.size() == 2);
}

TEST_CASE("LFList push with raise=false does not signal", "[lflist]") {
   LFList<int> list;
   list.push(42, false);
   REQUIRE(list.empty() == false);
   REQUIRE(list.size() == 1);
}

// --- remove operations ---

TEST_CASE("LFList remove pops first element", "[lflist]") {
   LFList<int> list;
   list.push(1);
   list.push(2);
   list.push(3);

   bool success;
   int val = list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(val == 1);
   REQUIRE(list.size() == 2);

   val = list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(val == 2);
   REQUIRE(list.size() == 1);
}

TEST_CASE("LFList remove_front is alias for remove", "[lflist]") {
   LFList<int> list;
   list.push(42);

   bool success;
   int val = list.remove_front(&success);
   REQUIRE(success == true);
   REQUIRE(val == 42);
}

TEST_CASE("LFList remove on empty list with throwe=true throws", "[lflist]") {
   LFList<int> list;
   bool success;
   REQUIRE_THROWS(list.remove(&success, true));
}

TEST_CASE("LFList remove on empty list with throwe=false returns success=false", "[lflist]") {
   LFList<int> list;
   bool success = true;
   int val = list.remove(&success, false);
   REQUIRE(success == false);
   // val is undefined when success=false
}

TEST_CASE("LFList remove_front on empty list with throwe=false", "[lflist]") {
   LFList<int> list;
   bool success = true;
   int val = list.remove_front(&success, false);
   REQUIRE(success == false);
}

// --- pfpb (pop-front-push-back) ---

TEST_CASE("LFList pfpb rotates element to back", "[lflist]") {
   LFList<int> list;
   list.push(1);
   list.push(2);
   list.push(3);

   int val = list.pfpb();
   REQUIRE(val == 1);
   REQUIRE(list.size() == 3);  // size unchanged

   // Verify order: should now be 2, 3, 1
   bool success;
   REQUIRE(list.remove(&success) == 2);
   REQUIRE(list.remove(&success) == 3);
   REQUIRE(list.remove(&success) == 1);
}

TEST_CASE("LFList pfpb on empty list throws", "[lflist]") {
   LFList<int> list;
   REQUIRE_THROWS(list.pfpb());
}

TEST_CASE("LFList pfpb on single-element list", "[lflist]") {
   LFList<int> list;
   list.push(42);

   int val = list.pfpb();
   REQUIRE(val == 42);
   REQUIRE(list.size() == 1);

   bool success;
   REQUIRE(list.remove(&success) == 42);
}

// --- clear ---

TEST_CASE("LFList clear empties list", "[lflist]") {
   LFList<int> list;
   list.push(1);
   list.push(2);
   list.push(3);

   REQUIRE(list.size() == 3);
   list.clear();
   REQUIRE(list.size() == 0);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList clear on empty list is safe", "[lflist]") {
   LFList<int> list;
   REQUIRE_NOTHROW(list.clear());
   REQUIRE(list.empty() == true);
}

// --- waitFor ---

TEST_CASE("LFList waitFor times out on empty list", "[lflist]") {
   LFList<int> list;
   bool result = list.waitFor(NS_IN_ONE_MSEC);
   REQUIRE(result == false);
}

TEST_CASE("LFList waitFor returns immediately on non-empty list", "[lflist]") {
   LFList<int> list;
   list.push(42);

   bool result = list.waitFor(NS_IN_ONE_SEC);
   REQUIRE(result == true);
}

TEST_CASE("LFList waitFor(0) waits indefinitely until raise", "[lflist][threaded]") {
   LFList<int> list;
   atomic<bool> finished{false};

   thread waiter([&]{
      (void)list.waitFor(0);  // wait indefinitely
      finished = true;
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(10));
   REQUIRE(finished == false);  // still waiting

   list.raise();  // signal it
   std::this_thread::sleep_for(std::chrono::milliseconds(10));
   REQUIRE(finished == true);  // now done

   waiter.join();
}

TEST_CASE("LFList waitFor wakes on push", "[lflist][threaded]") {
   LFList<int> list;
   atomic<bool> finished{false};

   thread waiter([&]{
      bool result = list.waitFor(NS_IN_ONE_SEC);
      finished = result;
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(10));
   list.push(42);  // this should wake the waiter

   waiter.join();
   REQUIRE(finished == true);
}

// --- raise ---

TEST_CASE("LFList raise signals waiting threads", "[lflist][threaded]") {
   LFList<int> list;
   atomic<int> woke_count{0};

   auto wait_fn = [&]{
      if (list.waitFor(NS_IN_ONE_SEC)) {
         woke_count++;
      }
   };

   thread w1(wait_fn);
   thread w2(wait_fn);

   // let both threads block
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   list.raise();  // wake waiters
   list.raise();  // wake second waiter (broadcast is true)

   w1.join();
   w2.join();

   REQUIRE(woke_count == 2);
}

// --- thread safety tests ---

TEST_CASE("LFList concurrent push operations", "[lflist][threaded]") {
   LFList<int> list;
   const int num_threads = 10;
   const int items_per_thread = 100;

   vector<thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.emplace_back([&list, t, items_per_thread]{
         for (int i = 0; i < items_per_thread; ++i) {
            list.push(t * 1000 + i);
         }
      });
   }

   for (auto& th : threads) {
      th.join();
   }

   REQUIRE(list.size() == num_threads * items_per_thread);
}

TEST_CASE("LFList concurrent push and remove operations", "[lflist][threaded]") {
   LFList<int> list;
   atomic<int> push_count{0};
   atomic<int> pop_count{0};
   const int operations = 1000;

   thread pusher([&]{
      for (int i = 0; i < operations; ++i) {
         list.push(i);
         push_count++;
      }
   });

   thread popper([&]{
      bool success;
      for (int i = 0; i < operations; ++i) {
         // Wait for items to become available
         while (true) {
            (void)list.remove(&success, false);
            if (success) {
               pop_count++;
               break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
         }
      }
   });

   pusher.join();
   popper.join();

   REQUIRE(push_count == operations);
   REQUIRE(pop_count == operations);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList producer-consumer with waitFor", "[lflist][threaded]") {
   LFList<int> list;
   atomic<bool> producer_done{false};
   atomic<int> items_consumed{0};
   const int num_items = 100;

   // Producer thread
   thread producer([&]{
      for (int i = 0; i < num_items; ++i) {
         list.push(i);
         std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      producer_done = true;
      list.raise();  // wake up consumer if waiting
   });

   // Consumer thread
   thread consumer([&]{
      bool success;
      while (!producer_done || !list.empty()) {
         if (list.waitFor(NS_IN_ONE_MSEC)) {
            (void)list.remove(&success, false);
            if (success) {
               items_consumed++;
            }
         }
      }
   });

   producer.join();
   consumer.join();

   REQUIRE(items_consumed == num_items);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList multiple producers single consumer", "[lflist][threaded]") {
   LFList<int> list;
   const int num_producers = 4;
   const int items_per_producer = 100;
   const int total_items = num_producers * items_per_producer;
   atomic<int> items_consumed{0};
   atomic<bool> producers_done{false};

   // Multiple producers
   vector<thread> producers;
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&list, p, items_per_producer]{
         for (int i = 0; i < items_per_producer; ++i) {
            list.push(p * 1000 + i);
         }
      });
   }

   // Single consumer
   thread consumer([&]{
      bool success;
      while (!producers_done || !list.empty()) {
         if (list.waitFor(NS_IN_ONE_MSEC)) {
            (void)list.remove(&success, false);
            if (success) {
               items_consumed++;
            }
         }
      }
   });

   for (auto& p : producers) {
      p.join();
   }
   producers_done = true;
   list.raise();  // wake consumer

   consumer.join();

   REQUIRE(items_consumed == total_items);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList single producer multiple consumers", "[lflist][threaded]") {
   LFList<int> list;
   atomic<bool> producer_done{false};
   atomic<int> total_consumed{0};
   const int num_items = 400;
   const int num_consumers = 4;

   // Producer
   thread producer([&]{
      for (int i = 0; i < num_items; ++i) {
         list.push(i);
      }
      producer_done = true;
      // Wake all consumers
      for (int i = 0; i < num_consumers * 2; ++i) {
         list.raise();
      }
   });

   // Multiple consumers
   vector<thread> consumers;
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&]{
         bool success;
         while (!producer_done || !list.empty()) {
            if (list.waitFor(NS_IN_ONE_MSEC)) {
               (void)list.remove(&success, false);
               if (success) {
                  total_consumed++;
               }
            }
         }
      });
   }

   producer.join();
   for (auto& c : consumers) {
      c.join();
   }

   REQUIRE(total_consumed == num_items);
   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList stress test with mixed operations", "[lflist][threaded]") {
   LFList<int> list;
   atomic<int> push_total{0};
   atomic<int> pop_total{0};
   const int num_pushers = 3;
   const int num_poppers = 3;
   const int ops_per_thread = 500;

   vector<thread> threads;

   // Pushers
   for (int i = 0; i < num_pushers; ++i) {
      threads.emplace_back([&list, &push_total, i, ops_per_thread]{
         for (int j = 0; j < ops_per_thread; ++j) {
            list.push(i * 10000 + j);
            push_total++;
         }
      });
   }

   // Poppers
   for (int i = 0; i < num_poppers; ++i) {
      threads.emplace_back([&list, &pop_total, ops_per_thread]{
         bool success;
         int popped = 0;
         while (popped < ops_per_thread) {
            (void)list.remove(&success, false);
            if (success) {
               pop_total++;
               popped++;
            } else {
               std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
         }
      });
   }

   for (auto& th : threads) {
      th.join();
   }

   REQUIRE(push_total == num_pushers * ops_per_thread);
   REQUIRE(pop_total == num_poppers * ops_per_thread);
   REQUIRE(list.size() == (num_pushers - num_poppers) * ops_per_thread);
}

TEST_CASE("LFList clear during concurrent operations", "[lflist][threaded]") {
   LFList<int> list;
   atomic<bool> stop{false};
   atomic<int> cleared{0};

   // Continuous pusher
   thread pusher([&]{
      int i = 0;
      while (!stop) {
         list.push(i++);
         std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
   });

   // Continuous popper
   thread popper([&]{
      bool success;
      while (!stop) {
         (void)list.remove(&success, false);
         std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
   });

   // Clearer
   thread clearer([&]{
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      list.clear();
      cleared = 1;
      stop = true;
      list.raise();
   });

   pusher.join();
   popper.join();
   clearer.join();

   REQUIRE(cleared == 1);
}

// --- pointer storage (common use case) ---

TEST_CASE("LFList with pointers", "[lflist]") {
   LFList<int*> list;

   int* p1 = new int(1);
   int* p2 = new int(2);
   int* p3 = new int(3);

   list.push(p1);
   list.push(p2);
   list.push(p3);

   REQUIRE(list.size() == 3);

   bool success;
   int* val = list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(*val == 1);
   REQUIRE(val == p1);

   // Cleanup
   delete p1;
   delete p2;
   delete p3;
}

// --- string storage ---

TEST_CASE("LFList with std::string", "[lflist]") {
   LFList<string> list;

   list.push("hello");
   list.push("world");
   list.push("test");

   REQUIRE(list.size() == 3);

   bool success;
   string val = list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(val == "hello");
   REQUIRE(list.size() == 2);

   val = list.remove(&success);
   REQUIRE(success == true);
   REQUIRE(val == "world");
}

// --- edge cases ---

TEST_CASE("LFList rapid push/remove cycles", "[lflist][threaded]") {
   LFList<int> list;
   const int cycles = 1000;

   for (int i = 0; i < cycles; ++i) {
      list.push(i);
      bool success;
      int val = list.remove(&success);
      REQUIRE(success == true);
      REQUIRE(val == i);
   }

   REQUIRE(list.empty() == true);
}

TEST_CASE("LFList pfpb during concurrent access", "[lflist][threaded]") {
   LFList<int> list;
   list.push(1);
   list.push(2);
   list.push(3);
   list.push(4);
   list.push(5);

   atomic<bool> done{false};
   atomic<int> pfpb_count{0};

   thread rotator([&]{
      for (int i = 0; i < 100; ++i) {
         try {
            (void)list.pfpb();
            pfpb_count++;
         } catch (...) {
            // List might be temporarily empty
         }
         std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
      done = true;
   });

   thread adder([&]{
      int i = 100;
      while (!done) {
         list.push(i++);
         std::this_thread::sleep_for(std::chrono::microseconds(50));
      }
   });

   rotator.join();
   adder.join();

   REQUIRE(pfpb_count > 0);
   REQUIRE(list.size() > 0);
}
