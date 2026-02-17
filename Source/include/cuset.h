/** @file   cuset.h
 *
 * @class  CUSet
 *
 * @brief   A concurrent unordered set implementation using recursive mutex locking
 *
 * @details This is a thread-safe wrapper around std::unordered_set providing concurrent access
 *          control via Semaphore (recursive mutex). All member methods are internally synchronized.
 *
 *          IMPORTANT THREAD-SAFETY NOTES:
 *          - Iterator methods (begin/end) have been removed as they cannot be made safe without
 *            external locking. Exposing iterators while holding internal locks would create
 *            deadlock risks; exposing them without locks creates use-after-free vulnerabilities.
 *          - Query methods (size/empty/contains) return point-in-time snapshots that may be stale
 *            immediately after return. Use freeze/thaw for consistent multi-operation reads.
 *          - Set semantics enforced: duplicate insertions are idempotent (no duplicates stored).
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
 * @copyright Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 *
 * @license You can obtain and redistribute or modify this program under the
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE-APACHE-2.0.txt
 */

#ifndef __CUSET_INC__
#define __CUSET_INC__

#include <unordered_set>
#include "condition.h"

#define MSG_EMPTY "the container is empty"

namespace crutil {
   template <class T>
   class CUSet : protected Semaphore {
   private:
      /// The underlying STL set that actually stores the data.
      std::unordered_set<T> m_data;

      /// A condition variable that is used to wait for the list being non-empty.
      /// It is a broadcast conditional
      Condition m_notempty;

   public:
      /// A default constructor, initializes the underlying set and sets the length to 0.
      CUSet(const char* tag=nullptr) : Semaphore(true, false), m_notempty(tag, true)
      {
      }

      /// A constructor that places a single item on the set.
      CUSet(T& item, const char* tag=nullptr) : Semaphore(true, false), m_notempty(tag, true)
      {
         m_data.insert(item);
         m_notempty.raise();
      }

      /// Move constructor - transfers ownership of data from source set.
      /// THREAD-SAFE: Locks source object during move operation.
      CUSet(CUSet<T>&& list, const char* tag=nullptr) : Semaphore(true, false), m_notempty(tag, true)
      {
         list.PP;
         m_data = std::move(list.m_data);
         bool has_data = !m_data.empty();
         list.VV;

         if (has_data) {
            m_notempty.raise();
         }
      }

      /// A destructor, empties the set and resets the conditional.
      virtual ~CUSet()
      {
         PP;
         m_data.clear();
         m_notempty.raise();
         VV;
      }

      /// Return the current length of the set.
      /// THREAD-SAFE: Returns point-in-time snapshot that may be stale immediately.
      [[nodiscard]] inline size_t size() {
         size_t result;
         PP;
         result = m_data.size();
         VV;
         return result;
      }

      /// Test if the set is currently empty.
      /// THREAD-SAFE: Returns point-in-time snapshot that may be stale immediately.
      [[nodiscard]] inline bool empty() {
         bool result;
         PP;
         result = m_data.empty();
         VV;
         return result;
      }

      /// A trivial wrapper on insert
      inline void add(const T& item) {
         PP;
         m_data.insert(item);
         m_notempty.raise();
         VV;
         return;
      }

      /// A trivial wrapper on splice
      inline unsigned int add(CUSet<T>&& lst) {
         return splice(std::move(lst));
      }

      /// Splice all elements from source set into this set, emptying the source.
      /// THREAD-SAFE: Locks both sets during transfer.
      /// EXCEPTION-SAFE: Ensures source is cleared even if insert throws.
      inline unsigned int splice(CUSet<T>&& lst) {
         if (lst.empty() == true) {
            return m_data.size();
         }

         PP;
         bool raise = m_data.empty();

         lst.PP;
         try {
            for (typename std::unordered_set<T>::iterator it = lst.m_data.begin(); it != lst.m_data.end(); ++it) {
               m_data.insert(std::move(*it));
            }
            lst.m_data.clear();
         } catch (...) {
            lst.m_data.clear();  // Ensure source is cleared even on exception
            lst.VV;
            VV;
            throw;
         }
         lst.VV;

         if (raise == true) {
            m_notempty.raise();
         }

         size_t result = m_data.size();
         VV;

         return result;
      }

      /// Wait for the set to become non-empty, or timeout
      /// If timeout is 0, then wait indefinitely, return false if we timed out.
      /// THREAD-SAFE: Check-then-wait is atomic via proper locking.
      /// @return true if data available, false if timeout
      [[nodiscard]] inline bool waitFor(const uint64_t timeout=0) {
         // Atomically check if we have data under lock to avoid race
         PP;
         bool has_data = !m_data.empty();
         VV;

         if (has_data) {
            return true;
         }

         return (m_notempty.waitFor(timeout) == 0);
      }

      /// On occasion we may want to signal data even if there is none. e.g. we are stopping a service
      inline void raise() {
         m_notempty.raise();
      }

      /// Clear the list, and set the length to zero, release all conditional waiters.
      inline void clear() {
         PP; 
         if (m_data.size() > 0) {
            m_data.clear(); 
            m_notempty.raise();
            m_notempty.reset();
         }
         VV;
      }

      /// Check for containment of a value in the set.
      /// THREAD-SAFE: Returns point-in-time snapshot that may be stale immediately.
      [[nodiscard]] inline bool contains(const T& value) {
         bool result;
         PP;
         result = (m_data.find(value) != m_data.end());
         VV;
         return result;
      }

      /// Remove and return an arbitrary element from the set.
      /// THREAD-SAFE: Entire operation is atomic.
      /// @throws CRException if set is empty
      [[nodiscard]] inline T remove_front() {
         T result;

         PP;
         if (m_data.empty()) {
            VV;
            CRX_THROW("cuset: remove_front, " MSG_EMPTY);
         }

         result = *m_data.begin();
         m_data.erase(m_data.begin());
         VV;

         return result;
      }

      /// add it to the set
      inline void push_back(T& data) {
         add(data);
      }

      inline void push_front(T& data) {
         add(data);
      }

      /// Remove the specified item from the set.
      /// THREAD-SAFE: Entire operation is atomic.
      /// @return 0 if nothing found, 1 if an element was removed
      /// @throws CRException if set is empty
      [[nodiscard]] inline int remove(const T& data) {
         int result;

         PP;
         if (m_data.empty()) {
            VV;
            CRX_THROW("cuset: remove, " MSG_EMPTY);
         }

         result = m_data.erase(data);
         VV;

         return result;
      }

      /// Lock the set and return current size for consistent multi-operation reads.
      /// MUST be paired with thaw() call. Recursive locks allow nested freeze/thaw.
      [[nodiscard]] inline size_t freeze() {
         PP;
         return m_data.size();
      }

      /// Unlock the set after freeze(). MUST be called to release lock.
      inline void thaw() {
         VV;
      }

      /// Move assignment operator - transfers ownership from source.
      /// THREAD-SAFE: Locks both objects, checks for self-assignment.
      inline CUSet<T>& operator=(CUSet<T>&& lst) {
         if (this == &lst) {
            return *this;  // Self-assignment check
         }

         PP;
         lst.PP;
         m_data = std::move(lst.m_data);
         lst.m_data.clear();
         lst.VV;
         VV;

         return *this;
      }

      // NOTE: begin() and end() iterator methods deliberately removed.
      // They cannot be made thread-safe without external locking:
      // - Returning iterators while holding locks would create deadlock risks
      // - Returning iterators without locks creates use-after-free vulnerabilities
      // Use freeze/thaw for consistent multi-operation reads, or copy data out.
   };
};

#endif
