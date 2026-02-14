/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @class  AInteger
 *
 * @brief   An atomic integer
 *
 * @details This is a wrapper for a concurrent integer implementation with atomic increment
 *          and decrement operations.
 *
 *          All of the member methods are thread safe.
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
 */

#ifndef __AINTEGER_INC__
#define __AINTEGER_INC__

#include <atomic>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include "crlikely.h"

namespace crutil {
   class AInteger {
   private:
      /// The integer value
      std::atomic<int64_t> m_value;

   public:
      /// A default constructor, initializes the value to 0
      AInteger() : m_value(0)
      {
      }

      /// A constructor that initializes the value to val
      AInteger(const int64_t val) : m_value(val)
      {
      }

      /// Copy constructor — atomic types are not copy-constructible, load explicitly
      AInteger(const AInteger& other) : m_value(other.m_value.load())
      {
      }

      /// empty destructor, does not need to be virtual
      ~AInteger() = default;

      inline AInteger& operator=(const int64_t val) {
         m_value.store(val);
         return *this;
      }

      inline bool test_and_set(int64_t expected, int64_t newval) {
         return m_value.compare_exchange_strong(expected, newval);
      }

      inline AInteger& operator=(const AInteger& val) {
         m_value.store(val.m_value.load());
         return *this;
      }

      /// return the value of the integer
      inline int64_t value() const {
         return m_value.load();
      }

      /// set the value of the integer
      inline AInteger& set(const int64_t val) {
         m_value.store(val);
         return *this;
      }

      /// clamp up to floor — atomically ensures value is not below val
      inline AInteger& floor(const int64_t val) {
         int64_t cur = m_value.load();
         while (cur < val && !m_value.compare_exchange_weak(cur, val));
         return *this;
      }

      /// clamp down to ceiling — atomically ensures value is not above val
      inline AInteger& ceiling(const int64_t val) {
         int64_t cur = m_value.load();
         while (cur > val && !m_value.compare_exchange_weak(cur, val));
         return *this;
      }

      /// Sub the value delta, and return the result
      inline AInteger& sub(const int64_t delta) {
         m_value.fetch_sub(delta);
         return *this;
      }

      inline AInteger& sub(const AInteger& delta) {
         m_value.fetch_sub(delta.m_value.load());
         return *this;
      }

      /// Add the value delta, and return the result
      inline AInteger& add(const int64_t delta) {
         m_value.fetch_add(delta);
         return *this;
      }

      inline AInteger& add(const AInteger& delta) {
         m_value.fetch_add(delta.m_value.load());
         return *this;
      }

      inline uint64_t pre_add(const AInteger& delta) {
         return (uint64_t) m_value.fetch_add(delta.m_value.load());
      }

      inline uint64_t pre_sub(const AInteger& delta) {
         return (uint64_t) m_value.fetch_sub(delta.m_value.load());
      }

      uint64_t uint64() {
         return (uint64_t) m_value.load();
      }

      inline AInteger& inc() {
         return add(1);
      }

      inline AInteger& dec() {
         return sub(1);
      }

      inline bool cas(int64_t expected, int64_t val) {
         return m_value.compare_exchange_strong(expected, val);
      }

      inline int64_t gas(int64_t val) {
         return m_value.exchange(val);
      }

      inline friend std::ostream& operator<<(std::ostream& output, const AInteger& val) {
         output << val.m_value.load();
         return output;
      }

      inline friend std::ostream& operator<<(std::ostream& output, const AInteger *val) {
         if (unlikely(val == NULL)) {
            output << "NULL Atomic Integer";
         }
         else {
            output << val->m_value.load();
         }
         return output;
      }

      inline std::string& toString(std::string& output) const {
         std::stringstream ss;
         ss << m_value.load();
         output = ss.str();
         return output;
      }

      inline bool operator==(const AInteger& val) const {
         return m_value.load() == val.m_value.load();
      }

      inline bool operator!=(const AInteger& val) const {
         return m_value.load() != val.m_value.load();
      }

      inline bool operator<(const AInteger& val) const {
         return m_value.load() < val.m_value.load();
      }

      inline bool operator<=(const AInteger& val) const {
         return m_value.load() <= val.m_value.load();
      }

      inline bool operator>(const AInteger& val) const {
         return m_value.load() > val.m_value.load();
      }

      inline bool operator>=(const AInteger& val) const {
         return m_value.load() >= val.m_value.load();
      }

      template <typename T>
      inline bool operator==(const T val) const {
         return m_value.load() == val;
      }

      template <typename T>
      inline bool operator!=(const T val) const {
         return m_value.load() != val;
      }

      template <typename T>
      inline bool operator<(const T val) const {
         return static_cast<T>(m_value.load()) < val;
      }

      template <typename T>
      inline bool operator<=(const T val) const {
         return static_cast<T>(m_value.load()) <= val;
      }

      template <typename T>
      inline bool operator>(const T val) const {
         return m_value.load() > val;
      }

      template <typename T>
      inline bool operator>=(const T val) const {
         return m_value.load() >= val;
      }

      // prefix operator
      inline AInteger& operator++() {
         add(1);
         return *this;
      }

      // prefix operator
      inline AInteger& operator--() {
         sub(1);
         return *this;
      }

      // postfix operator
      inline AInteger operator++(int) {
         AInteger tmp(*this);
         add(1);
         return tmp;
      }

      // postfix operator
      inline AInteger operator--(int) {
         AInteger tmp(*this);
         sub(1);
         return tmp;
      }

      inline AInteger& operator+=(const AInteger& val) {
         add(val);
         return *this;
      }

      template <typename T>
      inline AInteger& operator+=(const T val) {
         add((int64_t)val);
         return *this;
      }

      inline AInteger& operator-=(const AInteger& val) {
         sub(val);
         return *this;
      }

      template <typename T>
      inline AInteger& operator-=(const T val) {
         sub((int64_t)val);
         return *this;
      }
   };
};

#endif
