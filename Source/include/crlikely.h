/** CRLikely
 *
 * \brief   A simple definition for builtin likely operator.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crunnable
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 * 
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __LIKELY_INC__
#define __LIKELY_INC__

#ifdef __GNUC__
#ifndef likely
#define likely(X)       __builtin_expect(!!(X), 1)
#endif

#ifndef unlikely
#define unlikely(X)     __builtin_expect(!!(X), 0)
#endif
#else
#ifndef likely
#define likely(X) X
#endif

#ifndef unlikely
#define unlikely(X) X
#endif
#endif

#endif
