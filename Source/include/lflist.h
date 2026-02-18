/** \class  LFList
 *
 * \brief   A concurrent lock-free list
 *
 * \details This is a wrapper for a concurrent lock-free list implementation that doesn't suck.
 *          The implementation also provides a conditional that can be used to wait for the
 *          list not being empty.  This is very handy when implementing in-memory queues.
 * 
 *          All of the member methods are thread safe.
 *         
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crunnable
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 * 
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __LFLIST_INC__
#define __LFLIST_INC__

#include "tbb/concurrent_queue.h"
#include "condition.h"
#include "ilflist.h"

// Undefine system macro from sys/queue.h to avoid conflict
#ifdef LIST_EMPTY
#undef LIST_EMPTY
#endif

#define LIST_EMPTY "list is empty"

namespace crutil {
   template <class T>
   class LFList : public ILFList<T> {
   private:
      /// The underlying STL list that actually stores the data.
      tbb::concurrent_queue<T> m_data;

      /// A condition variable that is used to wait for the list being non-empty.
      /// It is a broadcast conditional
      Condition m_notempty;

   public:
      /// A default constructor, initializes the underlying list and sets the length to 0.
      LFList(const char* tag=NULL) : m_notempty(tag, true)
      {
         m_data.clear();
      }

      /// A constructor that places a single item on the list.
      LFList(const T& item, const char* tag=NULL) : m_notempty(tag, true)
      {
         m_data.push(item);
         m_notempty.raise();
      }

      /// A constructor that inializes based on moving contents of list
      LFList(LFList<T>&& list, const char* tag=NULL) : m_data(std::move(list.m_data)), m_notempty(tag, true)
      {
         if (m_data.empty() == false) {
            m_notempty.raise();
         }
      }

      /// A destructor, empties the list and resets the conditional.
      virtual ~LFList()      		 
      {
         // free any waiters
         m_notempty.raise();
         m_data.clear(); 
      }

      /// Return the current length of the list.
      inline virtual size_t size()
      {
         return m_data.unsafe_size();
      }

      /// Test if the list is currently empty.
      inline virtual bool empty()
      {
         return m_data.empty();
      }

      // empty, but return info if not empty. To be compat with rqlist
      inline virtual bool empty(uint64_t *r)
      {
         *r = 0;
         return m_data.empty();
      }


      /// Remove the first element on the list and return it.
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      inline virtual T remove(bool* success, const bool throwe=true) {
         T result;

         *success = m_data.try_pop(result);

         if (unlikely(*success == false)) {
            CRX_TIF_ERR(throwe, -1, "lflist: remove_front, " LIST_EMPTY);
         }

         return result;
      }

      inline virtual T remove_front(bool* success, const bool throwe=true) {
         return remove(success, throwe);
      }

      /// Remove the first element on the list, return it, and push it to the end
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      inline virtual T pfpb(const bool throwe=true) {
         bool success;
         T item = remove_front(&success, throwe);
         if (success == true) {
            m_data.push(item);
         }

         return item;
      }

      /// Push an element on to the front of the list
      inline virtual void push(const T& item, const bool raise=true) {
         m_data.push(item); 

         if (likely(raise == true)) {
            m_notempty.raise(); 
         }

         return;
      }

      inline virtual bool push_back(const T& item, const bool raise=true) {
         push(item, raise);
         return(true);
      }

      /// Wait for the list to become non-empty, or timeout 
      /// If timeout is 0, then wait indefinitely, return false if we timedout.
      /// throws exception on error.
      inline virtual bool waitFor(const uint64_t timeout=0) {
         // First if we are called and we have data, we always return
         if (m_data.empty() == false) {
            return true;
         }

         return (m_notempty.waitFor(timeout) == 0);
      }

      /// On occasion we may want to signal data even if there is none. e.g. we are stopping a service
      inline virtual void raise() {
         m_notempty.raise();
      }

      /// Clear the list, and set the length to zero, release all conditional waiters.
      inline virtual void clear() {
         m_data.clear();
         m_notempty.raise();
         m_notempty.reset();
      }
   };
};

#endif
