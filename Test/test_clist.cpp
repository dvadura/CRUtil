// CList tests using Catch2 framework

#include "catch2.hpp"
#include "clist.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <string>

using namespace std;
using namespace crutil;

// --- basic lifecycle ---

TEST_CASE("CList default constructor creates empty list", "[clist]") {
   CList<int> list;
   REQUIRE(list.empty() == true);
   REQUIRE(list.size() == 0);
}

TEST_CASE("CList single-item constructor", "[clist]") {
   CList<int> list(42);
   REQUIRE(list.empty() == false);
   REQUIRE(list.size() == 1);
   REQUIRE(list.front() == 42);
}

TEST_CASE("CList move constructor", "[clist]") {
   CList<int> list1(1);
   list1.push_back(2);
   list1.push_back(3);

   CList<int> list2(std::move(list1));
   REQUIRE(list2.size() == 3);
   REQUIRE(list2.front() == 1);
}

TEST_CASE("CList tagged constructor", "[clist]") {
   REQUIRE_NOTHROW([]{ CList<int> list("TAGGED"); }());
}

// --- size and empty ---

TEST_CASE("CList size tracking after add operations", "[clist]") {
   CList<int> list;
   REQUIRE(list.size() == 0);

   list.add(1);
   REQUIRE(list.size() == 1);

   list.add(2);
   REQUIRE(list.size() == 2);

   list.add(3);
   REQUIRE(list.size() == 3);
}

TEST_CASE("CList empty() returns correct state", "[clist]") {
   CList<int> list;
   REQUIRE(list.empty() == true);

   list.push_back(1);
   REQUIRE(list.empty() == false);

   list.pop_front();
   REQUIRE(list.empty() == true);
}

// --- push operations ---

TEST_CASE("CList push_front adds to front", "[clist]") {
   CList<int> list;
   list.push_front(1);
   list.push_front(2);
   list.push_front(3);

   REQUIRE(list.size() == 3);
   REQUIRE(list.front() == 3);
}

TEST_CASE("CList push_back adds to back", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   REQUIRE(list.size() == 3);
   REQUIRE(list.back() == 3);
   REQUIRE(list.front() == 1);
}

TEST_CASE("CList add is alias for push_back", "[clist]") {
   CList<int> list;
   list.add(1);
   list.add(2);

   REQUIRE(list.size() == 2);
   REQUIRE(list.front() == 1);
   REQUIRE(list.back() == 2);
}

// --- front and back ---

TEST_CASE("CList front() returns first element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   REQUIRE(list.front() == 1);
   REQUIRE(list.size() == 3);  // front() doesn't remove
}

TEST_CASE("CList back() returns last element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   REQUIRE(list.back() == 3);
   REQUIRE(list.size() == 3);  // back() doesn't remove
}

TEST_CASE("CList front() on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.front());
}

TEST_CASE("CList back() on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.back());
}

// --- pop operations ---

TEST_CASE("CList pop_front removes first element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   list.pop_front();
   REQUIRE(list.size() == 2);
   REQUIRE(list.front() == 2);
}

TEST_CASE("CList pop_back removes last element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   list.pop_back();
   REQUIRE(list.size() == 2);
   REQUIRE(list.back() == 2);
}

TEST_CASE("CList pop_front on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.pop_front());
}

TEST_CASE("CList pop_back on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.pop_back());
}

// --- remove operations ---

TEST_CASE("CList remove_front returns and removes element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   int val = list.remove_front();
   REQUIRE(val == 1);
   REQUIRE(list.size() == 2);
   REQUIRE(list.front() == 2);
}

TEST_CASE("CList remove_back returns and removes element", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   int val = list.remove_back();
   REQUIRE(val == 3);
   REQUIRE(list.size() == 2);
   REQUIRE(list.back() == 2);
}

TEST_CASE("CList remove_front on empty list with throwe=true throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.remove_front(true));
}

TEST_CASE("CList remove_front on empty list with throwe=false returns default", "[clist]") {
   CList<int> list;
   int val = list.remove_front(false);
   REQUIRE(errno == -1);
   REQUIRE(val == 0);  // default-initialized int
}

TEST_CASE("CList remove_back on empty list with throwe=true throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.remove_back(true));
}

TEST_CASE("CList remove_back on empty list with throwe=false returns default", "[clist]") {
   CList<int> list;
   int val = list.remove_back(false);
   REQUIRE(errno == -1);
   REQUIRE(val == 0);  // default-initialized int
}

// --- pfpb (pop-front-push-back) ---

TEST_CASE("CList pfpb rotates element to back", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   int val = list.pfpb();
   REQUIRE(val == 1);
   REQUIRE(list.size() == 3);  // size unchanged
   REQUIRE(list.front() == 2);  // 2 is now front
   REQUIRE(list.back() == 1);   // 1 is now back
}

TEST_CASE("CList pfpb on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.pfpb());
}

TEST_CASE("CList pfpb on single-element list", "[clist]") {
   CList<int> list;
   list.push_back(42);

   int val = list.pfpb();
   REQUIRE(val == 42);
   REQUIRE(list.size() == 1);
   REQUIRE(list.front() == 42);
}

// --- splice operations ---

TEST_CASE("CList splice moves all elements from source", "[clist]") {
   CList<int> list1;
   list1.push_back(1);
   list1.push_back(2);

   CList<int> list2;
   list2.push_back(3);
   list2.push_back(4);
   list2.push_back(5);

   unsigned int size = list1.splice(std::move(list2));
   REQUIRE(size == 5);
   REQUIRE(list1.size() == 5);
   REQUIRE(list2.size() == 0);
   REQUIRE(list2.empty() == true);
}

TEST_CASE("CList splice preserves order", "[clist]") {
   CList<int> list1;
   list1.push_back(1);
   list1.push_back(2);

   CList<int> list2;
   list2.push_back(3);
   list2.push_back(4);

   list1.splice(std::move(list2));
   REQUIRE(list1.front() == 1);
   REQUIRE(list1.remove_front() == 1);
   REQUIRE(list1.remove_front() == 2);
   REQUIRE(list1.remove_front() == 3);
   REQUIRE(list1.remove_front() == 4);
}

TEST_CASE("CList splice on empty source is no-op", "[clist]") {
   CList<int> list1;
   list1.push_back(1);
   list1.push_back(2);

   CList<int> list2;

   unsigned int size = list1.splice(std::move(list2));
   REQUIRE(size == 2);
   REQUIRE(list1.size() == 2);
}

TEST_CASE("CList add(CList&&) alias for splice", "[clist]") {
   CList<int> list1;
   list1.push_back(1);

   CList<int> list2;
   list2.push_back(2);
   list2.push_back(3);

   unsigned int size = list1.add(std::move(list2));
   REQUIRE(size == 3);
   REQUIRE(list1.size() == 3);
   REQUIRE(list2.empty() == true);
}

// --- remove by value ---

TEST_CASE("CList remove(value) removes all matching elements", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(4);
   list.push_back(2);
   list.push_back(4);
   list.push_back(3);
   list.push_back(4);

   unsigned int size = list.remove(4);
   REQUIRE(size == 3);
   REQUIRE(list.size() == 3);
   REQUIRE(list.front() == 1);
}

TEST_CASE("CList remove on empty list throws", "[clist]") {
   CList<int> list;
   REQUIRE_THROWS(list.remove(42));
}

TEST_CASE("CList remove when value not found", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   unsigned int size = list.remove(99);
   REQUIRE(size == 3);  // all elements remain
}

// --- clear ---

TEST_CASE("CList clear empties list", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   list.push_back(3);

   REQUIRE(list.size() == 3);
   list.clear();
   REQUIRE(list.size() == 0);
   REQUIRE(list.empty() == true);
}

TEST_CASE("CList clear on empty list is safe", "[clist]") {
   CList<int> list;
   REQUIRE_NOTHROW(list.clear());
   REQUIRE(list.empty() == true);
}

// --- waitFor ---

TEST_CASE("CList waitFor times out on empty list", "[clist]") {
   CList<int> list;
   bool result = list.waitFor(NS_IN_ONE_MSEC);
   REQUIRE(result == false);
}

TEST_CASE("CList waitFor returns immediately on non-empty list", "[clist]") {
   CList<int> list;
   list.push_back(42);

   bool result = list.waitFor(NS_IN_ONE_SEC);
   REQUIRE(result == true);
}

TEST_CASE("CList waitFor(0) waits indefinitely until raise", "[clist][threaded]") {
   CList<int> list;
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

// --- freeze/thaw ---

TEST_CASE("CList freeze returns size and locks", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);

   size_t size = list.freeze();
   REQUIRE(size == 2);
   list.thaw();
}

TEST_CASE("CList freeze/thaw pairing", "[clist]") {
   CList<int> list;
   REQUIRE_NOTHROW([]{
      CList<int> l;
      l.freeze();
      l.thaw();
   }());
}

// --- fit ---

TEST_CASE("CList fit shrinks capacity", "[clist]") {
   CList<int> list;
   list.push_back(1);
   list.push_back(2);
   REQUIRE_NOTHROW(list.fit());
}

// --- move assignment ---

TEST_CASE("CList move assignment operator", "[clist]") {
   CList<int> list1;
   list1.push_back(1);
   list1.push_back(2);
   list1.push_back(3);

   CList<int> list2;
   list2.push_back(99);

   list2 = std::move(list1);
   REQUIRE(list2.size() == 3);
   REQUIRE(list2.front() == 1);
   REQUIRE(list1.empty() == true);
}

// --- thread safety tests ---

TEST_CASE("CList concurrent push_back operations", "[clist][threaded]") {
   CList<int> list;
   const int num_threads = 10;
   const int items_per_thread = 100;

   vector<thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.emplace_back([&list, t, items_per_thread]{
         for (int i = 0; i < items_per_thread; ++i) {
            list.push_back(t * 1000 + i);
         }
      });
   }

   for (auto& th : threads) {
      th.join();
   }

   REQUIRE(list.size() == num_threads * items_per_thread);
}

TEST_CASE("CList concurrent push_front operations", "[clist][threaded]") {
   CList<int> list;
   const int num_threads = 10;
   const int items_per_thread = 100;

   vector<thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.emplace_back([&list, t, items_per_thread]{
         for (int i = 0; i < items_per_thread; ++i) {
            list.push_front(t * 1000 + i);
         }
      });
   }

   for (auto& th : threads) {
      th.join();
   }

   REQUIRE(list.size() == num_threads * items_per_thread);
}

TEST_CASE("CList concurrent mixed push/pop operations", "[clist][threaded]") {
   CList<int> list;
   atomic<int> push_count{0};
   atomic<int> pop_count{0};
   const int operations = 1000;

   thread pusher([&]{
      for (int i = 0; i < operations; ++i) {
         list.push_back(i);
         push_count++;
         std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
   });

   thread popper([&]{
      for (int i = 0; i < operations; ++i) {
         while (list.empty()) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
         }
         (void)list.remove_front();
         pop_count++;
      }
   });

   pusher.join();
   popper.join();

   REQUIRE(push_count == operations);
   REQUIRE(pop_count == operations);
   REQUIRE(list.empty() == true);
}

TEST_CASE("CList producer-consumer with waitFor", "[clist][threaded]") {
   CList<int> list;
   atomic<bool> producer_done{false};
   atomic<int> items_consumed{0};
   const int num_items = 100;

   // Producer thread
   thread producer([&]{
      for (int i = 0; i < num_items; ++i) {
         list.push_back(i);
         std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      producer_done = true;
      list.raise();  // wake up consumer if waiting
   });

   // Consumer thread
   thread consumer([&]{
      while (!producer_done || !list.empty()) {
         if (list.waitFor(NS_IN_ONE_MSEC)) {
            if (!list.empty()) {
               (void)list.remove_front();
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

TEST_CASE("CList multiple consumers with waitFor", "[clist][threaded]") {
   CList<int> list;
   atomic<bool> done{false};
   atomic<int> total_consumed{0};
   const int num_items = 100;
   const int num_consumers = 4;

   // Producer
   thread producer([&]{
      for (int i = 0; i < num_items; ++i) {
         list.push_back(i);
      }
      done = true;
      // Wake all consumers
      for (int i = 0; i < num_consumers; ++i) {
         list.raise();
      }
   });

   // Multiple consumers
   vector<thread> consumers;
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&]{
         while (!done || !list.empty()) {
            if (list.waitFor(NS_IN_ONE_MSEC)) {
               if (!list.empty()) {
                  (void)list.remove_front(false);  // don't throw if empty
                  if (errno == 0) {
                     total_consumed++;
                  }
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

// --- pointer storage (common use case) ---

TEST_CASE("CList with pointers", "[clist]") {
   CList<int*> list;

   int* p1 = new int(1);
   int* p2 = new int(2);
   int* p3 = new int(3);

   list.push_back(p1);
   list.push_back(p2);
   list.push_back(p3);

   REQUIRE(list.size() == 3);
   REQUIRE(*list.front() == 1);

   int* val = list.remove_front();
   REQUIRE(*val == 1);
   REQUIRE(val == p1);

   // Cleanup
   delete p1;
   delete p2;
   delete p3;
}

// --- string storage ---

TEST_CASE("CList with std::string", "[clist]") {
   CList<string> list;

   list.push_back("hello");
   list.push_back("world");
   list.push_back("test");

   REQUIRE(list.size() == 3);
   REQUIRE(list.front() == "hello");
   REQUIRE(list.back() == "test");

   string val = list.remove_front();
   REQUIRE(val == "hello");
   REQUIRE(list.size() == 2);
}
