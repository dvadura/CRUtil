/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** CRLikely
 *
 * @brief   A simple definition for builtin likely operator.
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
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
