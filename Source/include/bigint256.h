/** \brief  A 256-bit unsigned integer built on pair<uint128_t, uint128_t>
 *
 * \details Provides uint256_t as a specialization of pair<uint128_t, uint128_t>.
 *          All operations delegate to the 128-bit m_hi/m_lo sub-objects, which
 *          are already endian-safe via the W* macros in bigint128.h.  No raw
 *          byte/word overlay arrays are needed — there are no 256-bit intrinsics
 *          to union-pun against.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crunnable
 * \copy    Copyright (c) 2016 by Dennis Vadura, All rights reserved.
 *
 * \license You can obtain and redistribute or modify this program under the
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __BIGINT256_INC__
#define __BIGINT256_INC__

#include "bigint128.h"

#define DIV256_ZEROMSG   "BINT256: divide by zero"
#define INVALID256_INT   "BINT256: invalid unsigned integer [%s]"

namespace crunnable {

   // -----------------------------------------------------------------------------------
   // Specialization of pair<uint128_t, uint128_t> as a 256-bit unsigned integer
   // -----------------------------------------------------------------------------------
   template <>
   struct __attribute__((__packed__)) pair<uint128_t, uint128_t> {
      typedef pair<uint128_t, uint128_t> pair_t;

      typedef struct __attribute__((__packed__)) {
         uint128_t m_lo;
         uint128_t m_hi;
      } p_t;

      union __attribute__((__packed__)) {
         p_t m_dt;
      } m_u;

#define BI256_HI m_u.m_dt.m_hi
#define BI256_LO m_u.m_dt.m_lo

#define BI256_BYT_TLEN  (sizeof(uint128_t))
#define BI256_BYT_PLEN  (BI256_BYT_TLEN*2)
#define BI256_BIT_TLEN  (BI256_BYT_TLEN*8)
#define BI256_BIT_PLEN  (BI256_BYT_PLEN*8)

      // --------------------------------------------------------------------------------
      // Constructors
      // --------------------------------------------------------------------------------
      inline pair() noexcept = default;
      inline pair(const pair_t& val) noexcept = default;

      inline pair(const pair_t *val) {
         operator=(*(CRX_TIFNULL(val)));
      }

      inline pair(const char *str) {
         operator=(CRX_TIFNULL(str));
      }

      inline pair(char *str) {
         operator=((const char *) CRX_TIFNULL(str));
      }

      inline pair(const string& str) {
         operator=(str);
      }

      inline pair(const uint128_t hi, const uint128_t lo) noexcept
         : m_u{.m_dt={lo, hi}} {
      }

      // From a single uint128_t (goes into lo)
      inline pair(const uint128_t lo) noexcept
         : m_u{.m_dt={lo, uint128_t(0UL)}} {
      }

      // From integral type
      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair(const T lo) noexcept
         : m_u{.m_dt={uint128_t((uint64_t)lo), uint128_t(0UL)}} {
      }

      // From two integrals (hi64, lo64) — placed in lo128
      template <typename T, typename U, NEEDS(std::is_integral<T>() && std::is_integral<U>())>
      inline pair(const T hi, const U lo) noexcept
         : m_u{.m_dt={uint128_t((uint64_t)hi, (uint64_t)lo), uint128_t(0UL)}} {
      }


      // --------------------------------------------------------------------------------
      // Assignment operators
      // --------------------------------------------------------------------------------
      inline pair_t& operator=(const pair_t& val) noexcept = default;

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator=(const T& lo) noexcept {
         BI256_HI = 0UL;
         BI256_LO = (uint64_t)lo;
         return *this;
      }

      inline pair_t& operator=(const uint128_t& lo) noexcept {
         BI256_HI = 0UL;
         BI256_LO = lo;
         return *this;
      }

      pair_t& operator=(const char *str) {
         // Parse decimal string in chunks of 19 digits (10^19 fits in uint64_t)
         const uint64_t sc = 10000000000000000000UL;
         pair_t      psc(1UL);
         pair_t      tmp;

         char    *end, *start;
         char     buf[256];
         size_t   len;

         CRStrcpy(buf, str);
         start = CRS::trim(buf);
         len   = strlen(start);
         end   = start+len;

         *this = 0UL;
         while (len > 19) {
            start   = end-19;
            tmp     = BigIntUtil::strtoull(start);
            tmp    *= psc;
            psc    *= sc;
            *this  += tmp;
            *start  = '\0';
            end     = start;
            len    -= 19;
         }

         start  = buf;
         tmp    = BigIntUtil::strtoull(start);
         tmp   *= psc;
         *this += tmp;

         return *this;
      }

      inline pair_t& operator=(const string& str) {
         return operator=(str.c_str());
      }


      // --------------------------------------------------------------------------------
      // User defined conversion operators
      // --------------------------------------------------------------------------------
      inline operator bool() const noexcept {
         return (bool)BI256_HI || (bool)BI256_LO;
      }

      inline explicit operator uint8_t() const noexcept {
         return (uint8_t) BI256_LO;
      }

      inline explicit operator uint16_t() const noexcept {
         return (uint16_t) BI256_LO;
      }

      inline explicit operator uint32_t() const noexcept {
         return (uint32_t) BI256_LO;
      }

      inline operator uint64_t() const noexcept {
         return (uint64_t) BI256_LO;
      }

      inline operator uint128_t() const noexcept {
         return BI256_LO;
      }


      // --------------------------------------------------------------------------------
      // Named accessors
      // --------------------------------------------------------------------------------
      inline uint128_t lo128() const noexcept { return BI256_LO; }
      inline uint128_t hi128() const noexcept { return BI256_HI; }
      inline uint64_t lo64() const noexcept { return BI256_LO.lo64(); }
      inline uint64_t hi64() const noexcept { return BI256_HI.hi64(); }


      // --------------------------------------------------------------------------------
      // Logical operators
      // --------------------------------------------------------------------------------
      inline bool operator!() const noexcept {
         return !((bool)*this);
      }

      template <typename T>
      inline bool operator&&(const T& rhs) const noexcept {
         return ((bool)*this) && ((bool) rhs);
      }

      template <typename T>
      inline bool operator||(const T& rhs) const noexcept {
         return ((bool)*this) || ((bool) rhs);
      }


      // --------------------------------------------------------------------------------
      // Comparison operators: pair_t op pair_t
      // --------------------------------------------------------------------------------
      inline bool operator==(const pair_t& rhs) const noexcept {
         return (BI256_HI == rhs.BI256_HI && BI256_LO == rhs.BI256_LO);
      }

      inline bool operator!=(const pair_t& rhs) const noexcept {
         return !operator==(rhs);
      }

      inline bool operator>(const pair_t& rhs) const noexcept {
         return (BI256_HI > rhs.BI256_HI) || (BI256_HI == rhs.BI256_HI && BI256_LO > rhs.BI256_LO);
      }

      inline bool operator<(const pair_t& rhs) const noexcept {
         return (BI256_HI < rhs.BI256_HI) || (BI256_HI == rhs.BI256_HI && BI256_LO < rhs.BI256_LO);
      }

      inline bool operator>=(const pair_t& rhs) const noexcept {
         return operator>(rhs) || operator==(rhs);
      }

      inline bool operator<=(const pair_t& rhs) const noexcept {
         return operator<(rhs) || operator==(rhs);
      }

      // Comparison operators: pair_t op T (integral)
      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator==(const T rhs) const noexcept {
         return BI256_HI.isZero() && BI256_LO == (uint64_t)rhs;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator!=(const T rhs) const noexcept {
         return !operator==(rhs);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator>(const T rhs) const noexcept {
         return (bool)BI256_HI || BI256_LO > (uint64_t)rhs;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator<(const T rhs) const noexcept {
         return BI256_HI.isZero() && BI256_LO < (uint64_t)rhs;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator>=(const T rhs) const noexcept {
         return operator>(rhs) || operator==(rhs);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline bool operator<=(const T rhs) const noexcept {
         return operator<(rhs) || operator==(rhs);
      }

      // Comparison operators: pair_t op uint128_t
      inline bool operator==(const uint128_t& rhs) const noexcept {
         return BI256_HI.isZero() && BI256_LO == rhs;
      }

      inline bool operator!=(const uint128_t& rhs) const noexcept {
         return !operator==(rhs);
      }

      inline bool operator>(const uint128_t& rhs) const noexcept {
         return (bool)BI256_HI || BI256_LO > rhs;
      }

      inline bool operator<(const uint128_t& rhs) const noexcept {
         return BI256_HI.isZero() && BI256_LO < rhs;
      }

      inline bool operator>=(const uint128_t& rhs) const noexcept {
         return operator>(rhs) || operator==(rhs);
      }

      inline bool operator<=(const uint128_t& rhs) const noexcept {
         return operator<(rhs) || operator==(rhs);
      }


      // --------------------------------------------------------------------------------
      // Boolean (bitwise) operators
      // --------------------------------------------------------------------------------
      // pair_t op= pair_t
      inline pair_t& operator|=(const pair_t& rhs) noexcept {
         BI256_LO |= rhs.BI256_LO;
         BI256_HI |= rhs.BI256_HI;
         return *this;
      }

      inline pair_t& operator&=(const pair_t& rhs) noexcept {
         BI256_LO &= rhs.BI256_LO;
         BI256_HI &= rhs.BI256_HI;
         return *this;
      }

      inline pair_t& operator^=(const pair_t& rhs) noexcept {
         BI256_LO ^= rhs.BI256_LO;
         BI256_HI ^= rhs.BI256_HI;
         return *this;
      }

      // pair_t op= T
      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator|=(const T rhs) noexcept {
         BI256_LO |= (uint64_t)rhs;
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator&=(const T rhs) noexcept {
         BI256_HI = 0UL;
         BI256_LO &= (uint64_t)rhs;
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator^=(const T rhs) noexcept {
         BI256_LO ^= (uint64_t)rhs;
         return *this;
      }

      // pair_t op pair_t
      inline pair_t operator|(const pair_t& rhs) const noexcept {
         return pair_t(BI256_HI|rhs.BI256_HI, BI256_LO|rhs.BI256_LO);
      }

      inline pair_t operator&(const pair_t& rhs) const noexcept {
         return pair_t(BI256_HI&rhs.BI256_HI, BI256_LO&rhs.BI256_LO);
      }

      inline pair_t operator^(const pair_t& rhs) const noexcept {
         return pair_t(BI256_HI^rhs.BI256_HI, BI256_LO^rhs.BI256_LO);
      }

      inline pair_t operator~() const noexcept {
         return pair_t(~BI256_HI, ~BI256_LO);
      }

      // pair_t op T
      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator|(const T rhs) const noexcept {
         return pair_t(BI256_HI, BI256_LO | (uint64_t)rhs);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator&(const T rhs) const noexcept {
         return pair_t(uint128_t(0UL), BI256_LO & (uint64_t)rhs);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator^(const T rhs) const noexcept {
         return pair_t(BI256_HI, BI256_LO ^ (uint64_t)rhs);
      }


      // --------------------------------------------------------------------------------
      // Bitshift operators
      // --------------------------------------------------------------------------------
      pair_t& operator<<=(const size_t shift) noexcept {
         if (shift == 0) {
            return *this;
         }

         if (shift >= BI256_BIT_PLEN) {
            BI256_HI = 0UL;
            BI256_LO = 0UL;
         }
         else if (shift >= BI256_BIT_TLEN) {
            BI256_HI = BI256_LO << (shift - BI256_BIT_TLEN);
            BI256_LO = 0UL;
         }
         else {
            BI256_HI <<= shift;
            BI256_HI |= (BI256_LO >> (BI256_BIT_TLEN - shift));
            BI256_LO <<= shift;
         }

         return *this;
      }

      pair_t& operator>>=(const size_t shift) noexcept {
         if (shift == 0) {
            return *this;
         }

         if (shift >= BI256_BIT_PLEN) {
            BI256_HI = 0UL;
            BI256_LO = 0UL;
         }
         else if (shift >= BI256_BIT_TLEN) {
            BI256_LO = BI256_HI >> (shift - BI256_BIT_TLEN);
            BI256_HI = 0UL;
         }
         else {
            BI256_LO >>= shift;
            BI256_LO |= (BI256_HI << (BI256_BIT_TLEN - shift));
            BI256_HI >>= shift;
         }

         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator<<=(const T shift) noexcept {
         return operator<<=((size_t) shift);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator>>=(const T shift) noexcept {
         return operator>>=((size_t) shift);
      }

      inline pair_t operator<<(const size_t shift) const noexcept {
         pair_t tmp(*this);
         tmp <<= shift;
         return tmp;
      }

      inline pair_t operator>>(const size_t shift) const noexcept {
         pair_t tmp(*this);
         tmp >>= shift;
         return tmp;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator<<(const T shift) const noexcept {
         return operator<<((size_t) shift);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator>>(const T shift) const noexcept {
         return operator>>((size_t) shift);
      }


      // -------------------------------------------------------------------------------------
      // Addition operators
      // -------------------------------------------------------------------------------------
      inline pair_t operator+(const pair_t& rhs) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO + rhs.BI256_LO;
         res.BI256_HI = BI256_HI + rhs.BI256_HI + uint128_t(res.BI256_LO < BI256_LO ? 1UL : 0UL);
         return res;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator+(const T lo) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO + (uint64_t)lo;
         res.BI256_HI = BI256_HI + uint128_t(res.BI256_LO < BI256_LO ? 1UL : 0UL);
         return res;
      }

      inline pair_t operator+(const uint128_t& rhs) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO + rhs;
         res.BI256_HI = BI256_HI + uint128_t(res.BI256_LO < BI256_LO ? 1UL : 0UL);
         return res;
      }

      inline pair_t& operator+=(const pair_t& rhs) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO += rhs.BI256_LO;
         BI256_HI += rhs.BI256_HI + uint128_t(BI256_LO < tmp ? 1UL : 0UL);
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator+=(const T lo) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO += (uint64_t)lo;
         BI256_HI += uint128_t(BI256_LO < tmp ? 1UL : 0UL);
         return *this;
      }

      inline pair_t& operator+=(const uint128_t& rhs) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO += rhs;
         BI256_HI += uint128_t(BI256_LO < tmp ? 1UL : 0UL);
         return *this;
      }

      inline pair_t& operator++() noexcept {
         return (*this += 1);
      }

      inline pair_t operator++(int) noexcept {
         pair_t res(*this);
         *this += 1UL;
         return res;
      }


      // -------------------------------------------------------------------------------------
      // Subtraction operators
      // -------------------------------------------------------------------------------------
      inline pair_t operator-(const pair_t& rhs) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO - rhs.BI256_LO;
         res.BI256_HI = BI256_HI - rhs.BI256_HI - uint128_t(res.BI256_LO > BI256_LO ? 1UL : 0UL);
         return res;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator-(const T lo) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO - (uint64_t)lo;
         res.BI256_HI = BI256_HI - uint128_t(res.BI256_LO > BI256_LO ? 1UL : 0UL);
         return res;
      }

      inline pair_t operator-(const uint128_t& rhs) const noexcept {
         pair_t res;
         res.BI256_LO = BI256_LO - rhs;
         res.BI256_HI = BI256_HI - uint128_t(res.BI256_LO > BI256_LO ? 1UL : 0UL);
         return res;
      }

      inline pair_t& operator-=(const pair_t& rhs) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO -= rhs.BI256_LO;
         BI256_HI -= rhs.BI256_HI + uint128_t(BI256_LO > tmp ? 1UL : 0UL);
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator-=(const T lo) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO -= (uint64_t)lo;
         BI256_HI -= uint128_t(BI256_LO > tmp ? 1UL : 0UL);
         return *this;
      }

      inline pair_t& operator-=(const uint128_t& rhs) noexcept {
         uint128_t tmp = BI256_LO;
         BI256_LO -= rhs;
         BI256_HI -= uint128_t(BI256_LO > tmp ? 1UL : 0UL);
         return *this;
      }

      inline pair_t& operator--() noexcept {
         return (*this -= 1);
      }

      inline pair_t operator--(int) noexcept {
         pair_t res(*this);
         *this -= 1;
         return res;
      }

      inline pair_t operator+() const noexcept { return *this; }
      inline pair_t operator-() const noexcept { return ~(*this) + 1; }


      // -------------------------------------------------------------------------------------
      // Multiplication operators
      // -------------------------------------------------------------------------------------
   private:
      // Widening multiply: 128x128 -> 256 bit result
      // Uses 4 cross-products of 64-bit halves
      static pair_t mul128_widening(const uint128_t& a, const uint128_t& b) noexcept {
         uint64_t a_lo = a.lo64();
         uint64_t a_hi = a.hi64();
         uint64_t b_lo = b.lo64();
         uint64_t b_hi = b.hi64();

         // 64x64 -> 128 via compiler's unsigned __int128
         unsigned __int128 p0 = (unsigned __int128)a_lo * b_lo;
         unsigned __int128 p1 = (unsigned __int128)a_lo * b_hi;
         unsigned __int128 p2 = (unsigned __int128)a_hi * b_lo;
         unsigned __int128 p3 = (unsigned __int128)a_hi * b_hi;

         // Accumulate into 256-bit result
         // result_lo = p0 + (p1 << 64) + (p2 << 64)
         // result_hi = p3 + (p1 >> 64) + (p2 >> 64) + carry from above
         unsigned __int128 mid = p1 + (uint64_t)(p2);           // low 128 of p1+p2_lo
         unsigned __int128 carry_mid = (mid < p1) ? 1 : 0;      // carry from p1 + p2_lo

         unsigned __int128 mid_hi = (p2 >> 64) + (mid >> 64) + carry_mid; // upper part

         unsigned __int128 lo_sum = p0 + ((unsigned __int128)(uint64_t)mid << 64);
         unsigned __int128 carry_lo = (lo_sum < p0) ? 1 : 0;

         uint128_t res_lo(lo_sum);
         uint128_t res_hi(p3 + mid_hi + carry_lo);

         return pair_t(res_hi, res_lo);
      }

   public:
      pair_t operator*(const pair_t& rhs) const noexcept {
         if (isZero() || rhs.isZero()) {
            return pair_t(0UL);
         }

         if (isOne()) {
            return rhs;
         }

         if (rhs.isOne()) {
            return *this;
         }

         // result = widening(a.lo * b.lo) + (a.hi * b.lo) << 128 + (a.lo * b.hi) << 128
         pair_t result = mul128_widening(BI256_LO, rhs.BI256_LO);

         // (a.hi * b.lo) shifted by 128 — only low 128 bits of product matter (they go into result.hi)
         result.BI256_HI += BI256_HI * rhs.BI256_LO;
         result.BI256_HI += BI256_LO * rhs.BI256_HI;

         return result;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator*(const T lo) const noexcept {
         return operator*(pair_t(lo));
      }

      inline pair_t operator*(const uint128_t& rhs) const noexcept {
         return operator*(pair_t(rhs));
      }

      inline pair_t& operator*=(const pair_t& rhs) noexcept {
         *this = operator*(rhs);
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator*=(const T lo) noexcept {
         *this = operator*(pair_t(lo));
         return *this;
      }

      inline pair_t& operator*=(const uint128_t& rhs) noexcept {
         *this = operator*(pair_t(rhs));
         return *this;
      }


      // -------------------------------------------------------------------------------------
      // Division operators
      // -------------------------------------------------------------------------------------
   private:
      // Binary long-division for 256-bit values
      static pair_t rmdiv256(pair_t& x, const pair_t& y) {
         if (y.isZero()) {
            CRX_THROW(DIV256_ZEROMSG);
         }

         if (y.isOne()) {
            pair_t q = x;
            x = 0UL;
            return q;
         }

         // x < y  →  quotient=0, remainder=x
         if (x < y) {
            return pair_t(0UL);
         }

         // x == y  →  quotient=1, remainder=0
         if (x == y) {
            x = 0UL;
            return pair_t(1UL);
         }

         // Fast path: both fit in 128 bits
         if (x.isLow() && y.isLow()) {
            uint128_t q128 = x.BI256_LO / y.BI256_LO;
            x.BI256_LO = x.BI256_LO - q128 * y.BI256_LO;
            x.BI256_HI = 0UL;
            return pair_t(q128);
         }

         // Short path: divisor fits in 128 bits, dividend is > 128 bits
         // Base-2^128 long division: treat dividend as (hi128, lo128) / y128
         if (y.isLow()) {
            uint128_t y128 = y.BI256_LO;
            pair_t q(0UL);

            // q.hi = x.hi / y128
            q.BI256_HI = x.BI256_HI / y128;
            uint128_t rem = x.BI256_HI - q.BI256_HI * y128;

            // Now divide (rem << 128 | x.lo) by y128
            // This is a 256/128 division where high part is rem (< y128)
            // We use binary long division on the combined value
            if (rem.isZero()) {
               q.BI256_LO = x.BI256_LO / y128;
               x = pair_t(x.BI256_LO - q.BI256_LO * y128);
            }
            else {
               // Need to divide (rem:x.lo) by y128 — use shift-subtract on 256-bit
               pair_t num(rem, x.BI256_LO);
               pair_t div(y128);

               // Binary long division
               // shift = bit_width(num) - bit_width(div)
               size_t num_bits = 256 - num.countl_zero();
               size_t div_bits = 256 - div.countl_zero();
               size_t shift = num_bits - div_bits; // safe: num >= div since rem > 0 and lo128 present

               pair_t d = div << shift;
               pair_t qpart(0UL);

               for (size_t i = 0; i <= shift; ++i) {
                  qpart <<= 1;
                  if (num >= d) {
                     num -= d;
                     qpart |= 1UL;
                  }
                  d >>= 1;
               }

               q.BI256_LO = qpart.BI256_LO;
               x = num;
            }

            return q;
         }

         // Full path: both operands > 128 bits — binary long division
         pair_t q(0UL);
         size_t x_bits = 256 - x.countl_zero();
         size_t y_bits = 256 - y.countl_zero();

         if (x_bits < y_bits) {
            // x < y already handled above, but just in case
            return q;
         }

         size_t shift = x_bits - y_bits;
         pair_t d = y << shift;

         for (size_t i = 0; i <= shift; ++i) {
            q <<= 1;
            if (x >= d) {
               x -= d;
               q |= 1UL;
            }
            d >>= 1;
         }

         return q;
      }

   public:
      inline pair_t operator/(const pair_t& y) const {
         pair_t tmp(*this);
         return rmdiv256(tmp, y);
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator/(const T lo) const {
         return operator/(pair_t(lo));
      }

      inline pair_t operator/(const uint128_t& y) const {
         return operator/(pair_t(y));
      }

      inline pair_t& operator/=(const pair_t& y) {
         *this = rmdiv256(*this, y);
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator/=(const T lo) {
         return operator/=(pair_t(lo));
      }

      inline pair_t& operator/=(const uint128_t& y) {
         return operator/=(pair_t(y));
      }


      // -------------------------------------------------------------------------------------
      // Modulus operators
      // -------------------------------------------------------------------------------------
      inline pair_t operator%(const pair_t& y) const {
         pair_t x(*this);
         rmdiv256(x, y);
         return x;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t operator%(const T lo) const {
         return operator%(pair_t(lo));
      }

      inline pair_t operator%(const uint128_t& y) const {
         return operator%(pair_t(y));
      }

      inline pair_t& operator%=(const pair_t& y) {
         rmdiv256(*this, y);
         return *this;
      }

      template <typename T, NEEDS(std::is_integral<T>())>
      inline pair_t& operator%=(const T lo) {
         return operator%=(pair_t(lo));
      }

      inline pair_t& operator%=(const uint128_t& y) {
         return operator%=(pair_t(y));
      }


      // -------------------------------------------------------------------------------------
      // Miscellaneous convenience functions
      // -------------------------------------------------------------------------------------
      inline bool isLow() const noexcept {
         return BI256_HI.isZero();
      }

      inline bool isZero() const noexcept {
         return BI256_HI.isZero() && BI256_LO.isZero();
      }

      inline bool isOne() const noexcept {
         return BI256_HI.isZero() && BI256_LO.isOne();
      }

      inline bool isPow2() const noexcept {
         return (BI256_HI.popcount() + BI256_LO.popcount()) == 1;
      }

      inline size_t getPow2() const {
         CRX_TUNLESS(isPow2(), "BINT256: NOT Power of 2");

         if (!BI256_LO.isZero()) {
            return BI256_LO.countr_zero();
         }

         return 128 + BI256_HI.countr_zero();
      }


      // -------------------------------------------------------------------------------------
      // Bit utilities
      // -------------------------------------------------------------------------------------
      inline size_t popcount() const noexcept {
         return BI256_HI.popcount() + BI256_LO.popcount();
      }

      inline size_t countl_zero() const noexcept {
         if ((bool)BI256_HI) return BI256_HI.countl_zero();
         return 128 + BI256_LO.countl_zero();
      }

      inline size_t countr_zero() const noexcept {
         if ((bool)BI256_LO) return BI256_LO.countr_zero();
         return 128 + BI256_HI.countr_zero();
      }


      // -------------------------------------------------------------------------------------
      // String output functions
      // -------------------------------------------------------------------------------------
      std::string& toString(std::string& output) const {
         if (isZero()) {
            output = "0";
         }
         else {
            const pair_t dv(10000000000000000000UL);
            pair_t x(*this);
            char   tmp[128];
            char   buf[512];
            char   *end = buf+sizeof(buf)-1;

            *end = '\0';

            while (!x.isZero()) {
               char *start;
               char save;

               pair_t r = rmdiv256(x, dv);
               start = end - snprintf(tmp, sizeof(tmp), "%019llu", (unsigned long long)(uint64_t)x.BI256_LO);
               save = *end;
               strcpy(start, tmp);
               *end = save;
               end = start;
               x = r;
            }

            while (*end == '0') end++;

            output = end;
         }

         return output;
      }

      std::string& toHexString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }

         // Delegate to 128-bit toHexString for each half
         std::string hi_str, lo_str;

         if ((bool)BI256_HI) {
            BI256_HI.toHexString(hi_str);
            BI256_LO.toHexString(lo_str);

            // lo must be zero-padded to 32 hex digits
            while (lo_str.size() < 32) {
               lo_str = "0" + lo_str;
            }

            output = hi_str + lo_str;
         }
         else {
            BI256_LO.toHexString(output);
         }

         return output;
      }

      std::string& toOctString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }
         // Extract 3 bits at a time via shift-and-mask
         char buf[88]; // 256/3 + 2 = ~87 digits max
         char *end = buf + sizeof(buf) - 1;
         *end = '\0';
         pair_t x(*this);
         while (!x.isZero()) {
            unsigned digit = (unsigned)((uint64_t)x.BI256_LO & 7);
            *--end = (char)('0' + digit);
            x >>= 3;
         }
         output = end;
         return output;
      }

      inline friend std::ostream& operator<<(std::ostream& output, const pair_t& val) {
         std::string tmp;
         auto flags = output.flags() & std::ios::basefield;
         if (flags == std::ios::hex) {
            val.toHexString(tmp);
            if (output.flags() & std::ios::showbase) output << "0x";
         }
         else if (flags == std::ios::oct) {
            val.toOctString(tmp);
            if (output.flags() & std::ios::showbase && tmp != "0") output << "0";
         }
         else {
            val.toString(tmp);
         }
         return output << tmp;
      }

      inline friend ostream& operator<<(std::ostream& output, const pair_t* val) {
         return output << *(CRX_TIFNULL(val));
      }

#undef BI256_HI
#undef BI256_LO
#undef BI256_BYT_TLEN
#undef BI256_BYT_PLEN
#undef BI256_BIT_TLEN
#undef BI256_BIT_PLEN
   };

   typedef pair<uint128_t, uint128_t> uint256_t;


   // -------------------------------------------------------------------------------------
   // Free-standing operators: T op uint256_t and T op= uint256_t
   // -------------------------------------------------------------------------------------

   // Comparison operators
   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator==(const T lo, const uint256_t &rhs) noexcept {
      return rhs == lo;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator!=(const T lo, const uint256_t &rhs) noexcept {
      return rhs != lo;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>(const T lo, const uint256_t &rhs) noexcept {
      return rhs < lo;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<(const T lo, const uint256_t &rhs) noexcept {
      return rhs > lo;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>=(const T lo, const uint256_t &rhs) noexcept {
      return rhs <= lo;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<=(const T lo, const uint256_t &rhs) noexcept {
      return rhs >= lo;
   }

   // Compound assignment operators: T op= uint256_t
   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator+=(T& lo, const uint256_t &rhs) noexcept {
      return (lo += (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator-=(T& lo, const uint256_t &rhs) noexcept {
      return (lo -= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator*=(T& lo, const uint256_t &rhs) noexcept {
      return (lo *= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator/=(T& lo, const uint256_t &rhs) {
      return (lo /= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator%=(T& lo, const uint256_t &rhs) {
      return (lo %= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator&=(T& lo, const uint256_t &rhs) noexcept {
      return (lo &= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator|=(T& lo, const uint256_t &rhs) noexcept {
      return (lo |= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator^=(T& lo, const uint256_t &rhs) noexcept {
      return (lo ^= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator<<=(T& lo, const uint256_t &rhs) noexcept {
      return (lo <<= (T)(uint64_t) rhs);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator>>=(T& lo, const uint256_t &rhs) noexcept {
      return (lo >>= (T)(uint64_t) rhs);
   }

   // Free-standing binary operators: T op uint256_t
   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator+(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) + rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator-(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) - rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator*(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) * rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator/(const T lo, const uint256_t& rhs) {
      return uint256_t(lo) / rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator%(const T lo, const uint256_t& rhs) {
      return uint256_t(lo) % rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator&(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) & rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator|(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) | rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator^(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) ^ rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator<<(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) << (size_t)(uint64_t)rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint256_t operator>>(const T lo, const uint256_t& rhs) noexcept {
      return uint256_t(lo) >> (size_t)(uint64_t)rhs;
   }


   // -------------------------------------------------------------------------------------
   // User-defined literal: "12345..."_u256
   // -------------------------------------------------------------------------------------
   namespace literals {
      inline uint256_t operator""_u256(const char* str, size_t) {
         return uint256_t(str);
      }
   }
}


// -------------------------------------------------------------------------------------
// std::hash specialization for uint256_t
// -------------------------------------------------------------------------------------
namespace std {
   template<>
   struct hash<crunnable::uint256_t> {
      size_t operator()(const crunnable::uint256_t& v) const noexcept {
         // Use boost-style hash combining on the two 128-bit halves
         size_t seed = hash<crunnable::uint128_t>{}(v.lo128());
         seed ^= hash<crunnable::uint128_t>{}(v.hi128()) + 0x9e3779b9 + (seed<<6) + (seed>>2);
         return seed;
      }
   };


   // -------------------------------------------------------------------------------------
   // std::numeric_limits specialization for uint256_t
   // -------------------------------------------------------------------------------------
   template<>
   struct numeric_limits<crunnable::uint256_t> {
      static constexpr bool is_specialized = true;
      static constexpr bool is_signed      = false;
      static constexpr bool is_integer     = true;
      static constexpr bool is_exact       = true;
      static constexpr bool is_bounded     = true;
      static constexpr bool is_modulo      = true;
      static constexpr bool has_infinity   = false;
      static constexpr bool has_quiet_NaN  = false;
      static constexpr bool has_signaling_NaN = false;
      static constexpr bool has_denorm_loss   = false;
      static constexpr bool is_iec559         = false;
      static constexpr bool traps             = false;
      static constexpr bool tinyness_before   = false;
      static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
      static constexpr std::float_round_style  round_style = std::round_toward_zero;
      static constexpr int digits      = 256;
      static constexpr int digits10    = 77;
      static constexpr int max_digits10 = 78;
      static constexpr int radix       = 2;
      static constexpr int min_exponent    = 0;
      static constexpr int min_exponent10  = 0;
      static constexpr int max_exponent    = 0;
      static constexpr int max_exponent10  = 0;

      static crunnable::uint256_t min()     noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t max()     noexcept {
         return crunnable::uint256_t(
            crunnable::uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL),
            crunnable::uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL)
         );
      }
      static crunnable::uint256_t lowest()  noexcept { return min(); }
      static crunnable::uint256_t epsilon()       noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t round_error()   noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t infinity()      noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t quiet_NaN()     noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t signaling_NaN() noexcept { return crunnable::uint256_t(0UL); }
      static crunnable::uint256_t denorm_min()    noexcept { return crunnable::uint256_t(0UL); }
   };
}

#endif
