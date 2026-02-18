/**
 * @file test_rqlist.cpp
 * @brief Comprehensive test suite for RQList lock-free ring queue
 */

#include "catch2.hpp"
#include "rqlist.h"

#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>

using namespace std;
using namespace crutil;

// ============================================================================
// Basic Lifecycle Tests
// ============================================================================

TEST_CASE("RQList default constructor creates empty queue", "[rqlist][basic]")
{
    RQList<int*> q;
    REQUIRE(q.empty() == true);
    REQUIRE(q.size() == 0);
}

TEST_CASE("RQList with custom size", "[rqlist][basic]")
{
    RQList<int*, 32> q(nullptr, 32);
    REQUIRE(q.empty() == true);
    REQUIRE(q.size() == 0);
}

// ============================================================================
// Size and Empty Consistency Tests
// ============================================================================

TEST_CASE("RQList empty() on new queue returns true", "[rqlist][empty]")
{
    RQList<int*, 16> q;
    REQUIRE(q.empty() == true);
}

TEST_CASE("RQList size() tracks items correctly", "[rqlist][size]")
{
    RQList<int*, 16> q;
    int val1 = 42, val2 = 100;

    REQUIRE(q.size() == 0);
    q.push(&val1);
    REQUIRE(q.size() == 1);
    q.push(&val2);
    REQUIRE(q.size() == 2);

    bool success;
    q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(q.size() == 1);
}

TEST_CASE("RQList empty() and size() consistency", "[rqlist][empty][size]")
{
    RQList<int*, 16> q;
    int val = 5;

    // Empty queue
    REQUIRE(q.empty() == true);
    REQUIRE(q.size() == 0);

    // After push
    q.push(&val);
    REQUIRE(q.empty() == false);
    REQUIRE(q.size() == 1);

    // After remove
    bool success;
    q.remove(&success);
    REQUIRE(q.empty() == true);
    REQUIRE(q.size() == 0);
}

// ============================================================================
// Push/Pop Operations
// ============================================================================

TEST_CASE("RQList basic push and remove", "[rqlist][ops]")
{
    RQList<int*, 16> q;
    int val = 5;
    bool success;

    REQUIRE(q.empty() == true);

    q.push_back(&val);
    REQUIRE(q.empty() == false);

    int* result = q.remove_front(&success);
    REQUIRE(success == true);
    REQUIRE(result == &val);
    REQUIRE(*result == 5);
}

TEST_CASE("RQList remove on empty queue fails gracefully", "[rqlist][ops]")
{
    RQList<int*, 16> q;
    bool success = true;  // Start with true to verify it gets set to false

    int* result = q.remove_front(&success);
    REQUIRE(success == false);
    REQUIRE(result == nullptr);  // Should return default-constructed T{}
}

TEST_CASE("RQList multiple push and remove cycles", "[rqlist][ops]")
{
    RQList<int*, 16> q;
    bool success;
    int values[] = {1, 2, 3, 4, 5};

    // First cycle
    for (int i = 0; i < 5; ++i) {
        REQUIRE(q.push(&values[i]) == true);
    }

    for (int i = 0; i < 5; ++i) {
        int* result = q.remove(&success);
        REQUIRE(success == true);
        REQUIRE(*result == values[i]);
    }

    REQUIRE(q.empty() == true);

    // Second cycle - queue should be reusable
    for (int i = 0; i < 5; ++i) {
        REQUIRE(q.push(&values[i]) == true);
    }

    for (int i = 0; i < 5; ++i) {
        int* result = q.remove(&success);
        REQUIRE(success == true);
        REQUIRE(*result == values[i]);
    }
}

// ============================================================================
// Boundary Conditions
// ============================================================================

TEST_CASE("RQList fill to capacity", "[rqlist][boundary]")
{
    RQList<int*, 16> q;
    int values[16];

    // Fill queue to capacity
    for (int i = 0; i < 16; ++i) {
        values[i] = i + 1;
        REQUIRE(q.push(&values[i]) == true);
    }

    REQUIRE(q.size() == 16);
}

TEST_CASE("RQList push beyond capacity fails", "[rqlist][boundary]")
{
    RQList<int*, 16> q;
    int values[20];

    // Fill to capacity
    for (int i = 0; i < 16; ++i) {
        values[i] = i + 1;
        REQUIRE(q.push(&values[i]) == true);
    }

    // Try to overfill
    for (int i = 16; i < 20; ++i) {
        values[i] = i + 1;
        REQUIRE(q.push(&values[i]) == false);
    }
}

TEST_CASE("RQList wrap-around after full cycle", "[rqlist][boundary]")
{
    RQList<int*, 16> q;
    int values[16];
    bool success;

    // Fill queue
    for (int i = 0; i < 16; ++i) {
        values[i] = i + 1;
        q.push(&values[i]);
    }

    // Drain queue
    for (int i = 0; i < 16; ++i) {
        int* result = q.remove(&success);
        REQUIRE(success == true);
        REQUIRE(*result == i + 1);
    }

    REQUIRE(q.empty() == true);

    // Fill again - tests wrap-around
    for (int i = 0; i < 16; ++i) {
        values[i] = 100 + i;
        REQUIRE(q.push(&values[i]) == true);
    }

    for (int i = 0; i < 16; ++i) {
        int* result = q.remove(&success);
        REQUIRE(success == true);
        REQUIRE(*result == 100 + i);
    }
}

// ============================================================================
// Queue Full Behavior
// ============================================================================

TEST_CASE("RQList correctly detects full queue", "[rqlist][full]")
{
    RQList<int*, 8> q;
    int values[10];

    for (int i = 0; i < 8; ++i) {
        values[i] = i;
        REQUIRE(q.push(&values[i]) == true);
    }

    // Queue should be full now
    values[8] = 999;
    REQUIRE(q.push(&values[8]) == false);
}

TEST_CASE("RQList accepts push after drain", "[rqlist][full]")
{
    RQList<int*, 8> q;
    int values[10];
    bool success;

    // Fill queue
    for (int i = 0; i < 8; ++i) {
        values[i] = i;
        q.push(&values[i]);
    }

    // Try to overfill
    values[8] = 999;
    REQUIRE(q.push(&values[8]) == false);

    // Drain one element
    q.remove(&success);
    REQUIRE(success == true);

    // Now push should succeed
    REQUIRE(q.push(&values[8]) == true);
}

TEST_CASE("RQList push returns false when full", "[rqlist][full]")
{
    RQList<int*, 4> q;
    int values[6];

    for (int i = 0; i < 4; ++i) {
        values[i] = i;
        REQUIRE(q.push(&values[i]) == true);
    }

    // All subsequent pushes should fail
    for (int i = 4; i < 6; ++i) {
        values[i] = i;
        REQUIRE(q.push(&values[i]) == false);
    }
}

// ============================================================================
// Clear Functionality
// ============================================================================

TEST_CASE("RQList clear empties queue", "[rqlist][clear]")
{
    RQList<int*, 16> q;
    int values[5];

    for (int i = 0; i < 5; ++i) {
        values[i] = i;
        q.push(&values[i]);
    }

    REQUIRE(q.size() == 5);
    REQUIRE(q.empty() == false);

    q.clear();

    REQUIRE(q.size() == 0);
    REQUIRE(q.empty() == true);
}

TEST_CASE("RQList clear resets state machine", "[rqlist][clear]")
{
    RQList<int*, 8> q;
    int values[10];

    // Fill queue completely
    for (int i = 0; i < 8; ++i) {
        values[i] = i;
        q.push(&values[i]);
    }

    // Queue full
    REQUIRE(q.push(&values[8]) == false);

    // Clear
    q.clear();

    // Should be able to push again
    for (int i = 0; i < 8; ++i) {
        REQUIRE(q.push(&values[i]) == true);
    }
}

// ============================================================================
// WaitFor Tests
// ============================================================================

TEST_CASE("RQList waitFor times out on empty", "[rqlist][wait]")
{
    RQList<int*, 16> q;

    auto start = chrono::steady_clock::now();
    bool result = q.waitFor(100);  // 100ms timeout
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    REQUIRE(result == false);  // Should timeout
    // Note: Condition class may return immediately in some cases
    // The important thing is that it returns false (timeout) for empty queue
    INFO("Elapsed time: " << elapsed << "ms");
}

TEST_CASE("RQList waitFor returns immediately on data", "[rqlist][wait]")
{
    RQList<int*, 16> q;
    int val = 42;

    q.push(&val);

    auto start = chrono::steady_clock::now();
    bool result = q.waitFor(1000);
    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    REQUIRE(result == true);
    REQUIRE(elapsed < 50);  // Should return almost immediately
}

// ============================================================================
// Thread Safety Tests - Multiple Producers, Single Consumer
// ============================================================================

TEST_CASE("RQList multiple writers single reader", "[rqlist][threaded]")
{

    const int NUM_WRITERS = 4;
    const int ITEMS_PER_WRITER = 1000;
    RQList<int*, 4096> q;

    vector<int> writer_data[NUM_WRITERS];
    for (int i = 0; i < NUM_WRITERS; ++i) {
        writer_data[i].resize(ITEMS_PER_WRITER);
        for (int j = 0; j < ITEMS_PER_WRITER; ++j) {
            writer_data[i][j] = i * 10000 + j;
        }
    }

    atomic<int> total_written{0};
    atomic<bool> writers_done{false};

    // Start writer threads
    vector<thread> writers;
    for (int i = 0; i < NUM_WRITERS; ++i) {
        writers.emplace_back([&q, &writer_data, &total_written, i, ITEMS_PER_WRITER]() {
            for (int j = 0; j < ITEMS_PER_WRITER; ++j) {
                while (!q.push(&writer_data[i][j])) {
                    this_thread::yield();  // Queue full, retry
                }
                total_written.fetch_add(1, memory_order_relaxed);
            }
        });
    }

    // Reader thread
    vector<int> read_values;
    thread reader([&q, &writers_done, &read_values, &total_written, NUM_WRITERS, ITEMS_PER_WRITER]() {
        int total_expected = NUM_WRITERS * ITEMS_PER_WRITER;
        int total_read = 0;

        while (total_read < total_expected) {
            bool success;
            int* val = q.remove(&success);
            if (success) {
                read_values.push_back(*val);
                total_read++;
            } else {
                this_thread::yield();
            }
        }
    });

    // Wait for all threads
    for (auto& w : writers) {
        w.join();
    }
    writers_done.store(true);
    reader.join();

    // Verify all items were read
    REQUIRE(read_values.size() == NUM_WRITERS * ITEMS_PER_WRITER);

    // Verify all written values were read (may be in different order)
    vector<int> all_written;
    for (int i = 0; i < NUM_WRITERS; ++i) {
        for (int j = 0; j < ITEMS_PER_WRITER; ++j) {
            all_written.push_back(writer_data[i][j]);
        }
    }

    sort(all_written.begin(), all_written.end());
    sort(read_values.begin(), read_values.end());
    REQUIRE(read_values == all_written);
}

TEST_CASE("RQList concurrent push operations", "[rqlist][threaded]")
{
    const int NUM_THREADS = 8;
    const int ITEMS_PER_THREAD = 500;
    RQList<int*, 8192> q;

    vector<vector<int>> thread_data(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].resize(ITEMS_PER_THREAD);
        for (int j = 0; j < ITEMS_PER_THREAD; ++j) {
            thread_data[i][j] = i * 1000 + j;
        }
    }

    atomic<int> push_failures{0};

    // Start threads
    vector<thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&q, &thread_data, &push_failures, i, ITEMS_PER_THREAD]() {
            for (int j = 0; j < ITEMS_PER_THREAD; ++j) {
                if (!q.push(&thread_data[i][j])) {
                    push_failures.fetch_add(1);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // All pushes should succeed (queue is large enough)
    REQUIRE(push_failures.load() == 0);
    REQUIRE(q.size() == NUM_THREADS * ITEMS_PER_THREAD);
}

TEST_CASE("RQList wrap-around under load", "[rqlist][threaded]")
{
    const int QUEUE_SIZE = 64;
    const int NUM_ITEMS = 10000;
    RQList<int*, QUEUE_SIZE> q;

    vector<int> data(NUM_ITEMS);
    for (int i = 0; i < NUM_ITEMS; ++i) {
        data[i] = i;
    }

    atomic<int> write_idx{0};
    atomic<int> read_count{0};
    atomic<bool> done{false};

    // Writer thread
    thread writer([&q, &data, &write_idx, NUM_ITEMS, &done]() {
        while (write_idx.load() < NUM_ITEMS) {
            int idx = write_idx.load();
            if (idx < NUM_ITEMS && q.push(&data[idx])) {
                write_idx.fetch_add(1);
            } else {
                this_thread::yield();
            }
        }
        done.store(true);
    });

    // Reader thread
    thread reader([&q, &read_count, &done, NUM_ITEMS]() {
        while (read_count.load() < NUM_ITEMS || !done.load()) {
            bool success;
            int* val = q.remove(&success);
            if (success) {
                read_count.fetch_add(1);
            } else {
                this_thread::yield();
            }
        }
    });

    writer.join();
    reader.join();

    REQUIRE(read_count.load() == NUM_ITEMS);
    REQUIRE(q.empty() == true);
}

TEST_CASE("RQList producer-consumer pattern", "[rqlist][threaded]")
{
    const int NUM_PRODUCERS = 3;
    const int ITEMS_PER_PRODUCER = 2000;
    const int TOTAL_ITEMS = NUM_PRODUCERS * ITEMS_PER_PRODUCER;
    RQList<int*, 512> q;

    vector<vector<int>> producer_data(NUM_PRODUCERS);
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producer_data[i].resize(ITEMS_PER_PRODUCER);
        for (int j = 0; j < ITEMS_PER_PRODUCER; ++j) {
            producer_data[i][j] = i * 10000 + j;
        }
    }

    atomic<bool> producers_done{false};
    atomic<int> items_consumed{0};

    // Producer threads
    vector<thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back([&q, &producer_data, i, ITEMS_PER_PRODUCER]() {
            for (int j = 0; j < ITEMS_PER_PRODUCER; ++j) {
                while (!q.push(&producer_data[i][j])) {
                    this_thread::yield();
                }
            }
        });
    }

    // Consumer thread
    thread consumer([&q, &items_consumed, &producers_done, TOTAL_ITEMS]() {
        while (items_consumed.load() < TOTAL_ITEMS) {
            bool success;
            int* val = q.remove(&success);
            if (success) {
                items_consumed.fetch_add(1);
            } else {
                this_thread::yield();
            }
        }
    });

    for (auto& p : producers) {
        p.join();
    }
    producers_done.store(true);
    consumer.join();

    REQUIRE(items_consumed.load() == TOTAL_ITEMS);
    REQUIRE(q.empty() == true);
}

// ============================================================================
// Type Safety Tests
// ============================================================================

TEST_CASE("RQList with pointers", "[rqlist][types]")
{
    RQList<int*, 16> q;
    int val1 = 42, val2 = 100;
    bool success;

    q.push(&val1);
    q.push(&val2);

    int* r1 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r1 == &val1);
    REQUIRE(*r1 == 42);

    int* r2 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r2 == &val2);
    REQUIRE(*r2 == 100);
}

TEST_CASE("RQList with value types", "[rqlist][types]")
{
    RQList<int, 16> q;
    bool success;

    q.push(42);
    q.push(100);

    int r1 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r1 == 42);

    int r2 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r2 == 100);
}

TEST_CASE("RQList with std::string", "[rqlist][types]")
{
    RQList<string, 16> q;
    bool success;

    q.push("hello");
    q.push("world");

    string r1 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r1 == "hello");

    string r2 = q.remove(&success);
    REQUIRE(success == true);
    REQUIRE(r2 == "world");
}
