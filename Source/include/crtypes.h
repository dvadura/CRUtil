/** CRTypes
 *
 * \brief   A common location to define types used by CRunnable.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crunnable
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 * 
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __CRTYPES_INC__
#define __CRTYPES_INC__

#include "endian.h"

#if defined(__linux__) || defined(__CYGWIN__)
#   include <sys/epoll.h>
#   include <sys/prctl.h>
#   include <sys/inotify.h>
#elif defined(__APPLE__)
#   include <sys/event.h>
#endif

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <libgen.h>
#include <syslog.h>
#include <stdint.h>
#include <limits.h>
#include <pthread.h>
#include <ifaddrs.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include <sys/un.h>

#ifdef HASTERM
#   include <term.h>
#   include <curses.h>
#endif

#ifdef HAS_BYTESWAP
#   include <bits/byteswap.h>
#endif

#ifdef __i386__
#   include <asm/posix_types_32.h>
#endif

#include <set>
#include <map>
#include <list>
#include <deque>
#include <string>
#include <memory>
#include <sstream>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <system_error>
#include <unordered_map>

#define USEASM 1

#define MS_IN_ONE_SEC     ((uint64_t) 1000L)
#define US_IN_ONE_MSEC    ((uint64_t) 1000L)
#define NS_IN_ONE_USEC    ((uint64_t) 1000L)

#define US_IN_ONE_SEC     ((uint64_t) (MS_IN_ONE_SEC * US_IN_ONE_MSEC))

#define NS_IN_ONE_MSEC    ((uint64_t) (US_IN_ONE_MSEC * NS_IN_ONE_USEC))
#define NS_IN_HALF_SEC    ((uint64_t) (NS_IN_ONE_MSEC * 500))

#define NS_IN_ONE_SEC     ((uint64_t) (US_IN_ONE_SEC * NS_IN_ONE_USEC))
#define NS_IN_TWO_SEC     ((uint64_t) (NS_IN_ONE_SEC  * 2))
#define NS_IN_FIVE_SEC    ((uint64_t) (NS_IN_ONE_SEC  * 5))

#define SC_IN_ONE_MIN     ((uint64_t) 60)
#define NS_IN_ONE_MIN     ((uint64_t) (NS_IN_ONE_SEC  * SC_IN_ONE_MIN))
#define US_IN_ONE_MIN     ((uint64_t) NS_IN_ONE_MIN/1000)
#define MS_IN_ONE_MIN     ((uint64_t) US_IN_ONE_MIN/1000)

// Timespec is in sec/nsec
typedef struct timespec ts_t;

// Timeval is in sec/usec
typedef struct timeval  tv_t;

// Make sure that we have a definition of number of log facilities
#ifndef LOG_NFACILITIES
#   define LOG_NFACILITIES 24
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#   define htonll(x) (x)
#   define ntohll(x) (x)
#else
#   ifndef htonll
#      define htonll htobe64
#   endif
#   ifndef ntohll
#      define ntohll be64toh
#   endif
#endif
#endif
