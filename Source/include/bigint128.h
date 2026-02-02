/** \brief  An implementation of both a pair and intrinsic 128 bit unsigned integer
 *
 * \details The idea is to define uint128_t as a class type and to provide the full suite of
 *          operators. The underlying implementation is either a pair of 64bit long longs, or
 *          a single unsigned __int128, In either case all of the operations are supported as
 *          a uniform abstraction. The instrinsic based implementation is approximately 2-3x
 *          faster on division. Otherwise the ops seem to be decent in that division takes apx
 *          60-120ns for the pair based, and 60ns for the intrinsic regardless of divisor, when
 *          measured unoptimized.  Optimized results can be in the 0-40ns for both methods.
 *          Intrinsic divide seems to be 2x faster in almost all cases.
 *
 *          Best compiled output seems to be with g++ 5.3.0 with -O1+ and -mtune=nehalem, -O1
 *          is sufficient for most instances. In some cases -O3 or -O4 can double the speed
 *          of the pair based division, and almost remove all overhead of the intrinsic division.
 *          This will depend on the target CPU though, and whether or not intrinsic 128 bit ints
 *          are fully supported.
 *
 *          NOTE: This library relies on SFINAE and uses type traits to get the right method or
 *                class instance. To compile make sure you have c++11x support.
 *
 *                g++ -std=c++11 -O4 -g x.cpp ../../objs/crstring.o -lrt
 *
 *                use -lrt if you are using crtimer.h to measure things.
 *
 *          NOTE: Probably missing some conversions for some expression uses. Will add over time.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crutil
 * \copy    Copyright (c) 2016 by Dennis Vadura, All rights reserved.
 *
 * \license You can obtain and redistribute or modify this program under the
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __BIGINT128_INC__
#define __BIGINT128_INC__

#include "crstring.h"
#include "needs.h"
#include "endian.h"

#include <functional>
#include <limits>

#ifdef __SIZEOF_INT64__
#define INT64_INTRINSIC
#endif

#ifdef __SIZEOF_INT128__
#define INT128_INTRINSIC
#endif

#ifdef __SIZEOF_INT256__
#define INT256_INTRINSIC
#endif

// Compile-time index remapping: logical index 0 = least-significant word.
// On little-endian these are identity (zero overhead, identical codegen).
#if __BYTE_ORDER == __LITTLE_ENDIAN
   #define W64(i) (i)
   #define W32(i) (i)
   #define W16(i) (i)
   #define W8(i)  (i)
   static_assert(W32(0) == 0 && W32(3) == 3, "LE index sanity check");
#elif __BYTE_ORDER == __BIG_ENDIAN
   #define W64(i) (1  - (i))
   #define W32(i) (3  - (i))
   #define W16(i) (7  - (i))
   #define W8(i)  (15 - (i))
   static_assert(W32(0) == 3 && W32(3) == 0, "BE index sanity check");
#else
   #error "Unsupported byte order"
#endif

#define DIV_ZEROMSG   "BINT: divide by zero"
#define INVALID_INT   "BINT: invalid unsigned integer [%s]"

namespace crutil {
   class BigIntUtil {
      public:
      static unsigned long long strtoull(char *data) {
         unsigned long long res;

         errno = 0;
         res = ::strtoull(data, NULL, 0);

         if (unlikely(errno != 0)) {
            CRX_THROW(INVALID_INT, data);
         }
         return res;
      }
   };

   // -----------------------------------------------------------------------------------
   // Template where T is not an integral type
   // -----------------------------------------------------------------------------------
   template <typename T_hi, typename T_lo>
   struct __attribute__ ((__packed__)) pair {
      typedef pair<T_hi,T_lo> pair_t;

      // FIXME: Remove after structured pairs are supported.
      static_assert(!std::is_class<T_lo>::value || !std::is_class<T_hi>::value, "Structured pairs are not yet fully implemented");

      typedef struct __attribute__ ((__packed__)) {
         T_lo m_lo;
         T_hi m_hi;
      } p_t;

      union __attribute__ ((__packed__)) {
         p_t m_dt;
      } m_u;

#define BI_HI m_u.m_dt.m_hi
#define BI_LO m_u.m_dt.m_lo

      // --------------------------------------------------------------------------------
      // Constructors
      // --------------------------------------------------------------------------------
      pair() = default;
      pair(const pair<T_hi,T_lo>& val) = default;

      pair(const T_lo lo) {
         BI_HI = 0;
         BI_LO = lo;
      }

      pair(const T_hi hi, const T_lo lo) {
         BI_HI=hi;
         BI_LO=lo;
      }
   };


   // -----------------------------------------------------------------------------------
   // Template where T is uint64_t, this specialization is the base bigint implementation
   // -----------------------------------------------------------------------------------
   template <>
   struct __attribute__ ((__packed__)) pair<uint64_t,uint64_t> {
      typedef pair<uint64_t,uint64_t> pair_t;

      typedef struct __attribute__ ((__packed__)) {
         uint64_t m_lo;
         uint64_t m_hi;
      } p_t;

#define BI_BYT_TLEN  (sizeof(uint64_t))
#define BI_BYT_PLEN  (BI_BYT_TLEN*2)

#define BI_BIT_TLEN  (BI_BYT_TLEN*8)
#define BI_BIT_PLEN  (BI_BYT_PLEN*8)

      // sizes: p_t=16, m_u=16, m_dt=16  iff T := uint64_t
      //
      //    BI_HI=17179869187 BI_LO=8589934593
      //
      //    i=0 ub32[0]=1
      //    i=1 ub32[1]=2
      //    i=2 ub32[2]=3
      //    i=3 ub32[3]=4
      //
      //    i=0 ub64[0]=8589934593
      //    i=1 ub64[1]=17179869187

      union __attribute__ ((__packed__)) {
         uint8_t   ub8[BI_BYT_PLEN/sizeof(uint8_t)];
         uint16_t ub16[BI_BYT_PLEN/sizeof(uint16_t)];
         uint32_t ub32[BI_BYT_PLEN/sizeof(uint32_t)];
         uint64_t ub64[BI_BYT_PLEN/sizeof(uint64_t)];
         p_t      m_dt;
      } m_u;

#define BI_UB64 m_u.ub64
#define BI_UB32 m_u.ub32
#define BI_UB16 m_u.ub16
#define BI_UB8  m_u.ub8

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

         inline constexpr pair(const uint64_t hi, const uint64_t lo) noexcept
            : m_u{.m_dt={lo, hi}} {
         }

         // pair_t& = T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair(const T lo) noexcept
            : m_u{.m_dt={(uint64_t)lo, 0}} {
         }

         // pair_t& = T,U
         template <typename T, typename U, NEEDS(std::is_integral<T>() && std::is_integral<U>())>
         inline constexpr pair(const T hi, const U lo) noexcept
            : m_u{.m_dt={(uint64_t)lo, (uint64_t)hi}} {
         }


      // --------------------------------------------------------------------------------
      // Assignment operators
      // --------------------------------------------------------------------------------
         inline pair_t& operator=(const pair_t& val) noexcept = default;

         template <typename T>
         inline pair_t& operator=(const T& lo) noexcept {
            BI_HI=0;
            BI_LO=(uint64_t)lo;
            return *this;
         }

         pair_t& operator=(const char *str) {
            const uint64_t sc = 10000000000000000000UL;
            pair_t      psc(1UL);
            pair_t      tmp;

            char    *end, *start;
            char     buf[128];
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
         inline constexpr operator bool() const noexcept {
            return (BI_HI != 0) || (BI_LO != 0);
         }

         inline explicit operator uint8_t() const noexcept {
            return (uint8_t) BI_LO;
         }

         inline explicit operator uint16_t() const noexcept {
            return (uint16_t) BI_LO;
         }

         inline explicit operator uint32_t() const noexcept {
            return (uint32_t) BI_LO;
         }

         inline operator uint64_t() const noexcept {
            return (uint64_t) BI_LO;
         }

      // --------------------------------------------------------------------------------
      // Named accessors
      // --------------------------------------------------------------------------------
         inline constexpr uint64_t lo64() const noexcept { return BI_LO; }
         inline constexpr uint64_t hi64() const noexcept { return BI_HI; }


      // --------------------------------------------------------------------------------
      // Logical operators
      // --------------------------------------------------------------------------------
         inline constexpr bool operator!() const noexcept {
            return !((bool)*this);
         }

         template <typename T>
         inline constexpr bool operator&&(const T& rhs) const noexcept {
            return ((bool)*this) && ((bool) rhs);
         }

         template <typename T>
         inline constexpr bool operator||(const T& rhs) const noexcept {
            return ((bool) *this) || ((bool) rhs);
         }


      // --------------------------------------------------------------------------------
      // Comparison operators
      // --------------------------------------------------------------------------------
         // pair_t op pair_t&
         inline constexpr bool operator==(const pair_t& rhs) const noexcept {
            return (BI_HI == rhs.BI_HI && BI_LO == rhs.BI_LO);
         }

         inline constexpr bool operator!=(const pair_t& rhs) const noexcept {
            return !operator==(rhs);
         }

         inline constexpr bool operator>(const pair_t& rhs) const noexcept {
            return (BI_HI > rhs.BI_HI) || (BI_HI == rhs.BI_HI && BI_LO > rhs.BI_LO);
         }

         inline constexpr bool operator<(const pair_t& rhs) const noexcept {
            return (BI_HI == rhs.BI_HI && BI_LO < rhs.BI_LO) || (BI_HI < rhs.BI_HI);
         }

         inline constexpr bool operator>=(const pair_t& rhs) const noexcept {
            return operator>(rhs) || operator==(rhs);
         }

         inline constexpr bool operator<=(const pair_t& rhs) const noexcept {
            return operator<(rhs) || operator==(rhs);
         }

         // pair_t op T&
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator==(const T rhs) const noexcept {
            return (BI_HI == 0UL && BI_LO == (uint64_t)rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator!=(const T rhs) const noexcept {
            return !operator==(rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator>(const T rhs) const noexcept {
            return (BI_HI != 0) || (BI_LO > (uint64_t) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator<(const T rhs) const noexcept {
            return (BI_HI == 0L) && (BI_LO < (uint64_t) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator>=(const T rhs) const noexcept {
            return operator>(rhs) || operator==(rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator<=(const T rhs) const noexcept {
            return operator<(rhs) || operator==(rhs);
         }


      // --------------------------------------------------------------------------------
      // Boolean operators
      // --------------------------------------------------------------------------------
         // pair_t& op= pair_t&
         inline constexpr pair_t& operator|=(const pair_t& rhs) noexcept {
            BI_LO |= rhs.BI_LO;
            BI_HI |= rhs.BI_HI;
            return *this;
         }

         inline constexpr pair_t& operator&=(const pair_t& rhs) noexcept {
            BI_LO &= rhs.BI_LO;
            BI_HI &= rhs.BI_HI;
            return *this;
         }

         inline constexpr pair_t& operator^=(const pair_t& rhs) noexcept {
            BI_LO ^= rhs.BI_LO;
            BI_HI ^= rhs.BI_HI;
            return *this;
         }

         // pair_t& op= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator|=(const T rhs) noexcept {
            BI_LO |= rhs;
            return *this;
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator&=(const T rhs) noexcept {
            BI_HI  = 0;
            BI_LO &= rhs;
            return *this;
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator^=(const T rhs) noexcept {
            BI_LO ^= rhs;
            return *this;
         }

         // pair_t op pair_t&
         inline constexpr pair_t operator|(const pair_t& rhs) const noexcept {
            return pair_t(BI_HI|rhs.BI_HI, BI_LO|rhs.BI_LO);
         }

         inline constexpr pair_t operator&(const pair_t& rhs) const noexcept {
            return pair_t(BI_HI&rhs.BI_HI, BI_LO&rhs.BI_LO);
         }

         inline constexpr pair_t operator^(const pair_t& rhs) const noexcept {
            return pair_t(BI_HI^rhs.BI_HI, BI_LO^rhs.BI_LO);
         }

         inline constexpr pair_t operator~() const noexcept {
            return pair_t(~((uint64_t) BI_HI),~((uint64_t)BI_LO));
         }

         // pair_t op T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t operator|(const T rhs) const noexcept {
            return pair_t(BI_HI, BI_LO | rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t operator&(const T rhs) const noexcept {
            return pair_t(0, BI_LO & rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t operator^(const T rhs) const noexcept {
            return pair_t(BI_HI, BI_LO ^ rhs);
         }


      // --------------------------------------------------------------------------------
      // Bitshift operators, note that signed pair needs to override due to sign extension issues.
      // --------------------------------------------------------------------------------
         // pair_t& op= size_t
         constexpr pair_t& operator<<=(const size_t shift) noexcept {
            if (shift == 0) {
               return *this;
            }

            if (shift >= BI_BIT_PLEN) {
               BI_HI = 0;
               BI_LO = 0;
            }
            else if (shift >= BI_BIT_TLEN) {
               BI_HI = BI_LO << (shift-BI_BIT_TLEN);
               BI_LO = 0;
            }
            else {
               // split shift between hi,lo
               BI_HI <<= shift;
               BI_HI |= (BI_LO >> (BI_BIT_TLEN-shift));
               BI_LO <<= shift;
            }

            return *this;
         }

         constexpr pair_t& operator>>=(const size_t shift) noexcept {
            if (shift == 0) {
               return *this;
            }

            if (shift >= BI_BIT_PLEN) {
               BI_HI = 0;
               BI_LO = 0;
            }
            else if (shift >= BI_BIT_TLEN) {
               BI_LO = BI_HI >> (shift-BI_BIT_TLEN);
               BI_HI = 0;
            }
            else {
               BI_LO >>= shift;
               BI_LO |= (BI_HI << (BI_BIT_TLEN-shift));
               BI_HI >>= shift;
            }

            return *this;
         }

         // pair_t& op T (i.e. not uint64_t)
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator<<=(const T shift) noexcept {
            return operator<<=((size_t) shift);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator>>=(const T shift) noexcept {
            return operator>>=((size_t) shift);
         }

         // pair_t op= size_t
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

         // pair_t op T (i.e. not uint64_t)
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
         // pair_t + pair_t&
         inline pair_t operator+(const pair_t& rhs) const noexcept {
            pair_t res;

            // NOTE: do not change compiler generates code using adc in -O1+ mode
            res.BI_LO = BI_LO + rhs.BI_LO;
            res.BI_HI = BI_HI + rhs.BI_HI + (res.BI_LO < BI_LO);

            return res;
         }

         // pair_t& += T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t operator+(const T lo) const noexcept {
            pair_t res;

            // NOTE: do not change compiler generates code using adc in -O1+ mode
            res.BI_LO = BI_LO + lo;
            res.BI_HI = BI_HI + (res.BI_LO < BI_LO);

            return res;
         }

         // pair_t& += pair_t&
         inline constexpr pair_t& operator+=(const pair_t& rhs) noexcept {
            // NOTE: do not change compiler generates code using adc in -O1+ mode
            uint64_t tmp = BI_LO;
            BI_LO += rhs.BI_LO;
            BI_HI += rhs.BI_HI + (BI_LO < tmp);
            return *this;
         }

         // pair_t& += T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator+=(const T lo) noexcept {
            // NOTE: do not change compiler generates code using adc in -O1+ mode
            uint64_t tmp = BI_LO;
            BI_LO += lo;
            BI_HI += (BI_LO < tmp);
            return *this;
         }

         inline constexpr pair_t& operator++() noexcept {
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
         // pair_t - pair_t&
         inline pair_t operator-(const pair_t& rhs) const noexcept {
            pair_t res;

            // NOTE: do not change compiler usually generates code using sbb in -O1+ mode
            res.BI_LO = BI_LO - rhs.BI_LO;
            res.BI_HI = BI_HI - rhs.BI_HI - (res.BI_LO > BI_LO);

            return res;
         }

         // pair_t + T (i.e. not T and not pair_t)
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t operator-(const T lo) const noexcept {
            pair_t res;

            // NOTE: do not change compiler usually generates code using sbb in -O1+ mode
            res.BI_LO = BI_LO - lo;
            res.BI_HI = BI_HI - (res.BI_LO > BI_LO);

            return res;
         }

         // pair_t& -= pair_t&
         inline constexpr pair_t& operator-=(const pair_t& rhs) noexcept {
            uint64_t tmp = BI_LO;

            // NOTE: do not change compiler usually generates code using sbb in -O1+ mode
            BI_LO -= rhs.BI_LO;
            BI_HI -= rhs.BI_HI + (BI_LO > tmp);

            return *this;
         }

         // pair_t& -= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr pair_t& operator-=(const T lo) noexcept {
            uint64_t tmp = BI_LO;

            // NOTE: do not change compiler usually generates code using sbb in -O1+ mode
            BI_LO -= lo;
            BI_HI -= (BI_LO > tmp);

            return *this;
         }

         inline constexpr pair_t& operator--() noexcept {
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
      // perform base 2^32 multiplication on pair, throwing away any overflow
      // -------------------------------------------------------------------------------------
         pair_t operator*(const pair_t& rhs) const noexcept {
            pair_t wr(0);
            int n,t;

            // short circuit if one of the terms is zero or one.
            if (isZero() || rhs.isZero()) {
               return wr;
            }

            if (isOne() == true) {
               return rhs;
            }

            if (rhs.isOne() == true) {
               return *this;
            }

            // compute the length of each term.
            for (t=BI_BYT_PLEN/sizeof(uint32_t)-1; t >= 0 && BI_UB32[W32(t)] == 0; --t) {
            }

            for (n=BI_BYT_PLEN/sizeof(uint32_t)-1; n >= 0 && rhs.BI_UB32[W32(n)] == 0; --n) {
            }

            // compute the product, and watch for overflow
            for (int i=0; i <= t; ++i) {
               uint32_t c = 0;

               for (int j=0; j <= n; ++j) {
                  size_t ix = i+j;
                  uint64_t uv = (uint64_t)BI_UB32[W32(i)]*(uint64_t)rhs.BI_UB32[W32(j)]+c;

                  if (ix < BI_BYT_PLEN/sizeof(uint32_t)) {
                     uv += (uint64_t)wr.BI_UB32[W32(ix)];
                     wr.BI_UB32[W32(ix)] = (uv&0xffffffffUL);
                  }
                  c = (uv >> 32);
               }

               size_t ix = i+n+1;
               if (ix < BI_BYT_PLEN/sizeof(uint32_t)) {
                  wr.BI_UB32[W32(ix)] = c;
               }
               else {
                  // Intrinsic version does not throw, so neither should this.
                  // CRX_THROW("BINT: multiply overflow by (%u)", c);
               }
            }

            return wr;
         }

         // pair_t * T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t operator*(const T lo) const noexcept {
            return operator*(pair_t(lo));
         }

         // pair_t *= pair_t
         inline pair_t& operator*=(const pair_t& rhs) noexcept {
            *this = operator*(rhs);
            return *this;
         }

         // pair_t *= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t& operator*=(const T lo) noexcept {
            *this = operator*(pair_t(lo));
            return *this;
         }


      // -------------------------------------------------------------------------------------
      // Division operators
      // -------------------------------------------------------------------------------------
      // perform base 2^32 division on pair if divisor is < 2^32
      // -------------------------------------------------------------------------------------
         pair_t rmdiv32(pair_t& x, const uint32_t y) const {
            uint64_t tmp = 0L;
            uint64_t y64 = (uint64_t) y;
            pair_t q(0UL);

            // divide by 32 bits into 64 bits is easy
            if (x.isLow() == true) {
               q = x.BI_LO/y;
               x = x.BI_LO - q.BI_LO*(uint64_t)y;
               return q;
            }

            // otherwise do base 2^32 long division, on up to 4 operands
            // a b c d / y ==> (a b)/y ((a b)%y b)/y ...
            //
            q.BI_UB32[W32(3)] = (uint32_t) (x.BI_UB32[W32(3)]/y);
                         tmp = (((uint64_t)(x.BI_UB32[W32(3)]%y)) << 32) + (uint64_t) x.BI_UB32[W32(2)];

            q.BI_UB32[W32(2)] = (uint32_t) (tmp/y64);
                         tmp = ((tmp - ((uint64_t)q.BI_UB32[W32(2)])*y64) << 32) + (uint64_t) x.BI_UB32[W32(1)];

            q.BI_UB32[W32(1)] = (uint32_t) (tmp/y64);
                         tmp = ((tmp - ((uint64_t)q.BI_UB32[W32(1)])*y64) << 32) + (uint64_t) x.BI_UB32[W32(0)];

            q.BI_UB32[W32(0)] = (uint32_t) (tmp/y64);
                           x = tmp - ((uint64_t)q.BI_UB32[W32(0)])*y64;

            return q;
         }

         pair_t rmdiv(pair_t& x, const pair_t& y) const {
            int    n, t, i;
            pair_t q=x;

            if (y.isZero() == true) {
               CRX_THROW(DIV_ZEROMSG);
            }

            // divide by 1 has 0 remainder.
            if (y == 1UL) {
               x = 0UL;

               // q is result, x is remainder
               return q;
            }

            // divide by 2 is a shift
            if (y.isPow2() == true) {
               size_t shift = y.getPow2();
               pair_t mask(0xffffffffffffffffUL,0xffffffffffffffffUL);

               if (shift < BI_BIT_PLEN/2) {
                  mask.BI_LO <<= (BI_BIT_PLEN/2-shift);
                  mask.BI_LO >>= (BI_BIT_PLEN/2-shift);
                  x.BI_HI  = 0UL;
                  x.BI_LO &= mask.BI_LO;
               }
               else {
                  mask <<= (BI_BIT_PLEN-shift);
                  mask >>= (BI_BIT_PLEN-shift);
                  x &= mask;
               }

               // q is result, x is remainder
               q >>= shift;
               return q;
            }

            // Short circuit if y is only bottom 32 bits.
            if (y.isLow() == true && y.BI_UB32[W32(1)] == 0) {
               return rmdiv32(x, y.BI_UB32[W32(0)]);
            }

            // Now we need to perform full base 2^16 division on the pair.
            // Algorithm is from CRC handbook of applied cryptography.
            //
            // dividend := x = xn*b^n + xn-1 * b^n-1 ... x0 * b^0
            //  divisor := y = yt*b^t + yt-1 * b^t-1 ... y0 * b^0; n >= t > 0, yt != 0
            //
            // x := qy + r; 0 <= r < y, q > 0
            //
            // q := 0
            // while (x >= y * b^(n-t)) {q[n-t]++, x -= y * b^(n-t)}
            //
            // for i; i > t; --i do
            //    if x[i] == y[t] then
            //       q[i-t-1] = b-1
            //    else
            //       q[i-t-1] = (x[i]*b + x[i-1])/y[t]
            //    fi
            //
            //    while (q[i-t-1]*(y[t]*b -y[t-1]) > x[i]*b^2 + x[i-1]*b + x[i-2]) do
            //       --q[i-t-1];
            //    done
            //
            //    x -= q[i-t-1]*y*b^(i-t-1)
            //    if (x < 0) then
            //       x += y*b^(i-t-1)
            //       --q[i-t-1];
            //    fi
            // done
            //
            // return (q,x) # q is the result(quotient), x is the remainder
            //
            // NOTE: obvious implementation optimizations applied below, so code does not quite
            //       follow the above.

            // compute n,t using base 2^16 digits
            for (n=BI_BYT_PLEN/sizeof(uint16_t)-1; n > 1; --n) {
               if (x.BI_UB16[W16(n)] != 0) {
                  break;
               }
            }

            for (t=BI_BYT_PLEN/sizeof(uint16_t)-1; t > 1; --t) {
               if (y.BI_UB16[W16(t)] != 0) {
                  break;
               }
            }

            // if divisor is bigger than the dividend, then x is the remainder, and
            // result is 0
            q = 0UL;
            if (n < t || (n == t && x.BI_UB16[W16(n)] < y.BI_UB16[W16(t)])) {
               // q is result, x is remainder
               return q;
            }

            // Compute the initial quotient estimate,
            pair_t tr(y);
            i = n-t;

            // tr := y*b^(n-t), where b = 2^16, so y * 2^(n-t)*16, means shl (n-t)*16 bits
            if (i > 0) {
               tr <<= (i<<4);
            }

            while(x >= tr) {
               ++q.BI_UB16[W16(i)];
               x -= tr;
            }

            // Step 3, for i from n to (t+1), i in [n,t)
            for (i=n; i > t; --i) {
               int ix = i-t-1;

               if (x.BI_UB16[W16(i)] == y.BI_UB16[W16(t)]) {
                  q.BI_UB16[W16(ix)] = (uint16_t) 0xffff;
               }
               else {
                  unsigned int tmp = (unsigned int) x.BI_UB16[W16(i)];
                  tmp <<= 16;
                  tmp |= (unsigned int) x.BI_UB16[W16(i-1)];

                  q.BI_UB16[W16(ix)] = (uint16_t) (tmp / (unsigned int) y.BI_UB16[W16(t)]);
               }

               uint64_t lt = (((uint64_t)y.BI_UB16[W16(t)]) << 16) | (uint64_t) y.BI_UB16[W16(t-1)];
               uint64_t rt = (((uint64_t)x.BI_UB16[W16(i)]) << 32) | (((uint64_t)x.BI_UB16[W16(i-1)]) << 16) | ((uint64_t)x.BI_UB16[W16(i-2)]);

               while (((uint64_t) q.BI_UB16[W16(ix)])*lt > rt) {
                  --q.BI_UB16[W16(ix)];
               }

               // Note: rather than test if x < 0, we can test if subtraction term is greater than x
               //       and adjust the term appropriately before subtracting from x.
               tr = y << (ix << 4);
               pair_t ty = tr*((uint64_t)q.BI_UB16[W16(ix)]);

               if (ty > x) {
                  ty -= tr;
                  --q.BI_UB16[W16(ix)];
               }

               x -= ty;
            }

            // q is result, x is remainder.
            return q;
         }

         // pair_t / pair_t
         inline pair_t operator/(const pair_t& y) const {
            pair_t tmp(*this);
            return rmdiv(tmp, y);
         }

         // pair_t / T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t operator/(const T lo) const {
            pair_t tmp(*this);
            return (tmp / pair_t(lo));
         }

         // pair_t /= pair_t
         inline pair_t& operator/=(const pair_t& y) {
            *this = rmdiv(*this,y);
            return *this;
         }

         // pair_t / T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t& operator/=(const T lo) {
            *this /= pair_t(lo);
            return *this;
         }


      // -------------------------------------------------------------------------------------
      // Modulus operators
      // perform base 2^16 modulus on pair
      // -------------------------------------------------------------------------------------
         inline pair_t operator%(const pair_t& y) const {
            pair_t x(*this);
            rmdiv(x,y);
            return x;
         }

         // pair_t % T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t operator%(const T lo) {
            pair_t y(lo);
            return operator%(y);
         }

         inline pair_t& operator%=(const pair_t& y) {
            rmdiv(*this,y);
            return *this;
         }

         // pair_t % T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline pair_t& operator%=(const T lo) {
            pair_t y(lo);
            return operator%=(y);
         }


      // -------------------------------------------------------------------------------------
      // Miscellaneous convenience functions
      // -------------------------------------------------------------------------------------
         inline constexpr bool isLow() const noexcept {
            return (BI_HI == 0UL);
         }

         inline constexpr bool isZero() const noexcept {
            return ((BI_HI|BI_LO) == 0UL);
         }

         inline constexpr bool isOne() const noexcept {
            return isLow() && (BI_LO == 1UL);
         }

         // NOTE: uses gcc builtin's, should be fine on most compilers
         inline bool isPow2() const noexcept {
            return ((__builtin_popcountll(BI_HI)+__builtin_popcountll(BI_LO)) == 1);
         }

         inline size_t getPow2() const {
            CRX_TUNLESS(isPow2(), "BINT: NOT Power of 2");

            if (BI_LO != 0) {
               return  __builtin_ctzll(BI_LO);
            }

            return  __builtin_ctzll(BI_HI)+(sizeof(BI_LO)<<3);
         }


      // -------------------------------------------------------------------------------------
      // Bit utilities
      // -------------------------------------------------------------------------------------
         inline size_t popcount() const noexcept {
            return (size_t)__builtin_popcountll(BI_HI) + (size_t)__builtin_popcountll(BI_LO);
         }

         inline size_t countl_zero() const noexcept {
            if (BI_HI != 0) return (size_t)__builtin_clzll(BI_HI);
            if (BI_LO != 0) return 64 + (size_t)__builtin_clzll(BI_LO);
            return 128;
         }

         inline size_t countr_zero() const noexcept {
            if (BI_LO != 0) return (size_t)__builtin_ctzll(BI_LO);
            if (BI_HI != 0) return 64 + (size_t)__builtin_ctzll(BI_HI);
            return 128;
         }


      // -------------------------------------------------------------------------------------
      // String output functions
      // -------------------------------------------------------------------------------------
      std::string& toString(std::string& output) const {
         if (isZero() == true) {
            output = "0";
         }
         else {
            const pair_t dv(10000000000000000000UL);
                  pair_t x(*this);
                  char   tmp[128];
                  char   buf[512];
                  char   *end = buf+sizeof(buf)-1;

            *end = '\0';

            while(x.isZero() == false) {
               char *start;
               char save;

               pair_t r = rmdiv(x,dv);
               start = end - snprintf(tmp, sizeof(tmp), "%019llu", (unsigned long long)x.BI_LO);
               save = *end;
               strcpy(start, tmp);
               *end = save;
               end = start;
               x = r;
            }

            while (*end == '0') end++;

            output=end;
         }

         return output;
      }

      std::string& toHexString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }
         char buf[33];
         char *p = buf;
         bool started = false;
         for (int i = 3; i >= 0; --i) {
            uint32_t w = BI_UB32[W32(i)];
            if (!started && w == 0) continue;
            if (!started) {
               p += snprintf(p, (size_t)(buf + sizeof(buf) - p), "%x", w);
               started = true;
            } else {
               p += snprintf(p, (size_t)(buf + sizeof(buf) - p), "%08x", w);
            }
         }
         output = buf;
         return output;
      }

      std::string& toOctString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }
         // Extract 3 bits at a time, build right-to-left
         char buf[44]; // 128/3 + 1 = 43 digits max, +1 for NUL
         char *end = buf + sizeof(buf) - 1;
         *end = '\0';
         pair_t x(*this);
         while (!x.isZero()) {
            unsigned digit = (unsigned)(x.BI_LO & 7);
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

      std::string& toStringDigits(std::string& output, const int len, const void *varr, const char *fmt, const char *tag="dump") const {
         char buf[64];
         output = tag;
         output += ": ";

         for (int i=len; i>=0; --i) {
            if (len == 7)
               CRS::snprintf(buf, 64, fmt, ((const uint16_t *)varr)[W16(i)]);
            else
               CRS::snprintf(buf, 64, fmt, ((const uint32_t *)varr)[W32(i)]);
            output += buf;
         }

         return output;
      }

      inline std::string& toString16(std::string& output, const char *tag="ub16") const {
         return toStringDigits(output, 7, (const void *)BI_UB16, "%5lu ", tag);
      }

      inline std::string& toString16x(std::string& output, const char *tag="ub16x") const {
         return toStringDigits(output, 7, (const void *)BI_UB16, "x%04x ", tag);
      }

      inline std::string& toString32(std::string& output, const char *tag="ub32") const {
         return toStringDigits(output, 3, (const void *)BI_UB32, "%5lu ", tag);
      }

      inline std::string& toString32x(std::string& output, const char *tag="ub32x") const {
         return toStringDigits(output, 3, (const void *)BI_UB32, "x%04x ", tag);
      }
   };


   // NOTE: In order to preserve the same API, we have to create the equivalent of the non-intrinsic version
   //       but using intrinsic types when possible, and relying on the non-intrinsics for things like
   //       string assignment, toString, and the other convenience functions.
   //
   //       For the most part we use template functions, and the compiler generate the code.
   //

#ifdef INT128_INTRINSIC
   template <typename TVal>
   struct __attribute__ ((__packed__)) single {
      static_assert(std::is_same<unsigned __int128,TVal>::value, "TVal must be equal to unsigned __int128");

      typedef single<TVal> single_t;

#define BI_BYT_STLEN  (sizeof(unsigned __int128))
#define BI_BYT_SPLEN  (BI_BYT_STLEN*1)

#define BI_BIT_STLEN  (BI_BYT_STLEN*8)
#define BI_BIT_SPLEN  (BI_BYT_SPLEN*8)

      // sizes: p_t=16, m_u=16, m_dt=16  iff T := uint64_t
      //
      //    BI_HI=17179869187 BI_LO=8589934593
      //
      //    i=0 ub32[0]=1
      //    i=1 ub32[1]=2
      //    i=2 ub32[2]=3
      //    i=3 ub32[3]=4
      //
      //    i=0 ub64[0]=8589934593
      //    i=1 ub64[1]=17179869187

      union __attribute__ ((__packed__)) {
         uint8_t           ub8 [BI_BYT_SPLEN/sizeof(uint8_t)];
         uint16_t          ub16[BI_BYT_SPLEN/sizeof(uint16_t)];
         uint32_t          ub32[BI_BYT_SPLEN/sizeof(uint32_t)];
         uint64_t          ub64[BI_BYT_SPLEN/sizeof(uint64_t)];
         unsigned __int128 m_dt;
      } m_u;

#define BI_UB8   m_u.ub8
#define BI_UB16  m_u.ub16
#define BI_UB32  m_u.ub32
#define BI_UB64  m_u.ub64
#define BI_VAL   m_u.m_dt

#define UInt128t TVal

#define BI_SLO   BI_UB64[W64(0)]
#define BI_SHI   BI_UB64[W64(1)]

      // --------------------------------------------------------------------------------
      // Constructors
      // --------------------------------------------------------------------------------
         inline single() noexcept = default;

         inline single(const single_t *v) {
            operator=(*(CRX_TIFNULL(v)));
         }

         inline single(char *str) {
            operator=((const char *) CRX_TIFNULL(str));
         }

         inline single(const char *str) {
            operator=(CRX_TIFNULL(str));
         }

         inline single(const string& str) {
            operator=(str);
         }

         inline constexpr single(const UInt128t& val) noexcept
            : m_u{.m_dt = val} {}

         // single_t& = T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single(const T v) noexcept
            : m_u{.m_dt = (UInt128t) v} {}

         // single_t& = T,U
         template <typename T, typename U, NEEDS(std::is_integral<T>() && std::is_integral<U>())>
         inline constexpr single(const T hi, const U lo) noexcept
            : m_u{.m_dt = ((UInt128t)(hi & 0xffffffffffffffffUL) << 64) | (lo & 0xffffffffffffffffUL)} {}


      // --------------------------------------------------------------------------------
      // Assignment operators
      // --------------------------------------------------------------------------------
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator=(const T& lo) noexcept {
            BI_VAL=0;
            BI_VAL |= (lo & 0xffffffffffffffffUL);
            return *this;
         }

         single_t& operator=(const char *str) {
            const uint64_t sc = 10000000000000000000UL;
            single_t      psc(1UL);
            single_t      tmp;

            char    *end, *start;
            char     buf[128];
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

         inline single_t& operator=(const string& str) {
            return operator=(str.c_str());
         }

      // --------------------------------------------------------------------------------
      // User defined conversion operators
      // --------------------------------------------------------------------------------
         inline constexpr operator bool() const noexcept {
            return (BI_SHI != 0) || (BI_SLO != 0);
         }

         inline explicit operator uint8_t() const noexcept {
            return (uint8_t) BI_SLO;
         }

         inline explicit operator uint16_t() const noexcept {
            return (uint16_t) BI_SLO;
         }

         inline explicit operator uint32_t() const noexcept {
            return (uint32_t) BI_SLO;
         }

         inline operator uint64_t() const noexcept {
            return (uint64_t) BI_SLO;
         }

      // --------------------------------------------------------------------------------
      // Named accessors
      // --------------------------------------------------------------------------------
         inline constexpr uint64_t lo64() const noexcept { return (uint64_t)(BI_VAL); }
         inline constexpr uint64_t hi64() const noexcept { return (uint64_t)(BI_VAL >> 64); }


      // --------------------------------------------------------------------------------
      // Logical operators
      // --------------------------------------------------------------------------------
         inline constexpr bool operator!() const noexcept {
            return !((bool)*this);
         }

         template <typename T>
         inline constexpr bool operator&&(const T& rhs) const noexcept {
            return ((bool)*this) && ((bool) rhs);
         }

         template <typename T>
         inline constexpr bool operator||(const T& rhs) const noexcept {
            return ((bool) *this) || ((bool) rhs);
         }


      // --------------------------------------------------------------------------------
      // Comparison operators
      // --------------------------------------------------------------------------------
         // single_t op single_t&
         inline constexpr bool operator==(const single_t& rhs) const noexcept {
            return (BI_VAL == rhs.BI_VAL);
         }

         inline constexpr bool operator!=(const single_t& rhs) const noexcept {
            return (BI_VAL != rhs.BI_VAL);
         }

         inline constexpr bool operator>(const single_t& rhs) const noexcept {
            return (BI_VAL > rhs.BI_VAL);
         }

         inline constexpr bool operator<(const single_t& rhs) const noexcept {
            return (BI_VAL < rhs.BI_VAL);
         }

         inline constexpr bool operator>=(const single_t& rhs) const noexcept {
            return (BI_VAL >= rhs.BI_VAL);
         }

         inline constexpr bool operator<=(const single_t& rhs) const noexcept {
            return (BI_VAL <= rhs.BI_VAL);
         }

         // single_t op T&
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator==(const T rhs) const noexcept {
            return (BI_VAL == (unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator!=(const T rhs) const noexcept {
            return (BI_VAL != (unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator>(const T rhs) const noexcept {
            return (BI_VAL > (unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator<(const T rhs) const noexcept {
            return (BI_VAL < (unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator>=(const T rhs) const noexcept {
            return (BI_VAL >= (unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr bool operator<=(const T rhs) const noexcept {
            return (BI_VAL <= (unsigned __int128) rhs);
         }


      // --------------------------------------------------------------------------------
      // Boolean operators
      // --------------------------------------------------------------------------------
         // single_t& op= single_t&
         inline constexpr single_t& operator|=(const single_t& rhs) noexcept {
            BI_VAL |= rhs.BI_VAL;
            return *this;
         }

         inline constexpr single_t& operator&=(const single_t& rhs) noexcept {
            BI_VAL &= rhs.BI_VAL;
            return *this;
         }

         inline constexpr single_t& operator^=(const single_t& rhs) noexcept {
            BI_VAL ^= rhs.BI_VAL;
            return *this;
         }

         // single_t& op= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator|=(const T rhs) noexcept {
            BI_VAL |= (unsigned __int128) rhs;
            return *this;
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator&=(const T rhs) noexcept {
            BI_VAL &= (unsigned __int128) rhs;
            return *this;
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator^=(const T rhs) noexcept {
            BI_VAL ^= (unsigned __int128) rhs;
            return *this;
         }

         // single_t op single_t&
         inline constexpr single_t operator|(const single_t& rhs) const noexcept {
            return single_t(BI_VAL|rhs.BI_VAL);
         }

         inline constexpr single_t operator&(const single_t& rhs) const noexcept {
            return single_t(BI_VAL&rhs.BI_VAL);
         }

         inline constexpr single_t operator^(const single_t& rhs) const noexcept {
            return single_t(BI_VAL^rhs.BI_VAL);
         }

         inline constexpr single_t operator~() const noexcept {
            return single_t(~BI_VAL);
         }

         // single_t op T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator|(const T rhs) const noexcept {
            return single_t(BI_VAL|(unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator&(const T rhs) const noexcept {
            return single_t(BI_VAL&(unsigned __int128) rhs);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator^(const T rhs) const noexcept {
            return single_t(BI_VAL^(unsigned __int128) rhs);
         }


      // --------------------------------------------------------------------------------
      // Bitshift operators, note that signed pair needs to override due to sign extension issues.
      // --------------------------------------------------------------------------------
         // single_t& op= size_t
         inline constexpr single_t& operator<<=(const size_t shift) noexcept {
            if (shift == 0) {
               return *this;
            }

            if (shift >= BI_BIT_PLEN) {
               BI_VAL = 0;
               return *this;
            }

            BI_VAL <<= shift;
            return *this;
         }

         inline constexpr single_t& operator>>=(const size_t shift) noexcept {
            if (shift == 0) {
               return *this;
            }

            if (shift >= BI_BIT_PLEN) {
               BI_VAL = 0;
               return *this;
            }

            BI_VAL >>= shift;
            return *this;
         }

         // single_t& op T (i.e. not uint64_t)
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator<<=(const T shift) noexcept {
            return operator<<=((size_t) shift);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator>>=(const T shift) noexcept {
            return operator>>=((size_t) shift);
         }

         // single_t op= size_t
         inline constexpr single_t operator<<(const size_t shift) const noexcept {
            single_t tmp(*this);
            tmp <<= shift;
            return tmp;
         }

         inline constexpr single_t operator>>(const size_t shift) const noexcept {
            single_t tmp(*this);
            tmp >>= shift;
            return tmp;
         }

         // single_t op T (i.e. not uint64_t)
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator<<(const T shift) const noexcept {
            return operator<<((size_t) shift);
         }

         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator>>(const T shift) const noexcept {
            return operator>>((size_t) shift);
         }


      // -------------------------------------------------------------------------------------
      // Addition operators
      // -------------------------------------------------------------------------------------
         // single_t + single_t&
         inline constexpr single_t operator+(const single_t& rhs) const noexcept {
            return BI_VAL + rhs.BI_VAL;
         }

         // single_t& += T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator+(const T lo) const noexcept {
            return BI_VAL + (unsigned __int128) lo;
         }

         // single_t& += single_t&
         inline constexpr single_t& operator+=(const single_t& rhs) noexcept {
            BI_VAL += rhs.BI_VAL;
            return *this;
         }

         // single_t& += T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator+=(const T lo) noexcept {
            BI_VAL += (unsigned __int128) lo;
            return *this;
         }

         inline constexpr single_t& operator++() noexcept {
            ++BI_VAL;
            return *this;
         }

         inline constexpr single_t operator++(int) noexcept {
            single_t res(*this);
            ++BI_VAL;
            return res;
         }


      // -------------------------------------------------------------------------------------
      // Subtraction operators
      // -------------------------------------------------------------------------------------
         // single_t - single_t&
         inline constexpr single_t operator-(const single_t& rhs) const noexcept {
            return BI_VAL - rhs.BI_VAL;
         }

         // single_t + T (i.e. not T and not single_t)
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator-(const T lo) const noexcept {
            return BI_VAL - (unsigned __int128) lo;
         }

         // single_t& -= single_t&
         inline constexpr single_t& operator-=(const single_t& rhs) noexcept {
            BI_VAL -= rhs.BI_VAL;
            return *this;
         }

         // single_t& -= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator-=(const T lo) noexcept {
            BI_VAL -= (unsigned __int128) lo;
            return *this;
         }

         inline constexpr single_t& operator--() noexcept {
            --BI_VAL;
            return *this;
         }

         inline constexpr single_t operator--(int) noexcept {
            single_t res(*this);
            --BI_VAL;
            return res;
         }

         inline constexpr single_t operator+() const noexcept { return *this; }
         inline constexpr single_t operator-() const noexcept { return ~(*this) + 1; }


      // -------------------------------------------------------------------------------------
      // Multiplication operators
      // perform base 2^32 multiplication on pair, throwing away any overflow
      // -------------------------------------------------------------------------------------
         inline constexpr single_t operator*(const single_t& rhs) const noexcept {
            return BI_VAL * rhs.BI_VAL;
         }

         // single_t * T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t operator*(const T lo) const noexcept {
            return BI_VAL * (unsigned __int128) lo;
         }

         // single_t *= single_t
         inline constexpr single_t& operator*=(const single_t& rhs) noexcept {
            BI_VAL *= rhs.BI_VAL;
            return *this;
         }

         // single_t *= T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline constexpr single_t& operator*=(const T lo) noexcept {
            BI_VAL *= (unsigned __int128) lo;
            return *this;
         }


      // -------------------------------------------------------------------------------------
      // Division operators
      // -------------------------------------------------------------------------------------
         single_t rmdiv32(single_t& x, const uint32_t y) const {
            single_t q(x);

            CRX_TIF((y==0), DIV_ZEROMSG);
            q /= y;
            x -= q*y;
            return q;
         }

         single_t rmdiv(single_t& x, const single_t& y) const {
            single_t q(x);

            CRX_TIF(y.isZero(), DIV_ZEROMSG);
            q /= y;
            x -= q*y;
            return q;
         }

         // single_t / single_t
         inline single_t operator/(const single_t& y) const {
            CRX_TIF(y.isZero(), DIV_ZEROMSG);
            return BI_VAL/y.BI_VAL;
         }

         // single_t / T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline single_t operator/(const T y) const {
            CRX_TIF((y==0), DIV_ZEROMSG);
            return BI_VAL/y;
         }

         // single_t /= single_t
         inline single_t& operator/=(const single_t& y) {
            CRX_TIF(y.isZero(), DIV_ZEROMSG);
            BI_VAL /= y.BI_VAL;
            return *this;
         }

         // single_t / T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline single_t& operator/=(const T y) {
            CRX_TIF((y==0), DIV_ZEROMSG);
            BI_VAL /= y;
            return *this;
         }


      // -------------------------------------------------------------------------------------
      // Modulus operators
      // perform base 2^16 modulus on pair
      // -------------------------------------------------------------------------------------
         inline single_t operator%(const single_t& y) const {
            CRX_TIF(y.isZero(), DIV_ZEROMSG);
            return BI_VAL % y.BI_VAL;
         }

         // single_t % T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline single_t operator%(const T y) const {
            CRX_TIF((y==0), DIV_ZEROMSG);
            return BI_VAL % (unsigned __int128) y;
         }

         inline single_t& operator%=(const single_t& y) {
            CRX_TIF(y.isZero(), DIV_ZEROMSG);
            BI_VAL %= y.BI_VAL;
            return *this;
         }

         // single_t % T
         template <typename T, NEEDS(std::is_integral<T>())>
         inline single_t& operator%=(const T y) {
            CRX_TIF((y==0), DIV_ZEROMSG);
            BI_VAL %= (unsigned __int128) y;
            return *this;
         }


      // -------------------------------------------------------------------------------------
      // Miscellaneous convenience functions
      // -------------------------------------------------------------------------------------
         inline constexpr bool isLow() const noexcept {
            return (hi64() == 0UL);
         }

         inline constexpr bool isZero() const noexcept {
            return (BI_VAL == 0);
         }

         inline constexpr bool isOne() const noexcept {
            return (BI_VAL == 1);
         }

         // NOTE: uses gcc builtin's, should be fine on most compilers
         inline bool isPow2() const noexcept {
            return ((__builtin_popcountll(lo64())+__builtin_popcountll(hi64())) == 1);
         }

         inline size_t getPow2() const {
            CRX_TUNLESS(isPow2(), "BINT: NOT Power of 2");

            if (lo64() != 0) {
               return  __builtin_ctzll(lo64());
            }

            return  __builtin_ctzll(hi64())+64;
         }


      // -------------------------------------------------------------------------------------
      // Bit utilities
      // -------------------------------------------------------------------------------------
         inline size_t popcount() const noexcept {
            return (size_t)__builtin_popcountll(hi64()) + (size_t)__builtin_popcountll(lo64());
         }

         inline size_t countl_zero() const noexcept {
            if (hi64() != 0) return (size_t)__builtin_clzll(hi64());
            if (lo64() != 0) return 64 + (size_t)__builtin_clzll(lo64());
            return 128;
         }

         inline size_t countr_zero() const noexcept {
            if (lo64() != 0) return (size_t)__builtin_ctzll(lo64());
            if (hi64() != 0) return 64 + (size_t)__builtin_ctzll(hi64());
            return 128;
         }


      // -------------------------------------------------------------------------------------
      // String output functions
      // -------------------------------------------------------------------------------------
      std::string& toString(std::string& output) const {
         if (isZero() == true) {
            output = "0";
         }
         else {
            const single_t dv(10000000000000000000UL);
                  single_t x(*this);
                  char     tmp[128];
                  char     buf[512];
                  char     *end = buf+sizeof(buf)-1;

            *end = '\0';

            while(x.isZero() == false) {
               char *start;
               char save;
               single_t r;

               r.BI_VAL = x.BI_VAL/dv.BI_VAL;
               x.BI_VAL = x.BI_VAL-(r.BI_VAL*dv.BI_VAL);
               snprintf(tmp, sizeof(tmp), "%019llu", (unsigned long long)x.BI_SLO);
               size_t len = strlen(tmp);

               start = end-len;
               save = *end;
               strcpy(start, tmp);
               *end = save;
               end = start;
               x = r;
            }

            while (*end == '0') end++;

            output=end;
         }

         return output;
      }

      std::string& toHexString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }
         char buf[33];
         char *p = buf;
         bool started = false;
         for (int i = 3; i >= 0; --i) {
            uint32_t w = BI_UB32[W32(i)];
            if (!started && w == 0) continue;
            if (!started) {
               p += snprintf(p, (size_t)(buf + sizeof(buf) - p), "%x", w);
               started = true;
            } else {
               p += snprintf(p, (size_t)(buf + sizeof(buf) - p), "%08x", w);
            }
         }
         output = buf;
         return output;
      }

      std::string& toOctString(std::string& output) const {
         if (isZero()) {
            output = "0";
            return output;
         }
         char buf[44];
         char *end = buf + sizeof(buf) - 1;
         *end = '\0';
         single_t x(*this);
         while (!x.isZero()) {
            unsigned digit = (unsigned)(x.BI_SLO & 7);
            *--end = (char)('0' + digit);
            x >>= 3;
         }
         output = end;
         return output;
      }

      inline friend std::ostream& operator<<(std::ostream& output, const single_t& val) {
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

      inline friend ostream& operator<<(std::ostream& output, const single_t* val) {
         return output << *(CRX_TIFNULL(val));
      }

      std::string& toStringDigits(std::string& output, const int len, const void *varr, const char *fmt, const char *tag="dump") const {
         char buf[64];
         output = tag;
         output += ": ";

         for (int i=len; i>=0; --i) {
            if (len == 7)
               CRS::snprintf(buf, 64, fmt, ((const uint16_t *)varr)[W16(i)]);
            else
               CRS::snprintf(buf, 64, fmt, ((const uint32_t *)varr)[W32(i)]);
            output += buf;
         }

         return output;
      }

      inline std::string& toString16(std::string& output, const char *tag="ub16") const {
         return toStringDigits(output, 7, (const void *)BI_UB16, "%5lu ", tag);
      }

      inline std::string& toString16x(std::string& output, const char *tag="ub16x") const {
         return toStringDigits(output, 7, (const void *)BI_UB16, "x%04x ", tag);
      }

      inline std::string& toString32(std::string& output, const char *tag="ub32") const {
         return toStringDigits(output, 3, (const void *)BI_UB32, "%5lu ", tag);
      }

      inline std::string& toString32x(std::string& output, const char *tag="ub32x") const {
         return toStringDigits(output, 3, (const void *)BI_UB32, "x%04x ", tag);
      }
   };
#endif /* INT128_INTRINSIC */

   // Define a pair based uint128_t implementation in all compilation targets
   typedef pair<uint64_t,uint64_t>   uint128p_t;

   // Define a more efficient intrinsic implementation if 128 bit ints are supported.
   // The intrinsic implementation is between 2x and 3x faster for division, depending
   // on how much work needs to be done.  Both are fairly fast though. A typical division
   // takes between 60 and 120ns for the pair based implementation, and apx 60ns for the
   // intrinsic implementation. (Measured on 3.5Ghz CPU).

#ifdef INT128_INTRINSIC
   typedef single<unsigned __int128> uint128_t;
#else
   typedef uint128p_t                uint128_t;
#endif /* INT128_INTRINSIC */

   // These operators provide templates for cases where the LHS term is non uint128p_t type.
   // By way of explanation, A + B -> is converted to A.operator+(B). If A is not a uint128p_t then
   // the conversion does not work. As a result the two term versions of the operators are required
   // as defined here.

   // Comparison operators
   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator==(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI == 0L) && (lo == rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI == 0L) && (lo > rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>=(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI == 0L) && (lo >= rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI != 0) || ((uint64_t) lo < rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<=(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI != 0) || (lo <= rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator!=(const T lo, const uint128p_t &rhs) noexcept {
      return (rhs.BI_HI != 0) || (lo != rhs.BI_LO);
   }

   // Compound assignment operators: T op= uint128p_t
   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator&=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo &= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator^=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo ^= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator|=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo |= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator+=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo += (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator-=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo -= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator<<=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo <<= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator>>=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo >>= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator*=(T& lo, const uint128p_t &rhs) noexcept {
      return (lo *= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator/=(T& lo, const uint128p_t &rhs) {
      return (lo /= (T) rhs.BI_LO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator%=(T& lo, const uint128p_t &rhs) {
      return (lo %= (T) rhs.BI_LO);
   }

   // Free-standing binary operators: T op uint128p_t
   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator+(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) + rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator-(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) - rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator*(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) * rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator/(const T lo, const uint128p_t& rhs) {
      return uint128p_t(lo) / rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator%(const T lo, const uint128p_t& rhs) {
      return uint128p_t(lo) % rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator&(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) & rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator|(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) | rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator^(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) ^ rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator<<(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) << (size_t)(uint64_t)rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128p_t operator>>(const T lo, const uint128p_t& rhs) noexcept {
      return uint128p_t(lo) >> (size_t)(uint64_t)rhs;
   }

#ifdef INT128_INTRINSIC
   // Comparison operators
   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator==(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo == rhs.BI_VAL);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo > rhs.BI_VAL);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator>=(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo >= rhs.BI_VAL);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo < rhs.BI_VAL);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator<=(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo <= rhs.BI_VAL);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   bool operator!=(const T lo, const uint128_t &rhs) noexcept {
      return ((unsigned __int128) lo != rhs.BI_VAL);
   }

   // Compound assignment operators: T op= uint128_t
   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator&=(T& lo, const uint128_t &rhs) noexcept {
      return (lo &= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator^=(T& lo, const uint128_t &rhs) noexcept {
      return (lo ^= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator|=(T& lo, const uint128_t &rhs) noexcept {
      return (lo |= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator+=(T& lo, const uint128_t &rhs) noexcept {
      return (lo += (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator-=(T& lo, const uint128_t &rhs) noexcept {
      return (lo -= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator<<=(T& lo, const uint128_t &rhs) noexcept {
      return (lo <<= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator>>=(T& lo, const uint128_t &rhs) noexcept {
      return (lo >>= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator*=(T& lo, const uint128_t &rhs) noexcept {
      return (lo *= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator/=(T& lo, const uint128_t &rhs) {
      return (lo /= (T) rhs.BI_SLO);
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   T& operator%=(T& lo, const uint128_t &rhs) {
      return (lo %= (T) rhs.BI_SLO);
   }

   // Free-standing binary operators: T op uint128_t
   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator+(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) + rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator-(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) - rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator*(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) * rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator/(const T lo, const uint128_t& rhs) {
      return uint128_t(lo) / rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator%(const T lo, const uint128_t& rhs) {
      return uint128_t(lo) % rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator&(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) & rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator|(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) | rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator^(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) ^ rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator<<(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) << (size_t)(uint64_t)rhs;
   }

   template <typename T, NEEDS(std::is_integral<T>())>
   uint128_t operator>>(const T lo, const uint128_t& rhs) noexcept {
      return uint128_t(lo) >> (size_t)(uint64_t)rhs;
   }

#endif /* INT128_INTRINSIC */

   // -------------------------------------------------------------------------------------
   // User-defined literal: "12345678901234567890"_u128
   // -------------------------------------------------------------------------------------
   namespace literals {
      inline uint128_t operator""_u128(const char* str, size_t) {
         return uint128_t(str);
      }
   }
}

// -------------------------------------------------------------------------------------
// std::hash specialization
// -------------------------------------------------------------------------------------
namespace std {
   template<>
   struct hash<crutil::uint128p_t> {
      size_t operator()(const crutil::uint128p_t& v) const noexcept {
         size_t seed = hash<uint64_t>{}(v.BI_LO);
         seed ^= hash<uint64_t>{}(v.BI_HI) + 0x9e3779b9 + (seed<<6) + (seed>>2);
         return seed;
      }
   };

#ifdef INT128_INTRINSIC
   template<>
   struct hash<crutil::uint128_t> {
      size_t operator()(const crutil::uint128_t& v) const noexcept {
         size_t seed = hash<uint64_t>{}(v.BI_SLO);
         seed ^= hash<uint64_t>{}(v.BI_SHI) + 0x9e3779b9 + (seed<<6) + (seed>>2);
         return seed;
      }
   };
#endif

   // -------------------------------------------------------------------------------------
   // std::numeric_limits specialization
   // -------------------------------------------------------------------------------------
   template<>
   struct numeric_limits<crutil::uint128p_t> {
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
      static constexpr int digits      = 128;
      static constexpr int digits10    = 38;
      static constexpr int max_digits10 = 39;
      static constexpr int radix       = 2;
      static constexpr int min_exponent    = 0;
      static constexpr int min_exponent10  = 0;
      static constexpr int max_exponent    = 0;
      static constexpr int max_exponent10  = 0;

      static constexpr crutil::uint128p_t min()     noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t max()     noexcept { return crutil::uint128p_t(0xffffffffffffffffUL, 0xffffffffffffffffUL); }
      static constexpr crutil::uint128p_t lowest()  noexcept { return min(); }
      static constexpr crutil::uint128p_t epsilon()       noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t round_error()   noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t infinity()      noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t quiet_NaN()     noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t signaling_NaN() noexcept { return crutil::uint128p_t(0UL); }
      static constexpr crutil::uint128p_t denorm_min()    noexcept { return crutil::uint128p_t(0UL); }
   };

#ifdef INT128_INTRINSIC
   template<>
   struct numeric_limits<crutil::uint128_t> {
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
      static constexpr int digits      = 128;
      static constexpr int digits10    = 38;
      static constexpr int max_digits10 = 39;
      static constexpr int radix       = 2;
      static constexpr int min_exponent    = 0;
      static constexpr int min_exponent10  = 0;
      static constexpr int max_exponent    = 0;
      static constexpr int max_exponent10  = 0;

      static constexpr crutil::uint128_t min()     noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t max()     noexcept { return crutil::uint128_t(0xffffffffffffffffUL, 0xffffffffffffffffUL); }
      static constexpr crutil::uint128_t lowest()  noexcept { return min(); }
      static constexpr crutil::uint128_t epsilon()       noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t round_error()   noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t infinity()      noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t quiet_NaN()     noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t signaling_NaN() noexcept { return crutil::uint128_t(0UL); }
      static constexpr crutil::uint128_t denorm_min()    noexcept { return crutil::uint128_t(0UL); }
   };
#endif
}

#endif
