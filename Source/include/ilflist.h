/** @file ilflist.h
 *  @brief A concurrent lock-free list interface
 *
 *  @class ILFList
 *  @brief Interface for concurrent lock-free list implementations
 *
 *  @details This is the base interface for lock-free list implementations.
 *           Concrete implementations must provide thread-safe operations
 *           for multi-threaded access patterns.
 *
 *  @tparam T Element type (must be copyable)
 *
 *  @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 *  @see     https://github.com/dvadura/CRUtil
 *  @copyright Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 *
 *  @license You can obtain and redistribute or modify this program under the
 *           terms of the Software License Agreement provided in the file:
 *           <distribution-root>/LICENSE-APACHE-2.0.txt
 */

#ifndef __ILFLIST_INC__
#define __ILFLIST_INC__

namespace crutil {
   template <class T>
   class ILFList {
   public:
      /// A default constructor, initializes the underlying list and sets the length to 0.
      ILFList() {}

      /// A destructor, empties the list and resets the conditional.
      ~ILFList() {}

      /// Return the current length of the list.
      virtual size_t size() = 0;

      /// Test if the list is currently empty.
      virtual bool empty() = 0;

      /// Remove the first element on the list and return it.
      /// NOTE: it uses copy constructor, which is fine for lists of pointers
      virtual T remove_front(bool* success, const bool throwe=true) = 0;

      /// Add element T to the end
      virtual bool push_back(const T& item, const bool raise=true) = 0;

      /// Wait for the list to become non-empty, or timeout 
      /// If timeout is 0, then wait indefinitely, return false if we timedout.
      /// throws exception on error.
      virtual bool waitFor(const uint64_t timeout=0) = 0;

      /// On occasion we may want to signal data even if there is none. e.g. we are stopping a service
      virtual void raise() = 0;

      /// Clear the list, and set the length to zero, release all conditional waiters.
      virtual void clear() = 0;
   };
};

#endif
