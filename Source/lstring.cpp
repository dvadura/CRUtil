/** \class  lstring
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crunnable
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 * 
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#include "lstring.h"
#include "crstring.h"

using namespace crunnable;

constexpr std::map<int,lstring<15>> SMAP = {
   { 1, Obfuscate::encode("this is a test") }
};
