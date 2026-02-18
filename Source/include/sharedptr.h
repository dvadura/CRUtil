/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @class  SharedPtr
 *
 * @brief   A simple wrapper on std::shared_ptr with null-dereference protection
 *
 * @details A safer alternative to std::shared_ptr that throws exceptions when
 *          dereferencing null pointers instead of causing undefined behavior.
 *          Prevents default construction to avoid uninitialized shared pointers.
 *
 * @par Key Features:
 *          - Throws CRException on null dereference (via CRX_TIFNULL)
 *          - No default constructor - forces explicit initialization
 *          - No constructor from raw T* - use crutil::make_shared<T>() instead
 *          - Compatible with std::shared_ptr (can be constructed from it)
 *
 * @par Example Usage:
 * @code
 *    using namespace crutil;
 *
 *    // Create a SharedPtr using make_shared
 *    auto ptr = make_shared<MyClass>(arg1, arg2);
 *
 *    // Safe to dereference - throws if null
 *    ptr->method();    // Safe: throws CRException if ptr is null
 *    *ptr = value;     // Safe: throws CRException if ptr is null
 *
 *    // Convert from std::shared_ptr
 *    std::shared_ptr<MyClass> stdPtr = std::make_shared<MyClass>();
 *    SharedPtr<MyClass> safePtr(stdPtr);
 *
 *    // This will throw CRException instead of segfault:
 *    SharedPtr<MyClass> nullPtr(std::shared_ptr<MyClass>());
 *    nullPtr->method();  // Throws: "crtypes.h:35, ptr is NULL"
 * @endcode
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
 */

#ifndef __SHAREDPTR_INC__
#define __SHAREDPTR_INC__

#include "crexception.h"

namespace crutil {
   template <typename T>
   class SharedPtr : public std::shared_ptr<T> {
      public:
         typedef std::shared_ptr<T>  stdptr_t;

         // deliberately no constructor from T*, use make_shared<T>
         inline SharedPtr(const SharedPtr<T>& ptr) : stdptr_t(ptr) {
         }

         inline SharedPtr(const stdptr_t& ptr) : stdptr_t(ptr) {
         }

         // Move constructor
         inline SharedPtr(SharedPtr<T>&& ptr) noexcept : stdptr_t(std::move(ptr)) {
         }

         inline SharedPtr(stdptr_t&& ptr) noexcept : stdptr_t(std::move(ptr)) {
         }

         ~SharedPtr() {}
         
         // throw an exception if we try to deref a null pointer, ie. catch it early.
         T* operator->() const {
            return CRX_TIFNULL(this->get());
         }

         T& operator*() const {
            return *CRX_TIFNULL(this->get());
         }
   };

   template< class T, class... Args >
   SharedPtr<T> make_shared( Args&&... args) {
      return SharedPtr<T>(std::make_shared<T>(std::forward<Args>(args)...));
   }
}
#endif
