/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @class  Semaphore
 *
 * @brief   A fundamentally simple semaphore wrapper
 *
 * @details Implements P() and V() operations using a pthread_mutex. Thereby
 *          freeing people from the mundane task of typing all the mutex
 *          code.
 *
 * @par     DEBUG vs Release Behavior
 *
 *          In DEBUG builds, use the PP/VV macros instead of calling P()/V()
 *          directly.  PP and VV automatically capture __FILE__, __METHOD_NAME__,
 *          and __LINE__ so that error messages and stack traces report the
 *          exact call site.  In release builds PP/VV expand to plain P()/V().
 *
 *          DEBUG builds also track the previous lock owner, detect use-after-
 *          destroy, and record a full acquisition history string (m_where) that
 *          is included in exception messages.
 *
 * @par     SEMTRACE
 *
 *          When compiled with both -DDEBUG and -DSEMTRACE, every PP/VV call
 *          is recorded in a global trace ring via semtraceadd().  Each entry
 *          captures the timestamp, thread id, semaphore pointer, P-or-V flag,
 *          cost in nanoseconds, and call-site string.  Call semtracedump(FILE*)
 *          to dump the collected trace -- invaluable for diagnosing lock
 *          contention and ordering issues in multi-threaded code.
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
 */

#ifndef __SEMAPHORE_INC__
#define __SEMAPHORE_INC__

#include "crexception.h"
#include "crstring.h"
#include "crtimer.h"

#ifndef DEBUG_ADAPTIVE
#define DEBUG_ADAPTIVE DEBUG
#endif

#define THROW_MSG_DESTROY      "TID(%d), Semaphore::~Semaphore(%s), destroy err=%d(%s);\n%s%s"
#define THROW_MSG_ATTRDESTROY  "TID(%d), semaphore::~Semaphore(%s), attribute destroy, err=%d(%s)"

#ifdef DEBUG
#define TRYPP P(__FILE__, __METHOD_NAME__, __LINE__, true)
#define PP    P(__FILE__, __METHOD_NAME__, __LINE__)
#define VV    V(__FILE__, __METHOD_NAME__, __LINE__)
#define WHERE_BUFSIZE 2048
#ifdef SEMTRACE
#define SEMTRACE(PTR,PORV,COST,WHERE)    semtraceadd(PTR,PORV,COST,WHERE)
#define SEMTRACEDUMP(OUT)                 semtracedump(OUT)
#else
#define SEMTRACE(PTR,PORV,COST,WHERE)
#define SEMTRACEDUMP(FD)
#endif
#else
#define TRYPP P(true)
#define PP    P()
#define VV    V()
#endif

namespace crutil {
   extern void semtraceadd(void* ptr, bool porv, uint64_t cost, const char* where);
   extern void semtracedump(FILE* out=stderr);

   struct Trace {
      uint64_t  now;
      pid_t     tid;
      void*     sem;
      bool      porv;
      uint64_t  cost;
      char      where[224];
   };

   class Semaphore {
   private:
      /// The underlying pthread's mutex that the semaphore uses.
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
#define __sem_m_owner m_mutex.__data.__owner
#define __sem_m_depth m_mutex.__data.__count
#define __sem_m_users m_mutex.__data.__nusers
#elif defined(DEBUG)
      // Portable fallback: track lock depth and owner manually when
      // platform mutex internals are not accessible (e.g. macOS).
      pid_t m_sem_owner = 0;
      int   m_sem_depth = 0;
#define __sem_m_owner m_sem_owner
#define __sem_m_depth m_sem_depth
#define __SEM_PORTABLE_TRACKING__ 1
#endif
      pthread_mutex_t m_mutex;

      /// Note the attribute object must be present if non-default mutex init is used.
      /// pthreads keeps a pointer to this around for the lifeltime of the mutex.
      pthread_mutexattr_t m_attr;

      /// Flag indicating if it is a recursive semaphore
      bool m_recursive;

      /// Tag to use for verbosity
      const char *m_verbtag;

#ifdef DEBUG
      /// record the number of recursive calls
      pid_t m_prev_thread_id;
      bool  m_deleted;
      char  m_where[WHERE_BUFSIZE];
#endif

   public:
      /// --------------------------------------------------------------------------------------
      /// cv(void* cond): Called prior to calling pthread_cond_wait in Conditional
      ///                 the mutex is held when this happens
      /// --------------------------------------------------------------------------------------
      inline void cv(void* /*cond*/) {
#ifdef __sem_m_depth
         __sem_m_depth -= 1;
#endif
      }

      /// --------------------------------------------------------------------------------------
      /// cp(void* cond): Called prior to calling pthread_cond_wait in Conditional
      ///                 the mutex is held when this happens
      /// --------------------------------------------------------------------------------------
      inline void cp(void* /*cond*/) {
#ifdef __sem_m_depth
         __sem_m_depth += 1;
#endif
      }

      static const char* VERBTAG;

      /// Default constructor, inits the underlying mutex and marks it as unlocked.
      Semaphore(const bool recursive=false, const bool adaptive=false, const char* tag=Semaphore::VERBTAG) 
#ifdef DEBUG
         : m_recursive(recursive), m_verbtag(tag), m_prev_thread_id(0), m_deleted(false)
#else
         : m_recursive(recursive), m_verbtag(tag) 
#endif
      {
         int result;
#ifdef DEBUG
         *m_where = '\0';
#endif

#if 0
//#if DEBUG_ADAPTIVE
         // Turns out adaptive semaphores are mostly bad, for CPU thread load as they rely
         // on spin-locks. They're ok for some use cases but not for most.
         //
         // Tell us if we are using one somewhere.
         try {
            if (adaptive == true) {
               CRX_THROW_CHK(-1, "allocated adaptive semaphore");
            }
         }
         catch (CRException& ex) {
            CRX_REPORT_CATCH(stderr,ex);
         }
#endif
 
         if (unlikely((result=pthread_mutexattr_init(&m_attr)) != 0)) {
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            CRX_THROW_CHK(result, "mutex attrinit failed");
#else
            abort();
#endif
         }

         if (unlikely(recursive == true)) {
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
            pthread_mutexattr_settype(&m_attr, likely(recursive==true) ? PTHREAD_MUTEX_RECURSIVE_NP : PTHREAD_MUTEX_RECURSIVE);
#else
            pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
#endif
         }
         else if (unlikely(adaptive == true)) {
            // pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_ERRORCHECK);
#ifdef PTHREAD_ADAPTIVE_INITIALIZER_NP
            pthread_mutexattr_settype(&m_attr, likely(adaptive==true) ? PTHREAD_MUTEX_ADAPTIVE_NP : PTHREAD_MUTEX_NORMAL);
#else
            pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_NORMAL);
#endif
         }
         else {
            pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_NORMAL);
         }

         pthread_mutex_init(&m_mutex, &m_attr);
     }

      /// If locked the mutex is unlocked, and then destroyed.
      ~Semaphore() {
         int  result;

         // This happens with conditional related mutexes
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
         if (__sem_m_owner == 0 && __sem_m_users > 0) {
            __sem_m_users = 0;
         }
#endif

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexceptions"
#else
#pragma GCC diagnostic ignored "-Wterminate"
#endif

         if (unlikely(((result=pthread_mutex_destroy(&m_mutex)) != 0))) {
#ifdef DEBUG
            if (m_verbtag != NULL) {
               CRX_STACKTRACE(stderr, result, false, THROW_MSG_DESTROY, CRX_GETTID(), result, Semaphore::maperr(result), "Acquired at: ", m_where);
            }
            CRX_THROW_CHK(result, THROW_MSG_DESTROY, CRX_GETTID(), m_verbtag, result, Semaphore::maperr(result), m_where);
#else
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            if (m_verbtag != NULL) {
               CRX_STACKTRACE(stderr, result, false, THROW_MSG_DESTROY, CRX_GETTID(), result, Semaphore::maperr(result), "", "");
            }
            CRX_THROW_CHK(result, THROW_MSG_DESTROY, CRX_GETTID(), m_verbtag, result, Semaphore::maperr(result), "");
#endif
#endif
         }

         if (unlikely((result=pthread_mutexattr_destroy(&m_attr)) != 0)) {
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            if (m_verbtag != NULL) {
               CRX_STACKTRACE(stderr, result, false, THROW_MSG_ATTRDESTROY, CRX_GETTID(), result, Semaphore::maperr(result));
            }
            CRX_THROW_CHK(result, THROW_MSG_ATTRDESTROY, CRX_GETTID(), m_verbtag, result, Semaphore::maperr(result));
#endif
         }

#pragma GCC diagnostic pop

#ifdef DEBUG
         m_deleted = true;
         m_prev_thread_id = CRX_GETTID();
         CRS::clear(m_where);
#endif
      }

      /// --------------------------------------------------------------------------------------
      /// P(): Basic primitive to lock the mutex, 
      /// --------------------------------------------------------------------------------------
#ifdef DEBUG
      inline int P(const char* file="Unknown P location", const char* meth="For addnl info use PP not P()", const int line=0, const bool trylock=false) {
         pid_t tid = CRX_GETTID();
#else
      inline int P(const bool trylock=false) {
#endif
         int result;

#ifdef DEBUG
         // this may fail, but if it is still good, it will give early detection of fault
         if (unlikely(m_deleted == true)) {
            const char *msg="acquire DELETED semaphore, this=%llu, depth=%d, last acquired by tid=%d, [%s,%s:%d]";
            CRX_THROW_CHK(-1, msg, this, __sem_m_depth, m_prev_thread_id,file,meth,line);
         }

#ifdef SEMTRACE
         CRTime now;
#endif
         if (unlikely((result=pthread_mutex_trylock(&m_mutex)) != 0)) {
            if (result == EBUSY && tid != __sem_m_owner && trylock == false) {
               result = pthread_mutex_lock(&m_mutex);
            }
         }

#ifdef SEMTRACE
         { char buf[200];
           CRSnprintf(buf, "P from %s::%s:%d", file, meth, line);
           SEMTRACE(this,1,now.diff(),buf); }
#endif
#else
         result = (likely(trylock == false)) ? pthread_mutex_lock(&m_mutex) : pthread_mutex_trylock(&m_mutex);
#endif

         if (unlikely(result != 0)) {
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            if (unlikely(trylock == true)) {
               return __sem_m_owner;
            }
#endif
               
            int type;
            pthread_mutexattr_gettype(&m_attr, &type);

#ifdef DEBUG
            const char *msg="acquire semaphore, type=%s, err=%d(%s), this=%llu, depth=%d, current tid=%d, last acquired by tid=%d, from:\n%s";

            if (unlikely(m_verbtag != NULL)) {
               fprintf(stderr, 
                       "\n===================\n   (%u):---> %s(%lu) FAIL /P(%d) [%s:%d,%s]", 
                       tid, getVerbose(), (unsigned long)(&m_mutex), __sem_m_depth, file, line, meth);

               CRX_STACKTRACE(stderr, result, true, msg, maptype(type), result, Semaphore::maperr(result), this, __sem_m_depth, tid, __sem_m_owner, m_where);
            }
            else {
               CRX_THROW_CHK(result, msg, maptype(type), result, Semaphore::maperr(result), this, __sem_m_depth, tid, __sem_m_owner, m_where);
            }
#else
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            const char *msg="acquire semaphore, type=%s, err=%d(%s), owner=%d";
            CRX_THROW_CHK(result, msg, maptype(type), result, Semaphore::maperr(result), __sem_m_owner);
#endif
#endif
         }
         else if (m_recursive == false) {
#ifdef __sem_m_depth
            __sem_m_depth = 1;
#endif
         }
#ifdef __SEM_PORTABLE_TRACKING__
         // For recursive mutexes with portable tracking, manually increment depth
         else if (m_recursive == true) {
            __sem_m_depth += 1;
         }
         __sem_m_owner = CRX_GETTID();
#endif

#ifdef DEBUG
         char tmp[WHERE_BUFSIZE] = "";
         CRStrcpy(tmp, m_where);
         CRSnprintf(m_where, "++ [%6d:%s(%lu)] %s:%d,%s - depth=%d\n%s", CRX_GETTID(), getVerbose(), this, file, line, meth, __sem_m_depth, tmp);

         if (unlikely(m_verbtag != NULL)) {
            fprintf(stderr, 
                    "   (%u):---> %s(%lu) /P(%d) [%s:%d,%s]\n", 
                    __sem_m_owner, getVerbose(), (unsigned long)(&m_mutex), __sem_m_depth, file, line, meth);
         }
#endif
         return result;
      }

      /// --------------------------------------------------------------------------------------
      /// V(): Basic primitive to unlock the mutex, 
      /// --------------------------------------------------------------------------------------
#ifdef DEBUG
      inline void V(const char* file="Unknown P location", const char* meth="For addnl info use VV not V()", const int line=__LINE__) {
         pid_t tid = CRX_GETTID();
         pid_t ptid = m_prev_thread_id;
         int   depth = __sem_m_depth;
#else
      inline void V() {
#endif
         int result = EPERM;

#ifdef DEBUG
         // this may fail, but if it is still good, it will give early detection of fault
         if (unlikely(m_deleted == true)) {
            const char *msg="release FREE'd semaphore, this=%llu, depth=%d, last acquired by tid=%d";
            CRX_THROW_CHK(-1, msg, this, depth, m_prev_thread_id);
         }

         m_prev_thread_id = __sem_m_owner;
#endif
#ifdef __SEM_PORTABLE_TRACKING__
         // For portable tracking, manually update depth for both types
         if (m_recursive == true) {
            __sem_m_depth -= 1;  // Decrement for next call
         } else {
            __sem_m_depth = 0;   // Reset for next call
         }
#else
         // Platform with native depth tracking - only manage non-recursive
         if (m_recursive == false) {
#ifdef __sem_m_depth
            __sem_m_depth = 0;
#endif
         }
#endif

#ifdef DEBUG
#ifdef SEMTRACE
         CRTime now;
#endif
         if(unlikely((depth <= 0 || (result=pthread_mutex_unlock(&m_mutex)) != 0))) {
            int type;

            m_prev_thread_id = ptid;
            pthread_mutexattr_gettype(&m_attr, &type);

            const char *msg = "release semaphore, type=%s, err=%d(%s), this=%llu, depth=%d, current tid=%d, last acquired by tid=%d, from:\n%s";

            if (unlikely(m_verbtag != NULL)) {
               fprintf(stderr, 
                       "\n===================\n   (%u):---> %s(%lu) FAIL /V(%d), [%s:%d,%s]", 
                       tid, getVerbose(), (unsigned long)(&m_mutex), depth, file, line, meth);

               CRX_STACKTRACE(stderr, result, true, msg, maptype(type), result, Semaphore::maperr(result), this, depth, tid, m_prev_thread_id, m_where);
            }
            else {
               CRX_THROW_CHK(result, msg, maptype(type), result, Semaphore::maperr(result), this, depth, tid, m_prev_thread_id, m_where);
            }
         }

#ifdef SEMTRACE
         { char buf[200];
           CRSnprintf(buf, "V from %s::%s:%d", file, meth, line);
           SEMTRACE(this,0,now.diff(),buf); }
#endif
         char tmp[WHERE_BUFSIZE] = "";
         CRStrcpy(tmp, m_where);
         CRSnprintf(m_where, "-- [%06d:%s(%lu)] %s:%d,%s - depth=%d\n%s", CRX_GETTID(), getVerbose(), this, file, line, meth,  __sem_m_depth, tmp);

         if (unlikely(m_verbtag != NULL)) {
            fprintf(stderr, 
                    "   (%u):---> %s(%lu) /V(%d), [%s:%d,%s]\n", 
                    tid, getVerbose(), (unsigned long)(&m_mutex), depth, file, line, meth);
         }
#else
         if(unlikely((result=pthread_mutex_unlock(&m_mutex)) != 0)) {
            int type;
            pthread_mutexattr_gettype(&m_attr, &type);
            const char *msg = "release semaphore, type=%s";
            CRX_THROW_CHK(result, msg, maptype(type));
         }
#endif
      }

      /// --------------------------------------------------------------------------------------
      /// Other utility methods
      /// --------------------------------------------------------------------------------------
      inline pthread_mutex_t& getSEM() {
         return m_mutex;
      }

      inline const char* setVerbose() {
         return setVerbose(Semaphore::VERBTAG);
      }

      inline const char* setVerbose(const char *tag) {
         const char* result = m_verbtag;
         m_verbtag = tag;
         return result;
      }

      inline const char* getVerbose() {
         return (m_verbtag != NULL ? m_verbtag : "NULL");;
      }

      static const char* maperr(const int err) {
         const char* errstr;

         switch(err) {
            case EINVAL:  errstr = "EINVAL"; break;
            case EBUSY:   errstr = "EBUSY"; break;
            case EAGAIN:  errstr = "EAGAIN"; break;
            case ENOMEM:  errstr = "ENOMEM"; break;
            case EDEADLK: errstr = "EDEADLK"; break;
            case EPERM:   errstr = "EPERM"; break;

            default:
               errstr = "UNMAPPED";
               break;
         }

         return errstr;
      }

      static const char* maptype(const int type) {
         const char* typestr;

         switch(type) {
#ifdef PTHREAD_MUTEX_RECURSIVE_NP
            case PTHREAD_MUTEX_RECURSIVE_NP:
#else
            case PTHREAD_MUTEX_RECURSIVE:
#endif
               typestr = "RECURSIVE";
               break;
#if (defined(_GNU_SOURCE) && !defined(ANDROID) && !defined(__APPLE__))
            case PTHREAD_MUTEX_ADAPTIVE_NP:
               typestr = "ADAPTIVE_NP";
               break;
#endif

            case PTHREAD_MUTEX_ERRORCHECK:
               typestr = "ERRORCHECK";
               break;

            case PTHREAD_MUTEX_NORMAL:
            // case PTHREAD_MUTEX_DEFAULT: same as NORMAL
               typestr = "NORMAL(DEF)";
               break;

            default:
               typestr = "UNKNOWN";
               break;
         }

         return typestr;
      }
   };
};
#endif
