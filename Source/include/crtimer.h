/** \class  CRTimer
 *
 * \brief   A wraper for time
 *
 * \details Uses nanosecond precision time to do things like delay, time,
 *          and convert to GMT.
 *
 * \author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * \see     http://www.vadura.eu/crutil
 * \copy    Copyright (c) 2010-2013 by Dennis Vadura, All rights reserved.
 *
 * \license You can obtain and redistribute or modify this program under the 
 *          terms of the Software License Agreement Provided in the file:
 *          <distribution-root>/LICENSE.txt
 */

#ifndef __CRTIMERS_INC__
#define __CRTIMERS_INC__

#include "crexception.h"

namespace crutil {
   typedef struct crts {
      union {
         ts_t ts;
         tv_t tv;
      } t;

      uint32_t precision;

      crts(uint64_t offset=1L) {
         if (likely(offset>0)) {
            now(offset);
         }
         else {
            clear();
         }
      }

      crts(const struct crts& tm) {
         t = tm.t;
         precision = tm.precision;
      }

      crts(const ts_t& tm, uint64_t offset=0L) {
         precision = NS_IN_ONE_SEC;
         t.ts = tm;

         if (offset > 0L) {
            ns2ts(t2ns(), offset);
         }
      }

      crts(const tv_t& tm, uint64_t offset=0L) {
         precision = US_IN_ONE_SEC;
         t.tv = tm;

         if (offset > 0L) {
            ns2tv(t2ns(), offset);
         }
      }
      
      crts(const uint64_t tm, uint64_t offset) {
         ns2ts(tm+offset);
      }

      crts(const char* value) {
         operator=((const char*) value);
      }

      crts(const string value) {
         operator=(value.c_str());
      }

      crts(const string& value) {
         operator=(value.c_str());
      }

      crts& now(uint64_t offset=0L) {
         clock_gettime(CLOCK_REALTIME, &t.ts);
         precision = NS_IN_ONE_SEC;

         // no-one will miss a nanosecond if it ever is an actual offset.
         // 1ns is passed to crts() constructor as default to select the option
         // to get the current time. So we ignore it here as any offset 1ns or lower
         // means get the current time.
         if (offset > 1L) {
            ns2ts(t2ns(), offset);
         }

         return *this;
      }

      crts& clear() {
         precision = NS_IN_ONE_SEC;
         t.ts.tv_sec = 0L;
         t.ts.tv_nsec = 0L;

         return *this;
      }

      inline void ns2ts(uint64_t tm, uint64_t offset=0L) {
         precision = NS_IN_ONE_SEC;
         tm += offset;
         t.ts.tv_sec  = tm/precision;
         t.ts.tv_nsec = tm - t.ts.tv_sec*precision;
      }

      inline void ns2tv(uint64_t tm, uint64_t offset=0L) {
         ns2ts(tm,offset);

         precision = US_IN_ONE_SEC;
         tv_t tmp;
         tmp.tv_sec = t.ts.tv_sec;
         tmp.tv_usec = t.ts.tv_nsec/1000;
         t.tv = tmp;
      }

      inline uint64_t t2ns() const {
         return t.ts.tv_sec*NS_IN_ONE_SEC + ((precision == NS_IN_ONE_SEC) ? t.ts.tv_nsec : t.tv.tv_usec*NS_IN_ONE_USEC);
      }

      inline int nsleep(struct crts* rem=NULL) {
         return nanosleep(&t.ts, (rem == NULL) ? NULL : &rem->t.ts);
      }


      /// sleep for some period of time
      inline int nsleep(uint64_t nsec, struct crts* rem=NULL) {
         ns2ts(nsec);
         return nsleep(rem);
      }

      inline int usleep(uint64_t usec, struct crts* rem=NULL) {
         return nsleep(usec * NS_IN_ONE_USEC, rem);
      }

      inline int msleep(uint64_t msec, struct crts* rem=NULL) {
         return nsleep(msec * NS_IN_ONE_MSEC, rem);
      }

      inline int sleep(uint64_t sec, struct crts* rem=NULL) {
         return nsleep(sec * NS_IN_ONE_SEC, rem);
      }

      /// Implement an interval delay that takes the current time in the timer, and sleeps for 
      /// ((nsec()+target) - now.nsec()), ie. take the value of timer + target to be our total
      /// delay, and sleep for target-(now()-nsec()) nanoseconds.
      ///
      /// If adjustment is > target delay, then we have gone over target delay by adjustment-target
      /// amount.
      /// 
      /// For example:  target is 200
      ///               nsec() is baseline of 100.
      ///               tmp()  is 110
      ///         -->   adjustment := tmp.nsec() - nsec() = 10
      ///         -->   result will be nanosleep(10<200?180:___), or nanosleep(180);
      ///
      /// For example:  target is 100
      ///               nsec() is baseline of 50
      ///               tmp()  is 500
      ///         -->   adjustment := tmp.nsec() - nsec() = 450
      ///         -->   result will be return(450<100) ? ___ : 100 - (450 mod 100), or return 50;
      ///
      /// This is not super accurate, but it is good enough within a few microseconds of target.
      /// 
      inline int ndelay(uint64_t target, struct crts* rem=NULL) {
         uint64_t ns = nsec();
         crts tmp(ns != 0);
         uint64_t offset = tmp.nsec()-ns;
         uint64_t adjustment = target - ((offset <= target) ? offset : (offset % target));
         int result = nsleep(adjustment, rem);
         now();
         return result;
      }

      inline int udelay(uint64_t target, struct crts* rem=NULL) {
         return ndelay(target*1000);
      }

      inline int mdelay(uint64_t target, struct crts* rem=NULL) {
         return ndelay(target*1000000);
      }

      inline int sdelay(uint64_t target, struct crts* rem=NULL) {
         return ndelay(target*1000000000);
      }


      inline uint64_t diff() {
         crts now;
         return (now.t2ns()-t2ns());
      }

      inline uint64_t diff(crts& start) {
         return (t2ns()-start.t2ns());
      }

      inline uint64_t diffus() {
         return diff()/NS_IN_ONE_USEC;
      }

      inline uint64_t diffms() {
         return diff()/NS_IN_ONE_MSEC;
      }

      inline uint64_t nsec() const {
         return t2ns();
      }

      inline uint64_t usec() const {
         return t2ns()/NS_IN_ONE_USEC;
      }

      inline uint64_t msec() const {
         return t2ns()/NS_IN_ONE_MSEC;
      }
      
      inline uint64_t sec() const {
         return t2ns()/NS_IN_ONE_SEC;
      }

      inline ts_t& ts() { 
         if (precision != NS_IN_ONE_SEC) {
            ns2ts(t2ns(), 0);
         }
         return t.ts; 
      }

      inline tv_t& tv() { 
         if (precision != US_IN_ONE_SEC) {
            ns2tv(t2ns(), 0);
         }
         return t.tv; 
      }
      
      inline bool operator==(const crts& rhs) const {
         return t.ts.tv_sec == rhs.t.ts.tv_sec && t.ts.tv_nsec == rhs.t.ts.tv_nsec;
      }

      inline bool operator!=(const crts& rhs) const {
         return t.ts.tv_sec != rhs.t.ts.tv_sec || t.ts.tv_nsec != rhs.t.ts.tv_nsec;
      }

      inline bool operator<=(const crts& rhs) const {
         return t2ns() <= rhs.t2ns();
      }

      inline bool operator>=(const crts& rhs) const {
         return t2ns() >= rhs.t2ns();
      }

      inline bool operator<(const crts& rhs) const {
         return t2ns() < rhs.t2ns();
      }

      inline bool operator>(const crts& rhs) const {
         return t2ns() > rhs.t2ns();
      }

      template <typename T> 
      inline bool operator==(const T rhs) const {
         return t2ns() == (uint64_t) rhs;
      }

      template <typename T> 
      inline bool operator!=(const T rhs) const {
         return t2ns() != (uint64_t) rhs;
      }

      template <typename T> 
      inline bool operator<=(const T rhs) const {
         return t2ns() <= (uint64_t) rhs;
      }

      template <typename T> 
      inline bool operator>=(const T rhs) const {
         return t2ns() >= (uint64_t) rhs;
      }

      template <typename T> 
      inline bool operator<(const T rhs) const {
         return t2ns() < (uint64_t) rhs;
      }

      template <typename T> 
      inline bool operator>(const T rhs) const {
         return t2ns() > (uint64_t) rhs;
      }

      inline crts& operator+=(const crts& rhs) {
         if (precision == NS_IN_ONE_SEC) {
            ns2ts(t2ns()+rhs.t2ns());
         }
         else {
            ns2tv(t2ns()+rhs.t2ns());
         }

         return *this;
      }

      inline crts& operator-=(const crts& rhs) {
         if (precision == NS_IN_ONE_SEC) {
            ns2ts(t2ns()-rhs.t2ns());
         }
         else {
            ns2tv(t2ns()-rhs.t2ns());
         }

         return *this;
      }

      template <typename T> 
      inline crts& operator+=(const T rhs) {
         if (precision == NS_IN_ONE_SEC) {
            ns2ts(t2ns()+(uint64_t) rhs);
         }
         else {
            ns2tv(t2ns()+(uint64_t) rhs);
         }

         return *this;
      }

      template <typename T> 
      inline crts& operator-=(const T rhs) {
         if (precision == NS_IN_ONE_SEC) {
            ns2ts(t2ns()-(uint64_t)rhs);
         }
         else {
            ns2tv(t2ns()-(uint64_t)rhs);
         }

         return *this;
      }

      inline crts& operator=(const string rhs) {
         return operator=(rhs.c_str());
      }

      inline crts& operator=(const string& rhs) {
         return operator=(rhs.c_str());
      }

      inline crts& operator=(const char* rhs) {
         struct tm tn;
         const char *p;

         memset(&tn,'\0', sizeof(tn));

         // RFC 1123, 822
         p = strptime(CRX_TIFNULL(rhs), "%a, %d %b %Y %H:%M:%S %z", &tn);

         // ISO 8601
         if (p == NULL) {
            p = strptime(rhs, "%Y-%m-%d %H:%M:%S %z", &tn);
         }

         // PGSQL
         if (p == NULL) {
            p = strptime(rhs, "%Y-%m-%d %H:%M:%S", &tn);
         }

         if (p == NULL) {
            CRX_THROW("invalid time format");
         }

         time_t tt = mktime(&tn);

         if (tt == -1) {
            CRX_THROW("invalid time format");
         }

         ns2ts(tt*NS_IN_ONE_SEC);
         return *this;
      }

      inline struct tm* gmt(struct tm* res) {
         time_t s = sec();
         return gmtime_r(&s, CRX_TIFNULL(res));
      }

      inline string& gmt(const char* format, string& res) {
         struct tm tn;
         gmt(&tn);

         // could have up to 2 leap seconds, but leap seconds don't get accounted for in other software, 
         // so round down to 59 we don't care that much
         if (tn.tm_sec > 59) {
            tn.tm_sec = 59;
         } 

         char buf[128];
         strftime(buf, 128, format, &tn);
         res = buf;
         return res;
      }

      inline string& rfc1123(string& res) {
         return gmt("%a, %d %b %Y %H:%M:%S GMT", res);
      }

      inline string& rfc822(string& res) {
         return gmt("%a, %d %b %Y %H:%M:%S %z", res);
      }

      inline string& iso8601(string& res) {
         return gmt("%Y-%m-%d %H:%M:%S +00:00", res);
      }

      inline string& sql(string& res) {
         return iso8601(res);
      }

   } CRTime;
};

#endif
