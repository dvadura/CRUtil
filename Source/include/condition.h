/** \class  Condition
 *
 * \brief   A simple condition variable based on pthread_cond
 *
 * \details Uses an underlying semaphore to implement a set of waiters on a
 *          condition. waitFor can be called with a nanosecond resolution timeout
 *          value.  The value is used to set a relative timeout from now and attempts
 *          to compensate for the cost of setting up the waitFor, so sub 5ns timeouts
 *          are meaningless.
 *
 * \par     COND_DEBUG
 *
 *          When COND_DEBUG is defined (it is defined by default in the header),
 *          each Condition instance gains a m_debug flag controllable via
 *          setDebug(true).  When enabled, CO_DEBUG() calls emit detailed
 *          fprintf traces to stderr showing thread id, broadcast/normal mode,
 *          fired count, waiter count, and timeout for every waitFor/raise
 *          cycle.  This is particularly useful for diagnosing missed signals,
 *          spurious wakeups, and ordering issues in multi-threaded code.
 *          Set dflag=true in the constructor, or call setDebug(true) at
 *          runtime to activate.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crutil
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 *
 * \license You can obtain and redistribute or modify this program under the
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __CONDITION_INC__
#define __CONDITION_INC__

#include "semaphore.h"
#include "crtimer.h"
#include "crstring.h"

#define COND_DEBUG 
#ifdef COND_DEBUG
#define CO_DEBUG(MSG,...) if (m_debug) {fprintf(stderr, MSG, ## __VA_ARGS__ );}
#else
#define CO_DEBUG(MSG,...)
#endif

extern "C" {
   typedef void cancel_fn_t(void *);
}

namespace crutil {
   class Condition {
   private:
      /// semaphore used to protect access
      Semaphore m_sem;

      /// The underlying pthread's condition variable that we wrap
      pthread_cond_t m_condition;

      /// A local flag indicating if the conditional is currently disabled
      volatile bool m_enabled;

      /// A local flag telling us how many times we were fired.
      volatile int m_fired;

      /// A local count of how many clients are waiting for condition to be true.
      volatile int m_waiters;

      /// A local flag indicating if raising the condition signals or broadcasts
      volatile bool m_broadcast;

#ifdef COND_DEBUG
      /// A debug flag for debugging conditionals, when conditionally compiled in
      /// via COND_DEBUG being defined
      volatile bool m_debug;
#endif
      
      /// Release any thread waiting on the conditional and destroy the conditional.
      void disable()
      {
         // check if already disabled.
         if (unlikely(m_enabled == false)) {
            return;
         }

         // ignore any errors from raise at this point
         // it's really just there to clear out any waiters.
         try {
            CRException::notifyCancel(CRX_GETTID());
            raise(100);
            CRException::clearCancel(CRX_GETTID());
         }
         catch (CRException& ignore) {
         }

         m_sem.PP;
         // check one more time, another thread could have won the race.
         if (unlikely(m_enabled == false)) {
            m_sem.VV;
            return;
         }

         // Mark as disabled.
         m_enabled = false;
         m_waiters = 0;
         m_fired = 0;
         m_sem.VV;

         int result;
         if (unlikely((result=pthread_cond_destroy(&m_condition)) != 0)) {
            CRX_THROW_ERR(result, "COND: destroy failed");
         }
         ::memset(&m_condition, '\0', sizeof(pthread_cond_t));
      }

      /// Initialize the conditional.
      void enable()
      {
         m_sem.PP;
         if (unlikely(m_enabled == true)) {
            m_sem.VV;
            return;
         }

         int result;
         if (unlikely((result=pthread_cond_init(&m_condition, NULL)) != 0)) {
            m_sem.VV;
            CRX_THROW_ERR(result, "COND: init failed");
         }

         m_enabled = true;
         m_waiters = 0;
         m_fired = 0;
         m_sem.VV;
      }

      inline int do_wait(const uint64_t nsec_timeout)
      {
         int result;

         ++m_waiters;
#if (defined(_GNU_SOURCE) && !defined(ANDROID))
         CO_DEBUG("COND(%d)[%s,%c]: Wait (m_fired=%d,m_waiters=%d,to==%lu)\n", CRX_GETTID(),
                  m_sem.getVerbose(),(m_broadcast?'B':'N'),m_fired,m_waiters,nsec_timeout);
#endif

         m_sem.cv(this);
         if (likely(nsec_timeout == 0)) {
            result = pthread_cond_wait(&m_condition, &m_sem.getSEM());
         }
         else {
            CRTime now(nsec_timeout);
            result = pthread_cond_timedwait(&m_condition, &m_sem.getSEM(), &now.ts());
         }
         m_sem.cp(this);

         m_waiters = (m_broadcast == true) ? 0 : m_waiters-1;

         return result;
      }

      // the semaphore is held when this method runs
      static void threadCancel(void* arg)
      {
         if (arg == NULL) {
            return;
         }

         Condition* self = (Condition*) arg;

         // we are about to be canceled, calls pthread_exit(PTHREAD_CANCELLED)
         // cancellation does not consume a condition variable, so we need to reduce the #of waiters
         self->m_waiters -= 1;
         self->m_sem.cp(self);
         self->m_sem.VV;
      }

      static void initKey()
      {
         if (Condition::CONDKEY_INIT == false) {
            Condition::SEMCONDKEY.PP;
            if (Condition::CONDKEY_INIT == false) {
               Condition::CONDKEY_INIT = true;
               pthread_key_create(&Condition::CONDKEY, NULL);
            }
            Condition::SEMCONDKEY.VV;
         }
      }

   public:
      /// Create and initialize the conditional.
      /** If bflag is true, then the conditional will broadcast raise events, otherwise
       *  individual thread signalling with a predicate is used.  See waitFor() for 
       *  additional documentation on how the two modes differ.
       *
       * \param[in] bflag=false indicate if raise events are broadcast, default is false.
       * \param[in] dflag=true  indicates a debug flag, used to figure out why conditionals fail.
       */
      Condition(const bool bflag=false, const bool dflag=false) : m_sem(false,false), m_enabled(false)
      {
#ifdef COND_DEBUG
         m_debug = dflag;
#endif
         ::memset(&m_condition, '\0', sizeof(pthread_cond_t));
         m_broadcast = bflag;
         Condition::initKey();
         enable();
      }

      Condition(const char* tag, const bool bflag=false, const bool dflag=false) : m_sem(false,false), m_enabled(false)
      {
#ifdef COND_DEBUG
         m_debug = dflag;
#endif
         m_sem.setVerbose(tag);

         ::memset(&m_condition, '\0', sizeof(pthread_cond_t));
         m_broadcast = bflag;
         Condition::initKey();
         enable();
      }

      /// Delete the conditional.
      /**
       *  Note we Disable the conditional prior to object deletion which releases 
       *  any waiters.
       */
      ~Condition()
      {
         if (m_enabled == true) {
            // Don't care about any errors this may throw, just need to clean up.
            try {
               disable();
            }
            catch (CRException& ignore) {
            }
         }
      }
      
      /// Wait for the condition, optionally timeout out after a given # of nanoseconds.
      /** Wait for the condition to be signaled.  The function will return immediately 
       *  if this conditional instance is disabled.  This will happen if the conditional is 
       *  in the process of being deleted or reset. In this case the result is an exception
       *  with errno == ECANCELED indicating that the waitFor operation was cancelled early.
       *
       *  If a timeout is specified then the conditional may block and wait for a signal at 
       *  maximum until the system clock time is greater or equal to CURRENT_TIME+nsec_timeout 
       *  where CURRENT_TIME is the time at which waitFor is called.
       * 
       *  The conditional can be created in either broadcast or single event mode.  
       *
       *  If the conditional is created with the broadcast flag=true, then it is a broadcast
       *  conditional.  In this mode, if an event is raised and there are some clients
       *  waiting on the condition then all clients are released.  In this mode the conditional's 
       *  predicate is ignored.  If no clients are waiting when an event is raised
       *  the event is buffered and is coallesced with all prior raises.  Subsequently when a 
       *  client waitsFor the conditional, it proceeds immediately and the raise signal buffer is
       *  cleared.
       *
       *  If the conditional is created with the broadcast flag=false, then it is a single event
       *  conditinal.  In this mode, if an event is raised and there are some clients waiting on
       *  the condition then one of the clients is released.  Per (IEEE Std 1003.1-2001), spurious
       *  wakeup's are possible on multi-processor systems where multiple clients wait on the
       *  conditional from multiple processors.  To handle this, this implementation also wraps
       *  a predicate that is set to true in the first thread that is released. All other spurious
       *  woken threads will not get a true value for the predicate and will re-issue their waits.
       *  This design choice means that the conditional cannot be used to implement a reader-writer
       *  lock.  For that use case, please refer to RWLock::RWLock.  If an event is raised and there
       *  are no clients waiting then the raise count is incremented.  When a client waits on the
       *  conditional, the client is allowed to proceed, the predicate is set to true, and the
       *  raise count is decremented.  Clients waiting on the single-event conditional must call
       *  clear() to release the predicate as soon as they have performed the minimal amount of
       *  work that they need to perform atomically once the condition represented by the
       *  conditional becomes true.
       * 
       *  returns 0 if conditinal is true; returns errno value on error.
       *
       *  \param[in] nsec_timeout Number of nanoseconds to wait before timing out, default is 0
       */
      inline int waitFor(u_int64_t nsec_timeout=0L)
      {
         int result = 0;
         int oldtype;

         // early return if we are not supposed to signal events;
         if (unlikely(m_enabled == false)) {
            CRX_THROW_ERR(ECANCELED, "condition is disabled");
         }

         m_sem.PP;
#if (defined(_GNU_SOURCE) && !defined(ANDROID))
         pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,&oldtype);
         pthread_setspecific(Condition::CONDKEY,this);
         pthread_cleanup_push(Condition::threadCancel, this);

         CO_DEBUG("COND(%d)[%s,%c]: >>> waitFor(t=%lu) <m_fired=%d,m_waiters=%d>\n", CRX_GETTID(),
                  m_sem.getVerbose(),(m_broadcast?'B':'N'),nsec_timeout,m_fired,m_waiters);
#endif

         if (unlikely(m_broadcast == true)) {
            if (m_fired < 1) {
               result = do_wait(nsec_timeout);
               m_waiters = 0;
            }
            m_fired = 0;
         }
         else {
            if (m_fired < 1) {
               bool spurious = false;

               do {
                  CO_DEBUG("COND(%d)[%s,%c]: wait enter (waitagain=%d)\n", CRX_GETTID(),
                           m_sem.getVerbose(),(m_broadcast?'B':'N'), spurious);

                  result = do_wait(nsec_timeout);

                  // Spurious wakeup means:
                  //
                  //    1. you did not time out
                  //    2. you woke up, result is zero, but no-one fired.
                  //
                  // If a wakeup is spurious, the we need to loop back and wait again
                  spurious = (m_fired < 1) && result != ETIMEDOUT;

                  CO_DEBUG("COND(%d)[%s,%c]: wait exit (result=%d, waitagain=%d)\n", CRX_GETTID(),
                           m_sem.getVerbose(),(m_broadcast?'B':'N'), result, spurious);
               }
               while (result == 0 && spurious == true);
            }

            if (result == 0) {
               --m_fired;
            }
         }

#if (defined(_GNU_SOURCE) && !defined(ANDROID))
         pthread_cleanup_pop(0);
         pthread_setspecific(Condition::CONDKEY,NULL);
         pthread_setcanceltype(oldtype,&oldtype);
#endif
         m_sem.VV;

         if (m_enabled == false) {
            result = ECANCELED;
         }
         else if (unlikely(result != 0 && result != ETIMEDOUT)) {
            const char *msg = "COND: wait failed, <w=%d,f=%d>";
            CRX_THROW_ERR(result, "%s", msg,m_waiters,m_fired);
         }

         CO_DEBUG("COND(%d)[%s,%c]: <<< waitFor(rc=%d) <m_fired=%d,m_waiters=%d>\n", CRX_GETTID(),
                  m_sem.getVerbose(), (m_broadcast?'B':'N'), result, m_fired, m_waiters);

         return result;
      }

      /// Raise the condition, depending on whether the conditional is a broadcast conditional
      /// then we either call signal or broadcast.
      inline void raise(size_t usec=0L)
      {
         // early return if we are not supposed to signal events;
         m_sem.PP;
         if (unlikely(m_enabled == false)) {
            m_sem.VV;
            CRX_THROW_ERR(ECANCELED, "COND: condition is disabled");
         }

         int result = 0;

         ++m_fired;
         if (likely(m_waiters > 0)) {
            result = (m_broadcast == true) ? pthread_cond_broadcast(&m_condition) : pthread_cond_signal(&m_condition);
         }
         m_sem.VV;

         CO_DEBUG("COND(%d)[%s,%c]: Raise (m_fired=%d,m_waiters=%d)\n", CRX_GETTID(),
                  m_sem.getVerbose(), (m_broadcast?'B':'N'), m_fired, m_waiters);

         if (unlikely(result != 0)) {
            CRX_THROW_ERR(result, (m_broadcast == true) ? "broadcast failed" : "signal failed");
         }

         // optionally delay us.
         if (usec > 0) {
            CRTime now((uint64_t) 0L);
            now.usleep(usec);
         }
      }

      // only called from SIGUSR2 signal handler, should nerver be called directly, bad things happen if you do.
      inline void siguser2()
      {
         if (m_broadcast == true) {
            pthread_cond_broadcast(&m_condition);
         }
         else {
            pthread_cond_signal(&m_condition);
         }
         ++m_fired;
      }

      inline void notify() 
      {
         raise();
      }

      inline void setDebug(bool flag) 
      {
#ifdef COND_DEBUG
         m_debug=flag;
#endif
      }

      /// Reset the condition to be clear of any waiters, 
      inline void reset() 
      {
         disable();
         enable();
      }

      /// Indicate if the condition has waiters
      inline bool hasWaiters() const
      {
         return (m_waiters > 0);
      }

      /// Return count of watiers
      inline int waiters() const
      {
         return m_waiters;
      }

      /// Indicate if the conditional is a broadcast conditional
      inline bool isBroadcast()
      {
         return m_broadcast;
      }

      /// Indicate if conditional is currently disabled
      inline bool isEnabled()
      {
         return m_enabled;
      }

      /// Indicate if conditional is currently disabled
      inline bool isDisabled()
      {
         return !isEnabled();
      }

      /// set the broadcast flag to make it a broadcast conditional.  The flag will be set
      /// only if there are no waiters and no raises have been called.
      inline void setBroadcast()
      {
         m_sem.PP;
         if ((m_waiters | m_fired) == 0) {
            m_broadcast = true;
         }
         m_sem.VV;
      }

      /// Thread key used to store conditional signal pointers
      static pthread_key_t CONDKEY;
      static bool          CONDKEY_INIT;
      static Semaphore     SEMCONDKEY;
   };
};
#endif
