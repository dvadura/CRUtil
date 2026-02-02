/** \class  lstring
 *
 * \brief   Compile-time string obfuscation via XOR encoding.
 *
 * \details String literals are XOR-encoded at compile time using Obfuscate::encode(),
 *          producing an lstring<N> whose data array contains only the encoded bytes.
 *          The original plaintext never appears in the binary. At runtime, call
 *          Obfuscate::decode() to recover the original string into a caller-supplied
 *          std::string.
 *
 *          Usage:
 *
 *             // Encode at compile time -- plaintext is never stored in the binary.
 *             constexpr auto secret = Obfuscate::encode("my secret string");
 *
 *             // Decode at runtime -- caller controls allocation of the result.
 *             std::string plain;
 *             Obfuscate::decode(secret.data, plain);
 *
 *          Encoded strings can also be stored in constexpr containers:
 *
 *             constexpr std::map<int, lstring<15>> SMAP = {
 *                { 1, Obfuscate::encode("this is a test") }
 *             };
 *
 *          Based on:
 *
 *             https://sourceforge.net/p/constexprstr/code/HEAD/tree/main.cpp#l25
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crutil
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 * 
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __LSTRING_INC__
#define __LSTRING_INC__

#include "crtypes.h"

namespace crutil {
   // Forward declaration (needed by Obfuscate::encode)
   template <std::size_t N> struct lstring;

   struct Obfuscate {
      // make sure that there are 256 entries
      static constexpr unsigned char ob[256] = {
         0x67, 0xc6, 0x69, 0x73, 0x51, 0xff, 0x4a, 0xec,    0x29, 0xcd, 0xba, 0xab, 0xf2, 0xfb, 0xe3, 0x46, 
         0x7c, 0xc2, 0x54, 0xf8, 0x1b, 0xe8, 0xe7, 0x8d,    0x76, 0x5a, 0x2e, 0x63, 0x33, 0x9f, 0xc9, 0x9a, 
         0x66, 0x32, 0x0d, 0xb7, 0x31, 0x58, 0xa3, 0x5a,    0x25, 0x5d, 0x05, 0x17, 0x58, 0xe9, 0x5e, 0xd4, 
         0xab, 0xb2, 0xcd, 0xc6, 0x9b, 0xb4, 0x54, 0x11,    0x0e, 0x82, 0x74, 0x41, 0x21, 0x3d, 0xdc, 0x87, 
         0x70, 0xe9, 0x3e, 0xa1, 0x41, 0xe1, 0xfc, 0x67,    0x3e, 0x01, 0x7e, 0x97, 0xea, 0xdc, 0x6b, 0x96, 
         0x8f, 0x38, 0x5c, 0x2a, 0xec, 0xb0, 0x3b, 0xfb,    0x32, 0xaf, 0x3c, 0x54, 0xec, 0x18, 0xdb, 0x5c, 
         0x02, 0x1a, 0xfe, 0x43, 0xfb, 0xfa, 0xaa, 0x3a,    0xfb, 0x29, 0xd1, 0xe6, 0x05, 0x3c, 0x7c, 0x94, 
         0x75, 0xd8, 0xbe, 0x61, 0x89, 0xf9, 0x5c, 0xbb,    0xa8, 0x99, 0x0f, 0x95, 0xb1, 0xeb, 0xf1, 0xb3, 
         0x05, 0xef, 0xf7, 0x00, 0xe9, 0xa1, 0x3a, 0xe5,    0xca, 0x0b, 0xcb, 0xd0, 0x48, 0x47, 0x64, 0xbd, 
         0x1f, 0x23, 0x1e, 0xa8, 0x1c, 0x7b, 0x64, 0xc5,    0x14, 0x73, 0x5a, 0xc5, 0x5e, 0x4b, 0x79, 0x63, 
         0x3b, 0x70, 0x64, 0x24, 0x11, 0x9e, 0x09, 0xdc,    0xaa, 0xd4, 0xac, 0xf2, 0x1b, 0x10, 0xaf, 0x3b, 
         0x33, 0xcd, 0xe3, 0x50, 0x48, 0x47, 0x15, 0x5c,    0xbb, 0x6f, 0x22, 0x19, 0xba, 0x9b, 0x7d, 0xf5, 
         0x0b, 0xe1, 0x1a, 0x1c, 0x7f, 0x23, 0xf8, 0x29,    0xf8, 0xa4, 0x1b, 0x13, 0xb5, 0xca, 0x4e, 0xe8, 
         0x98, 0x32, 0x38, 0xe0, 0x79, 0x4d, 0x3d, 0x34,    0xbc, 0x5f, 0x4e, 0x77, 0xfa, 0xcb, 0x6c, 0x05, 
         0xac, 0x86, 0x21, 0x2b, 0xaa, 0x1a, 0x55, 0xa2,    0xbe, 0x70, 0xb5, 0x73, 0x3b, 0x04, 0x5c, 0xd3, 
         0x36, 0x94, 0xb3, 0xaf, 0xe2, 0xf0, 0xe4, 0x9e,    0x4f, 0x32, 0x15, 0x49, 0xfd, 0x82, 0x4e, 0xa9, 
      };

      constexpr Obfuscate() = default;

      constexpr char operator() (const char c, const size_t i) const {
         return c ^ ob[i & 0xff];
      }

      template <std::size_t N>
      static std::string& decode(const char (&enc)[N], std::string& result) {
         constexpr Obfuscate o;
         result.resize(N - 1);
         for (std::size_t i = 0; i < N - 1; ++i) {
            result[i] = o(enc[i], i);
         }
         return result;
      }

      // encode() defined after lstring (forward dependency)
      template <int N>
      static constexpr lstring<N> encode(const char (&s)[N]);
   };


   // Define ilist as a type parameterized by a list of integers.
   // http://loungecpp.wikidot.com/tips-and-tricks%3aindices -- slightly modified
   // So a ilist<0> will define ilist<0>::next to be ilist<0,1> and so on.
   template <std::size_t... L> 
      struct ilist {
         using next = ilist<L..., sizeof...(L)>;
      };

   // to build a an index list of size N, i.e. ilist<0,1,...,N>, we define it recursively as
   // ie. append<3>::type => append<2>::type ::next
   //                     => append<1>::type ::next ::next
   //                     => append<0>::type ::next ::next ::next
   //                     -- ilist<>      ::next ::next ::next
   //                     <= ilist<0>     ::next ::next
   //                     <= ilist<0,1>   ::next
   //                     <= ilist<0,1,2>
   template <std::size_t N>
      struct append {
         using type = typename append<N-1>::type::next;
      };

   // define the base case for the list, ilist<>
   template <>
      struct append<0> {
         using type = ilist<>;
      };

   // Now make it easy to get the list
   template <std::size_t N>
      using IList = typename append<N>::type;


   // Now create a container for the string data
   template <int M, typename T=char> struct sdata {
      const T data[M];

      template <typename TT=T, std::size_t... I>
         constexpr
            sdata(const TT (&v)[M], ilist<I...>) : data{ Obfuscate()(v[I], I)... } {}
   };

   // define lstring as a type parameterized by its length
   template <std::size_t N>
      struct lstring {
         const char data[N];

         ~lstring() = default;

         template <std::size_t... I>
            constexpr
               lstring(const char (&v)[N], ilist<I...>) : data{ Obfuscate()(v[I], I)... } {}

         //constexpr
         //   lstring(const lstring<N>& s) : str(&s.str.data, IList<N>) {}
      };

   // Out-of-line definition of Obfuscate::encode (requires lstring to be complete)
   template <int N>
      constexpr
         lstring<N> Obfuscate::encode(const char (&s)[N]) {
            return lstring<N>(s, IList<N>{});
         }

   // Legacy alias
   template <int N>
      constexpr
         lstring<N> cstr(const char (&s)[N]) {
            return Obfuscate::encode(s);
         }

};
#endif
