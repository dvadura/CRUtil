/** @file   clist.h
 *  @brief  Thread-safe concurrent list/queue implementation
 *
 *  Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 *  Licensed under terms in <distribution-root>/LICENSE-APACHE-2.0.txt
 */

/** @class  CList
 *
 *  @brief   A thread-safe concurrent list/queue
 *
 *  @details Thread-safe wrapper around std::deque with condition variable
 *           support for producer-consumer patterns. All operations are O(1)
 *           and fully thread-safe.
 *
 *           The implementation provides a condition variable that can be used
 *           to wait for the list becoming non-empty, making it ideal for
 *           implementing in-memory queues and producer-consumer patterns.
 *
 *  @warning This class returns values by copy to ensure thread safety.
 *           Methods like front() and back() are safe but may be expensive
 *           for large objects. Consider using pointers or shared_ptr<T>.
 *
 *           Random access (operator[]) and iterators (begin/end) have been
 *           intentionally removed as they cannot be made thread-safe without
 *           external locking that would defeat the purpose of this class.
 *
 *  @tparam T The type of elements stored in the list. Should be copyable.
 *
 *  @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 *  @see     https://github.com/dvadura/CRUtil
 */

#ifndef __CLIST_INC__
#define __CLIST_INC__

#include "condition.h"
#include <deque>
#include <algorithm>

#define CLIST_EMPTY "list is empty"

namespace crutil {
   template <class T>
   class CList : protected Semaphore {
   private:
      /// The underlying STL list that actually stores the data.
      std::deque<T> m_data;

      /// A condition variable that is used to wait for the list being non-empty.
      /// It is a broadcast conditional
      Condition m_notempty;

   public:
      /// A default constructor, initializes the underlying list and sets the length to 0.
      CList(const char* tag=NULL) : Semaphore(true, false), m_notempty(tag, true)
      {
         m_data.clear();
      }

      /// A constructor that places a single item on the list.
      CList(const T& item, const char* tag=NULL) : Semaphore(true, false), m_notempty(tag, true)
      {
         m_data.push_back(item);
         m_notempty.raise();
      }

      /// A constructor that places a single item on the list.
      CList(CList<T>&& list, const char* tag=NULL) : Semaphore(true, false), m_data(std::move(list.m_data)), m_notempty(tag, true)
      {
         if (m_data.empty() == false) {
            m_notempty.raise();
         }
      }

      /// A destructor, empties the list and resets the conditional.
      virtual ~CList()      		 
      {
         PP;
         m_data.clear(); 
         m_notempty.raise();
         VV;
      }

      /// Return the current length of the list.
      [[nodiscard]] inline size_t size()
      {
         size_t result;

         PP;
         result = m_data.size();
         VV;

         return result;
      }

      /// Test if the list is currently empty.
      [[nodiscard]] inline bool empty()
      {
         bool result;
         PP;
         result = m_data.empty();
         VV;
         return result;
      }

      /// Return the first element in the list, do not remove the element.
      /// Returns a copy for thread safety (no dangling references).
      [[nodiscard]] inline T front() {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: front, " CLIST_EMPTY);
         }

         T result = m_data.front();
         VV;

         return result;
      }

      /// Return the last element in the list, do not remove the element.
      /// Returns a copy for thread safety (no dangling references).
      [[nodiscard]] inline T back() {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: back, " CLIST_EMPTY);
         }

         T result = m_data.back();
         VV;

         return result;
      }

      /// Remove the first element form the list
      inline void pop_front() {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: pop_front, " CLIST_EMPTY);
         }
         m_data.pop_front();
         VV;

         return;
      }

      /// Remove the last element form the list
      inline void pop_back() {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: pop_back, " CLIST_EMPTY);
         }
         m_data.pop_back();
         VV;

         return;
      }

      /// Remove the first element on the list and return it.
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      [[nodiscard]] inline T remove_front(const bool throwe=true) {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_TIF(throwe, "clist: remove_front, " CLIST_EMPTY);
            errno = -1;
            return T{};
         }

         T result = m_data.front();
         m_data.pop_front();
         errno = 0;
         VV;

         return result;
      }

      /// Remove the last element on the list and return it.
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      [[nodiscard]] inline T remove_back(const bool throwe=true) {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_TIF(throwe, "clist: remove_back, " CLIST_EMPTY);
            errno = -1;
            return T{};
         }

         T result = m_data.back();
         m_data.pop_back();
         errno = 0;
         VV;

         return result;
      }

      /// Remove the first element on the list, return it, and push it to the end
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      [[nodiscard]] inline T pfpb() {
         PP;
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: pfpb, " CLIST_EMPTY);
         }

         T result = m_data.front();
         m_data.pop_front();
         m_data.push_back(result);
         VV;

         return result;
      }

      /// Push an element on to the front of the list
      inline void push_front(const T& item, const bool raise=true) {
         PP; 
         m_data.push_front(item); 
         if (likely(raise == true)) {
            m_notempty.raise(); 
         }
         VV; 

         return;
      }

      /// Push an element on to the end of the list, return the new length, as a result
      /// of the push.
      inline void push_back(const T& item, const bool raise=true) {
         PP; 
         m_data.push_back(item);  
         if (likely(raise == true)) {
            m_notempty.raise(); 
         }
         VV; 

         return;
      }

      /// A trivial wrapper on push_back
      inline void add(const T& item) {
         push_back(item);
         return;
      }

      /// A trivial wrapper on splice
      inline unsigned int add(CList<T>&& lst) {
         return splice(std::move(lst));
      }

      /// Push an element on to the end of the list, return the new length, as a result
      /// of the push.
      inline unsigned int splice(CList<T>&& lst) {
         if (lst.empty() == true) {
            return m_data.size();
         }

         PP; 
         bool raise = m_data.empty();

         lst.PP;
         for (typename std::deque<T>::iterator it = lst.m_data.begin(); it != lst.m_data.end(); ++it) {
            m_data.push_back(std::move(*it));
         }

         lst.m_data.clear(); 
         lst.VV;

         if (raise == true) {
            m_notempty.raise(); 
         }

         size_t result = m_data.size();
         VV; 

         return result;
      }

      /// Wait for the list to become non-empty, or timeout
      /// If timeout is 0, then wait indefinitely, return false if we timedout.
      /// throws exception on error.
      [[nodiscard]] inline bool waitFor(const uint64_t timeout=0) {
         // First if we are called and we have data, we always return
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

      /// remove the listed item from the list and adjust our length, return the new length.
      inline unsigned int remove(const T& data) {
         PP; 
         if (m_data.empty() == true) {
            VV;
            CRX_THROW("clist: remove, " CLIST_EMPTY);
         }

         m_data.erase(std::remove(m_data.begin(), m_data.end(), data), m_data.end());
         size_t result = m_data.size();
         VV;

         return result;
      }

      inline size_t freeze() {
         PP;
         return m_data.size();
      }

      inline void thaw() {
         VV;
      }

      inline void fit() {
         PP;
         m_data.shrink_to_fit();
         VV;
      }

      inline CList<T>& operator=(CList<T>&& lst) {
         PP;
         lst.PP;
         m_data = std::move(lst.m_data);
         lst.m_data.clear();
         lst.m_notempty.raise();
         lst.m_notempty.reset();
         lst.VV;
         VV;

         return *this;
      }
   };
};

#endif
