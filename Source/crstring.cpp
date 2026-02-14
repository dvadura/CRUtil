/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @file crstring.cpp
 *  @brief Implementation of safe C-string manipulation utilities
 *
 *  @details Provides implementations for the CRS class methods defined in crstring.h.
 *           See crstring.h for full API documentation and usage examples.
 */

#include "crexception.h"
#include "crstring.h"

using namespace std;
using namespace crutil;

const char* CRS::WS = " \t\n\r\f\v";

/// Private static methods
bool 
CRS::_validate(const char* str, size_t size, size_t& len)
{
   if (unlikely(size <= len)) {
      CRX_THROW("string length exceeds buffer size");
   }

   if(unlikely(*str == '\0')) {
      len = 0;
      return true;
   }

   if(unlikely(len == 0)) {
      return true;
   }

   return false;
}

char* 
CRS::_dortrim(char* str, const char* ws, size_t size, size_t& len) {
   if (unlikely(_validate(str, size, len))) {
      return str;
   }

   char* endp = str+len;
   
   do {
      if (likely(strchr(ws, *--endp) == NULL)) {
         break;
      }
      len--;
   }
   while(len > 0);

   return endp;
}

char* 
CRS::_doltrim(char* str, const char* ws, size_t size, size_t& len) {
   if (unlikely(_validate(str, size, len))) {
      return str;
   }

   char* frontp = str;

   do {
      if (likely(strchr(ws, *frontp) == NULL)) {
         break;
      }
      frontp++;
      len--;
   }
   while(len > 0);

   return frontp;
}

char*
CRS::_strncpy(char* dst, size_t size, const char* src)
{
   CRX_TIFNULL(src);

   // use loop so we don't overrun size
   for (; *dst != '\0' && size > 0; ++dst, --size);

   if (unlikely(size == 0)) {
      CRX_THROW("buffer overflow");
   }

   int len = (int)(size - 1);

   ::strncpy(dst, src, len)[len] = '\0';
   return dst;
}

/// Public static methods
///
int 
CRS::vsnprintf(char* str, size_t size, const char* format, va_list args) {
   // note vsnprintf(...) always includes null terminating byte.
   //return ::vsnprintf(CRX_TIFNULL(str), size, CRX_TIFNULL(format), args);
   CRX_TIFNULL(str);
   CRX_TIFNULL(format);
   return ::vsnprintf(str, size, format, args);
}

int
CRS::snprintf(char* str, size_t size, const char* format, ...) {
   va_list args;

   CRX_TIFNULL(format);
   va_start(args, format);
   int result = vsnprintf(str, size, format, args);
   va_end(args);

   return result;
}

char* 
CRS::strncpy(char* dst, const char* src, size_t size)
{
   CRX_TIFNULL(dst);

   if (unlikely(size == 0)) {
      return dst;
   }

   *dst = '\0';
   return _strncpy(dst, size, src);
}

char* 
CRS::join(char* dst, const char* src, size_t size)
{
   return _strncpy(dst, size, src);
}

char*
CRS::add(char* dst, const char* src, size_t size)
{
   char* base = dst;
   char* end  = _strncpy(dst, size, " ");
   size_t used = (size_t)(end + 1 - base);

   if (unlikely(used >= size)) {
      CRX_THROW("buffer overflow");
   }

   return _strncpy(end + 1, size - used, src);
}

const char* 
CRS::skip(const char* src, const char* match, bool accept) 
{
   size_t skip = ((accept == true) ? ::strspn(CRX_TIFNULL(src), match) : ::strcspn(CRX_TIFNULL(src), match));
   return src+skip;
}

char* 
CRS::skip(char* src, const char* match, bool accept)
{
   size_t skip = ((accept == true) ? ::strspn(CRX_TIFNULL(src), match) : ::strcspn(CRX_TIFNULL(src), match));
   return src+skip;
}


/// Trim functions, with move and in-place result semantics.
/// Trim with move semantics can be used to free the string.
char* 
CRS::ltrim(char* str, const char* ws) {
   size_t len = strlen(CRX_TIFNULL(str));
   const char* front = _doltrim(str, CRX_TIFNULL(ws), len+1, len);
   
   if (str != front) { 
      ::memmove(str,front,len);
   }
   str[len]='\0';

   return str;
}

/// Move semantics for Trim
char* 
CRS::ltrim(char* str, size_t& len, const char* ws) {
   len = strlen(CRX_TIFNULL(str));
   const char* front = _doltrim(str, CRX_TIFNULL(ws), len+1, len);
   
   if (str != front) { 
      ::memmove(str,front,len);
   }
   str[len] = '\0';

   return str;
}

/// Move semantics with check for buffer overrun of string
char* 
CRS::ltrim(char* str, size_t size, size_t& len, const char* ws) {
   len = strlen(CRX_TIFNULL(str));
   const char* front = _doltrim(str,CRX_TIFNULL(ws), size, len);
   
   if (str != front) { 
      ::memmove(str,front,len);
   }
   str[len] = '\0';

   return str;
}

/// Trim functions, with move and in-place result semantics.
/// Trim with move semantics can be used to free the string.
char*
CRS::rtrim(char* str, const char* ws) {
   size_t len = strlen(CRX_TIFNULL(str));

   char* end = _dortrim(str, CRX_TIFNULL(ws), len+1, len);
   if (len == 0) {
      *str = '\0';
   }
   else {
      *++end = '\0';
   }

   return str;
}

/// Move semantics for Trim
char*
CRS::rtrim(char* str, size_t& len, const char* ws) {
   len = strlen(CRX_TIFNULL(str));

   char* end = _dortrim(str, CRX_TIFNULL(ws), len+1, len);
   if (len == 0) {
      *str = '\0';
   }
   else {
      *++end = '\0';
   }

   return str;
}

/// Move semantics with check for buffer overrun of string
char*
CRS::rtrim(char* str, size_t size, size_t& len, const char* ws) {
   len = strlen(CRX_TIFNULL(str));

   char* end = _dortrim(str, CRX_TIFNULL(ws), size, len);
   if (len == 0) {
      *str = '\0';
   }
   else {
      *++end = '\0';
   }

   return str;
}

char* 
CRS::trim(char* str, const char* ws)
{
   return rtrim(ltrim(str,ws),ws);
}

char* 
CRS::trim(char* str, size_t& len, const char* ws)
{
   return rtrim(ltrim(str,len, ws), len, ws);
}

char* 
CRS::trim(char* str, size_t size, size_t& len, const char* ws)
{
   return rtrim(ltrim(str,size, len, ws), size, len, ws);
}

// chop the right end string.
string&
CRS::rtrim(string& str, const char* ws)
{
   if (str.empty() == true) {
      return str;
   }

   return str.erase(str.find_last_not_of(ws) + 1);
}

// chop the left end of the string
string&
CRS::ltrim(string& str, const char* ws)
{
   if (str.empty() == true) {
      return str;
   }

   return str.erase(0, str.find_first_not_of(ws));
}

// generic trim.
string&
CRS::trim(string& str, const char* ws)
{
   return rtrim(ltrim(str, ws), ws);
}

// replace where result does not modify source
string
CRS::replace(const string& subject, const string& search, const string& replace) 
{
   string result(subject);
   size_t pos = 0;

   while((pos = result.find(search, pos)) != string::npos) {
      result.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return result;
}

bool
CRS::eos(const char* str) {
   return (*CRX_TIFNULL(str) == '\0');
}

bool
CRS::noe(const char* str) {
   return (str == NULL) || eos(str);
}

bool
CRS::empty(const char* str) {
   return eos(str);
}

bool
CRS::empty(const string& str) {
   return str.empty();
}

void
CRS::clear(char* str) {
   CRX_TIFNULL(str)[0] = '\0';
}

// do the splits
vector<string>&
CRS::split(const string& str, vector<string>& result, const string& delim) {
   result.clear();

   if (delim.empty()) {
      result.push_back(str);
      return result;
   }

   string::size_type start = str.find_first_not_of(delim,0);
   string::size_type end   = str.find_first_of(delim, start);
 
   while (string::npos != end || string::npos != start) {
      result.push_back(str.substr(start,end-start));
      start = str.find_first_not_of(delim, end);
      end = str.find_first_of(delim, start);
   }

   return result;
}

vector<string>&
CRS::split(const string& str, vector<string>& result, const char* delim) {
   return split(str, result, string(CRX_TIFNULL(delim)));
}

vector<string>&
CRS::split(const char* str, vector<string>& result, const char* delim) {
   return split(string(CRX_TIFNULL(str)), result, string(CRX_TIFNULL(delim)));
}
