/** \class  CRException
 *
 * \brief   A simple wrapper on runtime_exception used by CRunnable library
 *
 * \details Pretty straight forward. Subclasses can overwrite what() to provide
 *          custom output of the exception m_messages.
 *
 *          In order to get stack traces you need to compile with debug symbols.
 *          On Linux, link with -rdynamic so backtrace_symbols can resolve names.
 *          On macOS, no special linker flag is needed.
 *
 *          Linux: g++ -std=gnu++17 -DDEBUG=1 -Og t.cpp -rdynamic
 *          macOS: g++ -std=gnu++17 -D_GNU_SOURCE -DDEBUG=1 -Og t.cpp
 *
 * \par Macro Quick Reference
 *
 *          **Throwing:**
 *          - CRX_THROW(MSG, ...)              -- throw with errno=-1
 *          - CRX_THROW_ERR(ERR, MSG, ...)     -- throw with explicit errno
 *          - CRX_THROW_CHK(ERR, MSG, ...)     -- throw unless thread is canceled
 *          - CRX_TIF(EXPR, MSG, ...)          -- throw if EXPR is true
 *          - CRX_TUNLESS(EXPR, MSG, ...)      -- throw if EXPR is false
 *          - CRX_TIF_ERR(EXPR, ERR, MSG, ...) -- throw with errno if EXPR is true
 *          - CRX_TIFNULL(PTR)                 -- throw on NULL, otherwise return PTR
 *
 *          **Catching / Reporting:**
 *          - CRX_CAPTURE_CATCH(STR, CRX)      -- append catch summary to a std::string
 *          - CRX_REPORT_CATCH(FD, CRX)        -- write catch summary to FILE* fd
 *
 *          **Stack Traces:**
 *          - CRX_STACKTRACE(FD, ERR, RETHROW, MSG, ...)   -- capture trace to FILE*,
 *              optionally rethrow (RETHROW=true/false)
 *          - CRX_REPORT_TRACE(FD, ERR, RETHROW, MSG, ...) -- alias for CRX_STACKTRACE
 *
 *          **Thread Cancellation:**
 *          - CRException::notifyCancel(tid)  -- mark a thread as canceled
 *          - CRException::clearCancel(tid)   -- clear cancellation for a thread
 *          - CRX_THROW_CHK and CRX_STACKTRACE(RETHROW=true) suppress throws for
 *            threads whose tid is NOT in the cancel map (i.e. other threads yield
 *            while the canceled thread is shutting down).
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crutil
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 *
 * \license You can obtain and redistribute or modify this program under the
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */


#ifndef __CREXCEPTION_INC__
#define __CREXCEPTION_INC__
#include <string>
using namespace std;

#include "crtypes.h"
#include "crlikely.h"

#if (defined(_GNU_SOURCE) || defined(__APPLE__)) && !defined(ANDROID)
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include <exception>
#include <stdexcept>

#define MAX_BACK_TRACE_SYMBOLS   50

#define __METHOD_NAME__  __PRETTY_FUNCTION__

#define CRX_THROW_CHK(ERR, MSG, ...)   if (CRException::isThreadCanceled(CRX_GETTID()) == false) {\
                                          throw CRException(__FILE__, __LINE__, ERR, __METHOD_NAME__, MSG, ## __VA_ARGS__);\
                                       }

#define CRX_THROW_ERR(ERR, MSG, ...)   throw CRException(__FILE__, __LINE__, ERR, __METHOD_NAME__, MSG, ## __VA_ARGS__)

#define CRX_LOG_THROW_ERR(L,ERR,MSG,...) {if (likely(L)) {if (likely((L)->geError())) {(L)->log(L_ERROR, 1, MSG, ## __VA_ARGS__); }}\
                                          CRX_THROW_ERR(ERR,MSG, ## __VA_ARGS__);}

#define CRX_THROW(MSG, ...)               CRX_THROW_ERR(-1,MSG, ## __VA_ARGS__)
#define CRX_LOG_THROW(L,MSG,...)          CRX_LOG_THROW_ERR(L,-1,MSG, ## __VA_ARGS__)
#define CRX_TUNLESS(EXPR, MSG, ...)       if (likely(!(EXPR)))   { CRX_THROW(MSG, ## __VA_ARGS__); }
#define CRX_TIF(EXPR, MSG, ...)           if (unlikely((EXPR)))  { CRX_THROW(MSG, ## __VA_ARGS__); }
#define CRX_TIF_ERR(EXPR, ERR, MSG, ...)  if (unlikely((EXPR)))  { CRX_THROW_ERR(ERR, MSG, ## __VA_ARGS__); }
#define CRX_TIFNULL(PTR)                  CRException::_throwifnull(PTR,__FILE__,__LINE__)
#define CRX_GETTID                        CRException::_gettid

#define CRX_CAPTURE_CATCH(STR,CRX) {\
   char pbuf[8];\
   char tbuf[8];\
   snprintf(pbuf,sizeof(pbuf),"%u",getpid());\
   snprintf(tbuf,sizeof(tbuf),"%u",CRX_GETTID());\
   STR += "\nCRException Caught: ";\
   if (CRX.isStaticOnly() == false) {\
      STR += "m_pid(";\
      STR += pbuf;\
      STR += ");m_tid(";\
      STR += tbuf;\
      STR += ") ";\
   }\
   STR += "at ";\
   STR += __FILE__;\
   STR += ":";\
   STR += std::to_string(__LINE__);\
   STR += " in [";\
   STR += __METHOD_NAME__;\
   STR += "]\n";\
   STR += CRX;\
}

#define CRX_LOG_CATCH(L,CRX)         {string __crx_out; CRX_CAPTURE_CATCH(__crx_out, CRX); (L)->log(L_ERROR, 1, "CRX: Capture Exception --------:\n%s.", __crx_out.c_str());}

#ifdef DEBUG
#define CRX_REPORT_CATCH(FD,CRX)     {string __crx_out; CRX_CAPTURE_CATCH(__crx_out, CRX); fprintf(FD,"\n-------------------\n"); fprintf(FD, "%s", __crx_out.c_str()); fprintf(FD,"\n\n");}
#else
#define CRX_REPORT_CATCH(FD,CRX)     {string __crx_out; CRX_CAPTURE_CATCH(__crx_out, CRX); fprintf(FD, "%s", __crx_out.c_str());}
#endif

#define CRX_STACKTRACE(FD,ERR,RETHROW,MSG,...)  {try { CRX_THROW_ERR(ERR,MSG,## __VA_ARGS__); } catch (CRException& e) { CRX_REPORT_CATCH(FD,e); usleep(200); if(RETHROW && CRException::isThreadCanceled(CRX_GETTID()) == false) {throw;}}}

#define CRX_REPORT_TRACE(FD,ERR,RETHROW,MSG,...)  CRX_STACKTRACE(FD,ERR,RETHROW,MSG,## __VA_ARGS__)
#define CRX_LOG_TRACE(L,ERR,RETHROW,MSG,...)  {try { CRX_THROW_ERR(ERR,MSG,## __VA_ARGS__); } catch (CRException& e) { CRX_LOG_CATCH(L,e); usleep(200); if(RETHROW && CRException::isThreadCanceled(CRX_GETTID()) == false) {throw;}}}

namespace crutil {
   class CRException : public std::runtime_error {
   private:
      const int   m_errnumber;
      const char *m_filename;
      const char *m_funcname;
      const unsigned int m_linenumber;
      void       *m_symbols[MAX_BACK_TRACE_SYMBOLS];
      int         m_nums;
      string      m_message;
      string      m_errmsg;
      pid_t       m_pid;
      pid_t       m_tid;
      bool        m_staticonly;

      /// map containing which thread_id's are canceled
      static std::map<pid_t,bool> s_tmap;
      static pthread_mutex_t      s_tlock;

#if (defined(_GNU_SOURCE) || defined(__APPLE__)) && !defined(ANDROID)
      /// Demangle a single backtrace symbol string, returning a readable version.
      static std::string demangle(const char* sym) {
         std::string result(sym);
         std::string mangled;
         size_t mpos = std::string::npos;
         size_t mlen = 0;

#if defined(__APPLE__)
         // macOS format: "N  binary  0xaddr _ZMangled + offset"
         // The mangled name starts with _Z and is followed by ' '
         size_t zpos = result.find(" _Z");
         if (zpos != std::string::npos) {
            zpos += 1; // skip the leading space
            size_t end = result.find(' ', zpos);
            if (end == std::string::npos) end = result.size();
            mangled = result.substr(zpos, end - zpos);
            mpos = zpos;
            mlen = end - zpos;
         }
#else
         // Linux format: "./binary(_ZMangled+0xoffset) [0xaddr]"
         size_t lparen = result.find('(');
         if (lparen != std::string::npos) {
            size_t plus = result.find('+', lparen);
            size_t rparen = result.find(')', lparen);
            size_t end = (plus != std::string::npos && plus < rparen) ? plus : rparen;
            if (end != std::string::npos && end > lparen + 1) {
               mangled = result.substr(lparen + 1, end - lparen - 1);
               mpos = lparen + 1;
               mlen = end - lparen - 1;
            }
         }
#endif

         if (mpos != std::string::npos && !mangled.empty()) {
            int status = -1;
            char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if (status == 0 && demangled != nullptr) {
               result.replace(mpos, mlen, demangled);
               free(demangled);
            }
         }

         return result;
      }
#endif

   protected:
#if 0
      const char *throwifnull(const char *ptr, const char* file, const unsigned int line) {
         if (unlikely(ptr == NULL)) {
            CRX_THROW("CRX: NULL pointer dereference %s(%d)", file, line);
         }

         return ptr;
       }
#endif

      virtual void initialize(const char *msg, va_list& args) {
         char buf[2048];

         if (likely(msg != NULL)) {
            vsnprintf(buf, sizeof(buf)-65, CRX_TIFNULL(msg), args);
            buf[sizeof(buf)-65] = '\0';
         }
         else {
            buf[0]='\0';
         }

         if (m_errnumber != -1) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), ", err=%d", m_errnumber);
            strcat(buf, tmp);
         }
         
         m_message = buf;
         m_errmsg.clear();

         // FIXME: might be a cool idea to define a class static set of error m_messages, 
         // and use those for the m_errmsg.  This would allow an app specific set of
         // error m_messages.
         if (likely(m_errnumber > 0)) {
            if (strerror_r(m_errnumber, buf, sizeof(buf)-1) == 0) {
               buf[sizeof(buf)-1] = '\0';
               m_errmsg = buf;
            }
         }

         m_pid = getpid();
         m_tid = CRX_GETTID();
      }

   public:
      /// Default constructor.
      CRException(const char *file, const unsigned int line, const int err, const char *func, const char *msg...)
         : runtime_error(""), m_errnumber(err), m_filename(file), m_funcname(func),  m_linenumber(line),  m_staticonly(false)
      {
#if (defined(_GNU_SOURCE) || defined(__APPLE__)) && !defined(ANDROID)
         m_nums = backtrace(m_symbols, MAX_BACK_TRACE_SYMBOLS);
#else
         m_nums = 0;
#endif
         va_list ap;

         if (msg != NULL) {
            va_start(ap, msg);
            initialize(msg, ap);
            va_end(ap);
         }
         else {
            initialize(msg, ap);
         }
      }

      static pid_t _gettid() {
         static thread_local pid_t tid = 0;

         if (likely(tid != 0)) {
            return tid;
         }

#if defined(__APPLE__)
         uint64_t tid64;
         pthread_threadid_np(nullptr, &tid64);
         tid = (pid_t) tid64;
#else
         tid = (pid_t) ::syscall(SYS_gettid);
#endif
         return tid;
      }



      /// Constructor accepting a pre-formatted string message (no variadic args).
      CRException(const char *file, const unsigned int line, const int err, const char *func, const string& msg)
         : runtime_error(""), m_errnumber(err), m_filename(file), m_funcname(func),  m_linenumber(line),  m_staticonly(false)
      {
#if (defined(_GNU_SOURCE) || defined(__APPLE__)) && !defined(ANDROID)
         m_nums = backtrace(m_symbols, MAX_BACK_TRACE_SYMBOLS);
#else
         m_nums = 0;
#endif
         m_message = msg;
         m_errmsg.clear();

         if (m_errnumber != -1) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), ", err=%d", m_errnumber);
            m_message += tmp;
         }

         if (likely(m_errnumber > 0)) {
            char buf[2048];
            if (strerror_r(m_errnumber, buf, sizeof(buf)-1) == 0) {
               buf[sizeof(buf)-1] = '\0';
               m_errmsg = buf;
            }
         }

         m_pid = getpid();
         m_tid = CRX_GETTID();
      }

      /// Return the reason for the exception. Overrides base class impl.
      virtual const char* what() const noexcept {
         return m_message.c_str();
      }

      virtual const char* errmsg() const noexcept {
         return m_errmsg.empty() ? m_message.c_str() : m_errmsg.c_str();
      }
      
      virtual string& errmsg(string& str) noexcept {
         return str = m_errmsg;
      }

      virtual string& what(string& str) noexcept {
         return str = m_message;
      }

      virtual const char* file() const {
         return m_filename;
      }
      
      virtual int geterrno() const {
         return m_errnumber;
      }

      virtual unsigned int line() const {
         return m_linenumber;
      }

      void setStaticOnly(bool value=true) {
         m_staticonly = value;
      }

      bool isStaticOnly() {
         return m_staticonly;
      }

      friend std::ostream& operator<<(std::ostream& output, const CRException& ex) {
         output << "CRException Raised: ";
         if (ex.m_staticonly == false) {
            output << "by m_pid(" << getpid() << ");m_tid(" << CRX_GETTID() << ")";
         }
         output << " at " << ex.m_filename << ":" << ex.m_linenumber << " in [" << ex.m_funcname << "]\n";
         output << "       APP Message: " << ex.m_message << "\n";
         
         if (ex.m_errnumber < 0) {
            output << "         APP Errno: " << ex.m_errnumber;
         }
         else {
            output << "         SYS errno: " << ex.m_errnumber << " -> " << ex.m_errmsg;
         }

         if (ex.m_staticonly == false && ex.m_nums > 1) {

#if (defined(_GNU_SOURCE) || defined(__APPLE__)) && !defined(ANDROID)
            char **syms = backtrace_symbols(ex.m_symbols, ex.m_nums);

            if (syms != NULL) {
               output << "\n         Calltrace: ";
               for (int i=1; i<ex.m_nums; ++i) {
                  output << demangle(syms[i]);
                  if (i < ex.m_nums-1) {
                     output << "\n                    ";
                  }
               }
               free(syms);
            }
#endif
         }

         return output;
      }

      friend std::ostream& operator<<(std::ostream& output, const CRException* ex) {
         if (unlikely(ex == NULL)) {
            output << "CRException ref was NULL";
            return output;
         }

         return output << *ex;
      }

      friend string& operator+=(string& output, const CRException& ex) {
         std::stringstream ss;
         ss << ex;
         output += ss.str();
         return output;
      }

      friend string& operator+=(string& output, const CRException* ex) {
         std::stringstream ss;
         ss << ex;
         output += ss.str();
         return output;
      }

      friend string& operator+(string& output, const CRException& ex) {
         output += ex;
         return output;
      }

      virtual std::string& toString(std::string& output) const {
         std::stringstream ss;
         ss << this;
         output = ss.str();
         return output;
      }

      template<typename T, typename F>
      static T* _throwifnull(T* ptr, F file, const unsigned int line) {
         if (unlikely((void*) ptr == NULL)) {
            throw CRException((const char*) file, line, -1, __METHOD_NAME__, "CRX: NULL pointer dereference %s(%d)", file, line);
         }

         return ptr;
      }

      static bool isThreadCanceled(const pid_t tid, const bool canceled=false) {
         if (s_tmap.empty() == true) {
            return false;
         }

         if (canceled == true) {
            return true;
         }

         pthread_mutex_lock(&CRException::s_tlock);
         bool result = (CRException::s_tmap.find(tid) == CRException::s_tmap.end());
         pthread_mutex_unlock(&CRException::s_tlock);
         
         return result;
      }

      static void notifyCancel(pid_t tid) {
         pthread_mutex_lock(&CRException::s_tlock);
         CRException::s_tmap[tid] = true;
         pthread_mutex_unlock(&CRException::s_tlock);
      }

      static void clearCancel(pid_t tid) {
         pthread_mutex_lock(&CRException::s_tlock);
         try {
            // highly unlikely but, comparator could in theory throw an exception, 
            // if it does we will ignore it
            CRException::s_tmap.erase(tid);
         }
         catch (std::exception& ignore) {
         }
         pthread_mutex_unlock(&CRException::s_tlock);
      }
   };
};
#endif
