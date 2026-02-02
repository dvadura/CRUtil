#ifndef __CRSTRING_INC__
#define __CRSTRING_INC__

#include "crexception.h"
#include <vector>

#define CRSnprintf(DST,FMT,...)   ((size_t) CRS::snprintf(DST,sizeof(DST),FMT,## __VA_ARGS__))
#define CRStrcpy(DST,SRC)         CRS::strncpy(DST,SRC,sizeof(DST))
#define CRStradd(DST,SRC)         CRS::add(DST,SRC,sizeof(DST))
#define CRStrjoin(DST,SRC)        CRS::join(DST,SRC,sizeof(DST))

namespace crutil {
   class CRS {
   private:
      static char* _dortrim(char* str, const char* ws, size_t size, size_t& len);
      static char* _doltrim(char* str, const char* ws, size_t size, size_t& len);
      static char* _strncpy(char* dst, size_t size, const char* src);
      static bool _validate(const char* str, size_t size, size_t& len);

   public:
      static const char* WS;

      static int snprintf(char* str, size_t size, const char* format, ...);
      static int vsnprintf(char* str, size_t size, const char* format, va_list args);

      static char* strncpy(char* dst, const char* src, size_t size);

      static char* ltrim(char* str, const char* ws=WS);
      static char* ltrim(char* str, size_t& len, const char* ws=WS);
      static char* ltrim(char* str, size_t size, size_t& len, const char* ws=WS);

      static char* rtrim(char* str, const char* ws=WS);
      static char* rtrim(char* str, size_t& len, const char* ws=WS);
      static char* rtrim(char* str, size_t size, size_t& len, const char* ws=WS);

      static char* trim(char* str, const char* ws=WS);
      static char* trim(char* str, size_t& len, const char* ws=WS);
      static char* trim(char* str, size_t size, size_t& len, const char* ws=WS);

      static string& ltrim(string& str, const char* ws=WS);
      static string& rtrim(string& str, const char* ws=WS);
      static string& trim(string& str, const char* ws=WS);
      static string  replace(const string& subject, const string& search, const string& replace);

      static       char* skip(char* str, const char* match=WS, bool accept=true);
      static const char* skip(const char* str, const char* match=WS, bool accept=true);

      static char* join(char* dst, const char* str, size_t size=0);
      static char* add(char* dst, const char* str, size_t size=0);

      static vector<string>& split(const char* str,   vector<string>& result, const char* delim=WS);
      static vector<string>& split(const string& str, vector<string>& result, const char* delim=WS);
      static vector<string>& split(const string& str, vector<string>& result, const string& delim);

      static bool eos(const char* str);
      static bool noe(const char* str);
      static bool empty(const char* str);
      static bool empty(const string& str);

      static void clear(char* str);
      
      template <typename T>
      static T* throwifempty(T* str) {
         if (likely(empty(str) == false)) {
            return str;
         }

         CRX_THROW("empty string");
      }

      template <typename T>
      static T& throwifempty(T& str) {
         if (likely(empty(str) == false)) {
            return str;
         }

         CRX_THROW("empty string");
      }
   };
};

#endif
