/** @file rqlist.h
 *  @brief A lock-free ring queue for multi-producer single-consumer patterns
 *
 *  @class RQList
 *  @brief A concurrent lock-free ring queue implementation
 *
 *  @details This is a lock-free ring queue designed for multi-producer single-consumer (MPSC) patterns.
 *           The implementation uses a state machine approach with atomic slot reservation to provide
 *           thread-safe concurrent access without locks.
 *
 *           **Algorithm Design:**
 *           - Writers atomically reserve slots using a global counter (m_write)
 *           - Each slot has a state: EMPTY → RESERVED → FILLED → EMPTY
 *           - RESERVED state protects against readers accessing incomplete writes
 *           - Single reader consumes from m_read position
 *           - No ABA problem due to independent slot state machines
 *
 *           **Thread Safety:**
 *           - Multiple producers can push concurrently
 *           - Only ONE consumer thread is supported
 *           - All member methods except clear() are thread-safe
 *           - clear() must be called when no other threads are active
 *
 *           **Memory Ordering:**
 *           - Uses acquire/release semantics for synchronization
 *           - Guarantees visibility of data writes across threads
 *           - Portable to weak-memory architectures (ARM, etc.)
 *
 *  @tparam T     Element type (must be copyable)
 *  @tparam TSIZE Default queue size
 *
 *  @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 *  @see     https://github.com/dvadura/CRUtil
 *  @copyright Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 *
 *  @license You can obtain and redistribute or modify this program under the
 *           terms of the Software License Agreement provided in the file:
 *           <distribution-root>/LICENSE-APACHE-2.0.txt
 */

#ifndef __RQList_INC__
#define __RQList_INC__

#include "condition.h"
#include <atomic>
#include "ilflist.h"

using namespace std;

namespace crutil {
   template <class T, int TSIZE=2048>
   class RQList : public ILFList<T> {
      /// @brief State of a queue element
      enum class State : uint64_t {
         EMPTY    = 0,  ///< Slot is free for writers
         RESERVED = 1,  ///< Slot reserved by writer, data not yet valid
         FILLED   = 2   ///< Slot contains valid data, ready for reader
      };

      /// @brief Ring queue element with state machine
      typedef struct {
         atomic<uint64_t> state;  ///< Current state (EMPTY/RESERVED/FILLED)
         T element;               ///< Stored element
      } RQElement;

   private:
      atomic<uint64_t> m_write;    ///< Write position (monotonically increasing)
      atomic<uint64_t> m_read;     ///< Read position (monotonically increasing)
      RQElement*       m_list;     ///< Ring buffer array
      size_t           m_size;     ///< Size of ring buffer
      atomic<bool>     m_full;     ///< Queue full flag

      /// A condition variable that is used to wait for the list being non-empty.
      /// It is a broadcast conditional
      Condition m_notempty;

   public:
      /// @brief Default constructor, initializes the ring queue
      /// @param tag  Optional name for the condition variable (for debugging)
      /// @param size Size of the ring buffer
      RQList(const char* tag=nullptr, int size=TSIZE)
         : m_notempty(tag, true)
         , m_full(false)
      {
         m_size = size;
         m_list = new RQElement[size];
         clear();
      }

      /// @brief Destructor, cleans up the ring buffer
      virtual ~RQList()
      {
         delete[] m_list;  // Fixed: was delete (memory leak)
      }

      /// @brief Return the approximate current size of the queue
      /// @details Due to concurrent access, this is an approximate value.
      ///          The actual number of filled slots may differ slightly.
      /// @return Approximate number of elements in the queue
      [[nodiscard]] virtual size_t size()
      {
         uint64_t w = m_write.load(memory_order_relaxed);
         uint64_t r = m_read.load(memory_order_relaxed);

         // Note: This is approximate due to concurrent access
         // For exact count, would need to scan all slots
         return (w >= r) ? (w - r) : 0;
      }

      /// @brief Check if the queue is empty
      /// @return true if the queue appears empty, false otherwise
      [[nodiscard]] virtual bool empty()
      {
         uint64_t read_pos = m_read.load(memory_order_relaxed);
         size_t offset = read_pos % m_size;
         uint64_t state = m_list[offset].state.load(memory_order_acquire);
         return (state == static_cast<uint64_t>(State::EMPTY));
      }

      /// @brief Remove an element from the front of the queue
      /// @details Only ONE reader thread is supported. This method is NOT thread-safe
      ///          for multiple consumers.
      /// @param[out] success Set to true if element was removed, false if queue was empty
      /// @param throwe       Unused (kept for interface compatibility)
      /// @return The removed element, or default-constructed T if queue was empty
      [[nodiscard]] virtual T remove(bool* success, const bool throwe=true)
      {
         *success = false;

         // Get current read position
         uint64_t read_pos = m_read.load(memory_order_relaxed);
         size_t offset = read_pos % m_size;

         // Check slot state with acquire semantics to see data writes
         uint64_t state = m_list[offset].state.load(memory_order_acquire);

         if (state == static_cast<uint64_t>(State::EMPTY)) {
            // Queue is empty
            return T{};
         }

         if (state == static_cast<uint64_t>(State::RESERVED)) {
            // Writer in progress, data not ready yet
            // Single reader so no need to retry - just return empty
            return T{};
         }

         // State is FILLED - read data
         T result = m_list[offset].element;

         // Mark slot as EMPTY (release semantics ensures read happens before)
         m_list[offset].state.store(static_cast<uint64_t>(State::EMPTY),
                                     memory_order_release);

         // Advance read pointer
         m_read.fetch_add(1, memory_order_relaxed);

         *success = true;
         m_full.store(false, memory_order_relaxed);
         return result;
      }

      /// @brief Remove an element from the front (alias for remove)
      /// @param[out] success Set to true if element was removed, false if queue was empty
      /// @param throwe       Unused (kept for interface compatibility)
      /// @return The removed element, or default-constructed T if queue was empty
      [[nodiscard]] virtual T remove_front(bool* success, const bool throwe=true)
      {
         return remove(success, throwe);
      }

      /// @brief Push an element onto the back of the queue
      /// @details Thread-safe for multiple producers. Returns false if queue is full.
      /// @param item  Element to push
      /// @param raise If true, signal waiters that queue is non-empty
      /// @return true if element was pushed, false if queue was full
      [[nodiscard]] virtual bool push(const T& item, const bool raise=true)
      {
         uint64_t current_write, current_read, next_write;
         size_t offset;

         while (true) {
            // Load current write and read positions
            current_write = m_write.load(memory_order_relaxed);
            current_read = m_read.load(memory_order_relaxed);

            // Check if queue would be full after this write
            // We need to ensure write doesn't lap read position
            if (current_write - current_read >= m_size) {
               // Queue is full
               m_full.store(true, memory_order_relaxed);
               return false;
            }

            // Calculate offset for this write
            offset = current_write % m_size;

            // Try to reserve this slot by advancing write pointer
            next_write = current_write + 1;
            if (m_write.compare_exchange_weak(current_write, next_write,
                                              memory_order_relaxed, memory_order_relaxed)) {
               // Successfully reserved this slot number
               // Now wait for slot to be available (EMPTY)
               uint64_t expected;
               int retries = 0;
               while (retries < 100) {  // Spin briefly if slot not yet EMPTY
                  expected = static_cast<uint64_t>(State::EMPTY);
                  if (m_list[offset].state.compare_exchange_weak(
                          expected, static_cast<uint64_t>(State::RESERVED),
                          memory_order_acquire, memory_order_relaxed)) {
                     // Successfully claimed slot - write data
                     m_list[offset].element = item;

                     // Mark as FILLED (release semantics ensures element write is visible)
                     m_list[offset].state.store(static_cast<uint64_t>(State::FILLED),
                                                 memory_order_release);

                     if (likely(raise)) {
                        m_notempty.raise();
                     }
                     m_full.store(false, memory_order_relaxed);
                     return true;
                  }
                  retries++;
               }
               // Couldn't claim slot after many retries - queue is full or contended
               m_full.store(true, memory_order_relaxed);
               return false;
            }
            // CAS on m_write failed (another thread advanced it), retry
         }
      }

      /// @brief Push an element onto the back (alias for push)
      /// @param item  Element to push
      /// @param raise If true, signal waiters that queue is non-empty
      /// @return true if element was pushed, false if queue was full
      [[nodiscard]] virtual bool push_back(const T& item, const bool raise=true)
      {
         return push(item, raise);
      }

      /// @brief Wait for the queue to become non-empty, or timeout
      /// @param timeout Timeout in milliseconds (0 = wait indefinitely)
      /// @return true if queue became non-empty, false if timed out
      [[nodiscard]] virtual bool waitFor(const uint64_t timeout=0)
      {
         if (!empty()) {
            return true;
         }

         return (m_notempty.waitFor(timeout) == 0);
      }

      /// @brief Signal waiters even if queue is empty
      /// @details Useful for waking up threads during shutdown
      virtual void raise()
      {
          m_notempty.raise();
      }

      /// @brief Clear the queue and reset all state
      /// @details NOT thread-safe. Must be called when no other threads are accessing the queue.
      virtual void clear()
      {
         // Reset all slots to EMPTY
         for (size_t i = 0; i < m_size; ++i) {
            m_list[i].state.store(static_cast<uint64_t>(State::EMPTY),
                                  memory_order_relaxed);
         }

         m_write.store(0, memory_order_relaxed);
         m_read.store(0, memory_order_relaxed);
         m_full.store(false, memory_order_relaxed);
      }
   };
};

#endif
