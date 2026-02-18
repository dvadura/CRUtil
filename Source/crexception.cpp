/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE-APACHE-2.0.txt
 */

/**
 * @file   crexception.cpp
 *
 * @brief  Static variable initialization and debug trace functions for CRException
 *
 * @details This file contains the static member variable initializers for the CRException
 *          class used for thread cancellation tracking, as well as the optional SEMTRACE
 *          debug functions (semtraceadd, semtracedump) for debugging semaphore/mutex usage.
 */

#include "crexception.h"
#include "crtimer.h"
#ifdef SEMTRACE
#include "semaphore.h"
#endif

using namespace crutil;

// CRException static member initializers for thread cancellation tracking
std::map<pid_t,bool> CRException::s_tmap;
pthread_mutex_t      CRException::s_tlock = PTHREAD_MUTEX_INITIALIZER;

#ifdef SEMTRACE
// Global trace ring buffer for debugging semaphore operations
Trace TRACE[1024*1024];
int   ti = 0;

/**
 * @brief Add an entry to the semaphore trace ring buffer
 *
 * @param ptr    Pointer to the semaphore being traced
 * @param porv   true for P (acquire), false for V (release)
 * @param cost   Operation cost in nanoseconds
 * @param where  Call-site string (file:line or function name)
 *
 * Records timestamp, thread ID, semaphore pointer, operation type, cost, and location
 * for post-mortem debugging of deadlocks and performance issues.
 */
void crutil::semtraceadd(void* ptr, bool porv, uint64_t cost, const char* where) {
   int i = ti++;
   if (i > 1024*1024) {
      return;
   }
   CRTime now;

   TRACE[i].now = now.t2ns();
   TRACE[i].tid = CRX_GETTID();
   TRACE[i].sem = ptr;
   TRACE[i].porv = porv;
   TRACE[i].cost = cost;
   strncpy(TRACE[i].where, where, 222);
   TRACE[i].where[222] = '\0';
}

/**
 * @brief Dump the last 20,000 entries from the trace ring buffer
 *
 * @param out  Output file stream (default: stderr)
 *
 * Prints trace entries in reverse chronological order showing timestamp,
 * thread ID, semaphore address, P/V operation, cost, and call site.
 */
void crutil::semtracedump(FILE* out) {
   int limit = 20000;

   for (int i=ti-1; i >= 0 && limit > 0; --i,--limit) {
      fprintf(out, "ts=%10lu tid=%08d sem=0x%08lx %1d %-8lu %s\n",
            TRACE[i].now,
            TRACE[i].tid,
            (uint64_t) TRACE[i].sem,
            TRACE[i].porv,
            TRACE[i].cost,
            TRACE[i].where);
   }
}
#endif
