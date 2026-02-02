/** \class   C++ template needs manifests
 *  
 *  \brief   A set of typedefs to make it easier to constrain c++11 templates
 *
 *  \details This is a convenient wrapper on the lovely and cryptic C++11 type_traits
 *           and friends. It's based on discussion in:
 *           
 *           http://pfultz2.com/blog/2014/11/08/non-template-constraints/
 *           and
 *           http://pfultz2.com/blog/2014/08/17/type-requirements/
 *           and
 *           http://ericniebler.com/2013/11/23/concept-checking-in-c11/
 *           and
 *           http://eli.thegreenplace.net/2014/sfinae-and-enable_if/
 *
 *           This stuff is not straight forward, but it is useful. I'm sure
 *           that it can be made even more interesting by the use of Concepts,
 *           but I've not done that here yet.
 *
 *  \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 *  \see     http://www.vadura.eu/crutil
 *  \copy    Copyright (c) 2016 by Dennis Vadura, All rights reserved.
 *
 *  \license You can obtain and redistribute or modify this program under the 
 *           terms of the Software License Agreement Provided in the file:
 *           <distribution-root>/LICENSE.txt
 */

#ifndef __NEEDS_INC__
#define __NEEDS_INC__

#include <type_traits>

/* 
 * By way of explanation (from http://pfultz2.com/blog/2014/11/08/non-template-constraints/), in order to
 * constrain a type or function overload so that SFINAE can eliminate it if the constraint does not match
 * you need to ensure that type expression is unique for each overload instance. The trick is to:
 *
 * a) have a local referenced type (dummy) so that in the event the constraint is not based on a type
 *    parameter (e.g. NEEDS(sizeof(int) == 4)) it still works.
 *
 * b) uses enumtag to generate unique types on the fly based on __LINE__ so that different constrained
 *    expressions for the same signature also work; for example:
 *
 *       template<NEEDS(sizeof(int) == 4)>
 *       void serialize(int n);
 *
 *       template<NEEDS(sizeof(int) == 8)>
 *       void serialize(int n);
 *
 *    works as expected, and also the following where T & U are template parameters that are used:
 *
 *       template <typename T, NEEDS(std::is_integral<T>())>
 *       pair(const T lo) {
 *       }
 *
 *       template <typename T, typename U, NEEDS(std::is_integral<T>() && std::is_integral<U>())>
 *       pair(const T hi, const U lo) {
 *       }
 *
 *    also works as expected.
 */

template<long N>
struct enumtag {
   enum class type {
      none,
      all
   };
};

#define NEEDS(...)     enumtag<__LINE__>::type = enumtag<__LINE__>::type::none, bool dummy=true, std::enable_if_t<dummy && (__VA_ARGS__), int> = 0

// A simpler version when you don't have duped signatures
#define REQUIRES(...)  std::enable_if_t<(__VA_ARGS__), int> = 0
#endif
