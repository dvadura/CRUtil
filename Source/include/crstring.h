/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @file crstring.h
 *  @brief Safe C-string manipulation utilities with buffer overflow protection
 *
 *  @details Provides the CRS static class with methods for safe string operations.
 *           See CRS class documentation for detailed API reference.
 */

#ifndef __CRSTRING_INC__
#define __CRSTRING_INC__

#include "crexception.h"
#include <vector>

/** @def CRSnprintf(DST, FMT, ...)
 *  @brief Safe snprintf wrapper using sizeof(DST) for automatic buffer sizing
 *  @param DST Character array buffer (not pointer)
 *  @param FMT Format string
 *  @param ... Variable arguments for format string
 *  @returns Number of characters written (cast to size_t)
 */
#define CRSnprintf(DST,FMT,...)   ((size_t) CRS::snprintf(DST,sizeof(DST),FMT,## __VA_ARGS__))

/** @def CRStrcpy(DST, SRC)
 *  @brief Safe string copy wrapper using sizeof(DST) for automatic buffer sizing
 *  @param DST Character array buffer to copy into (not pointer)
 *  @param SRC Source string to copy from
 *  @returns Pointer to DST
 */
#define CRStrcpy(DST,SRC)         CRS::strncpy(DST,SRC,sizeof(DST))

/** @def CRStradd(DST, SRC)
 *  @brief Safe string concatenation with space separator using sizeof(DST)
 *  @param DST Character array buffer to append to (not pointer)
 *  @param SRC Source string to append
 *  @returns Pointer to the appended content
 */
#define CRStradd(DST,SRC)         CRS::add(DST,SRC,sizeof(DST))

/** @def CRStrjoin(DST, SRC)
 *  @brief Safe string concatenation without separator using sizeof(DST)
 *  @param DST Character array buffer to append to (not pointer)
 *  @param SRC Source string to append
 *  @returns Pointer to the appended content
 */
#define CRStrjoin(DST,SRC)        CRS::join(DST,SRC,sizeof(DST))

namespace crutil {
   /** @class CRS
    *  @brief Safe C-string manipulation utilities with buffer overflow protection
    *
    *  @details Provides a collection of static utility functions for safe string
    *           operations on fixed-size C strings, including trimming, splitting,
    *           replacement, joining, and formatted printing. All functions validate
    *           buffer sizes and throw CRException on violations.
    *
    *           Key characteristics:
    *           - Buffer overflow protection: all operations respect buffer sizes
    *           - Exception-based error handling via CRException
    *           - Multiple overloads: C-string arrays, sized buffers, std::string
    *           - Macro wrappers for automatic buffer sizing (CRSnprintf, CRStrcpy, etc.)
    *
    *           Usage example:
    *           @code
    *           char str[100];
    *           CRS::strncpy(str, "hello", sizeof(str));
    *           CRS::join(str, " world", sizeof(str));
    *           CRS::rtrim(str);
    *           @endcode
    *
    *  @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
    *  @see     https://github.com/dvadura/CRUtil
    */
   class CRS {
   private:
      static char* _dortrim(char* str, const char* ws, size_t size, size_t& len);
      static char* _doltrim(char* str, const char* ws, size_t size, size_t& len);
      static char* _strncpy(char* dst, size_t size, const char* src);
      static bool _validate(const char* str, size_t size, size_t& len);

   public:
      /** @brief Default whitespace characters used by trim functions */
      static const char* WS;

      /// @name Safe Formatting Operations
      /// Functions for safe formatted printing with buffer overflow protection
      /// @{

      /** @brief Safe snprintf with buffer overflow protection
       *  @param [out] str Output buffer
       *  @param [in] size Size of output buffer
       *  @param [in] format Format string
       *  @param [in] ... Variable arguments for format string
       *  @returns Number of characters written (excluding null terminator)
       *  @throws CRException if str or format is NULL
       */
      static int snprintf(char* str, size_t size, const char* format, ...);

      /** @brief Safe vsnprintf with buffer overflow protection
       *  @param [out] str Output buffer
       *  @param [in] size Size of output buffer
       *  @param [in] format Format string
       *  @param [in] args Variable argument list
       *  @returns Number of characters written (excluding null terminator)
       *  @throws CRException if str or format is NULL
       */
      static int vsnprintf(char* str, size_t size, const char* format, va_list args);

      /** @brief Safe string copy with buffer overflow protection
       *  @param [out] dst Destination buffer
       *  @param [in] src Source string to copy
       *  @param [in] size Size of destination buffer
       *  @returns Pointer to destination buffer
       *  @throws CRException if dst is NULL, src is NULL, or buffer overflow would occur
       */
      static char* strncpy(char* dst, const char* src, size_t size);

      /// @}

      /// @name Trimming Operations
      /// Functions for removing whitespace from string boundaries
      /// @{

      /** @brief Trim whitespace from left side of C-string (in-place)
       *  @param [in,out] str String to trim (modified in-place with memmove)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* ltrim(char* str, const char* ws=WS);

      /** @brief Trim whitespace from left side of C-string and return new length
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* ltrim(char* str, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from left side with buffer size validation
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] size Maximum buffer size for overflow detection
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL or string exceeds buffer size
       */
      static char* ltrim(char* str, size_t size, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from right side of C-string (in-place)
       *  @param [in,out] str String to trim (null terminator moved left)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* rtrim(char* str, const char* ws=WS);

      /** @brief Trim whitespace from right side of C-string and return new length
       *  @param [in,out] str String to trim (null terminator moved left)
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* rtrim(char* str, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from right side with buffer size validation
       *  @param [in,out] str String to trim (null terminator moved left)
       *  @param [in] size Maximum buffer size for overflow detection
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL or string exceeds buffer size
       */
      static char* rtrim(char* str, size_t size, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from both sides of C-string (in-place)
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* trim(char* str, const char* ws=WS);

      /** @brief Trim whitespace from both sides and return new length
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL
       */
      static char* trim(char* str, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from both sides with buffer size validation
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] size Maximum buffer size for overflow detection
       *  @param [out] len Updated length of string after trimming
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Pointer to the trimmed string (same as input)
       *  @throws CRException if str is NULL or string exceeds buffer size
       */
      static char* trim(char* str, size_t size, size_t& len, const char* ws=WS);

      /** @brief Trim whitespace from left side of std::string (in-place)
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Reference to the trimmed string (same as input)
       */
      static string& ltrim(string& str, const char* ws=WS);

      /** @brief Trim whitespace from right side of std::string (in-place)
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Reference to the trimmed string (same as input)
       */
      static string& rtrim(string& str, const char* ws=WS);

      /** @brief Trim whitespace from both sides of std::string (in-place)
       *  @param [in,out] str String to trim (modified in-place)
       *  @param [in] ws Whitespace characters to trim (default: " \\t\\n\\r\\f\\v")
       *  @returns Reference to the trimmed string (same as input)
       */
      static string& trim(string& str, const char* ws=WS);

      /// @}
      /// @name String Manipulation Operations
      /// Functions for modifying, searching, and splitting strings
      /// @{

      /** @brief Replace all occurrences of a substring in a std::string
       *  @param [in] subject Original string to search within
       *  @param [in] search Substring to search for
       *  @param [in] replace Replacement string
       *  @returns New string with all occurrences replaced (original unchanged)
       */
      static string  replace(const string& subject, const string& search, const string& replace);

      /** @brief Skip characters matching (or not matching) a character set
       *  @param [in] str String to scan
       *  @param [in] match Character set to match against (default: whitespace)
       *  @param [in] accept If true, skip matching chars; if false, skip non-matching
       *  @returns Pointer to first character that breaks the skip condition
       *  @throws CRException if str is NULL
       */
      static       char* skip(char* str, const char* match=WS, bool accept=true);

      /** @brief Skip characters matching (or not matching) a character set (const version)
       *  @param [in] str String to scan
       *  @param [in] match Character set to match against (default: whitespace)
       *  @param [in] accept If true, skip matching chars; if false, skip non-matching
       *  @returns Pointer to first character that breaks the skip condition
       *  @throws CRException if str is NULL
       */
      static const char* skip(const char* str, const char* match=WS, bool accept=true);

      /** @brief Concatenate a string to an existing buffer without separator
       *  @param [in,out] dst Destination buffer (appends to existing content)
       *  @param [in] str Source string to append
       *  @param [in] size Size of destination buffer (0 means use strlen+1)
       *  @returns Pointer to the appended content within dst
       *  @throws CRException if dst or str is NULL, or buffer overflow would occur
       */
      static char* join(char* dst, const char* str, size_t size=0);

      /** @brief Concatenate a string with space separator
       *  @param [in,out] dst Destination buffer (appends space then content)
       *  @param [in] str Source string to append
       *  @param [in] size Size of destination buffer (0 means use strlen+1)
       *  @returns Pointer to the appended content within dst (after the space)
       *  @throws CRException if dst or str is NULL, or buffer overflow would occur
       */
      static char* add(char* dst, const char* str, size_t size=0);

      /** @brief Split a C-string into tokens based on delimiters
       *  @param [in] str String to split
       *  @param [out] result Vector to store split tokens (cleared before use)
       *  @param [in] delim Delimiter characters (default: whitespace)
       *  @returns Reference to result vector
       *  @throws CRException if str or delim is NULL
       */
      static vector<string>& split(const char* str,   vector<string>& result, const char* delim=WS);

      /** @brief Split a std::string into tokens based on delimiter characters
       *  @param [in] str String to split
       *  @param [out] result Vector to store split tokens (cleared before use)
       *  @param [in] delim Delimiter characters (default: whitespace)
       *  @returns Reference to result vector
       *  @throws CRException if delim is NULL
       */
      static vector<string>& split(const string& str, vector<string>& result, const char* delim=WS);

      /** @brief Split a std::string into tokens based on delimiter string
       *  @param [in] str String to split
       *  @param [out] result Vector to store split tokens (cleared before use)
       *  @param [in] delim Delimiter string
       *  @returns Reference to result vector
       */
      static vector<string>& split(const string& str, vector<string>& result, const string& delim);

      /// @}

      /// @name Utility Operations
      /// Functions for testing and manipulating string state
      /// @{

      /** @brief Test if string is at end-of-string (null terminator)
       *  @param [in] str String to test
       *  @returns True if *str == '\\0', false otherwise
       *  @throws CRException if str is NULL
       */
      static bool eos(const char* str);

      /** @brief Test if string is NULL or empty (null-or-empty)
       *  @param [in] str String to test
       *  @returns True if str is NULL or *str == '\\0', false otherwise
       */
      static bool noe(const char* str);

      /** @brief Test if C-string is empty
       *  @param [in] str String to test
       *  @returns True if *str == '\\0', false otherwise
       *  @throws CRException if str is NULL
       */
      static bool empty(const char* str);

      /** @brief Test if std::string is empty
       *  @param [in] str String to test
       *  @returns True if str.empty() == true, false otherwise
       */
      static bool empty(const string& str);

      /** @brief Clear a C-string by setting first character to null terminator
       *  @param [out] str String to clear
       *  @throws CRException if str is NULL
       */
      static void clear(char* str);

      /** @brief Throw exception if pointer-to-string is empty, otherwise return it
       *  @tparam T String type (const char*, char*, etc.)
       *  @param [in] str String pointer to validate
       *  @returns The input pointer if not empty
       *  @throws CRException with message "empty string" if empty
       */
      template <typename T>
      static T* throwifempty(T* str) {
         if (likely(empty(str) == false)) {
            return str;
         }

         CRX_THROW("empty string");
      }

      /** @brief Throw exception if string reference is empty, otherwise return it
       *  @tparam T String type (string, etc.)
       *  @param [in] str String reference to validate
       *  @returns The input reference if not empty
       *  @throws CRException with message "empty string" if empty
       */
      template <typename T>
      static T& throwifempty(T& str) {
         if (likely(empty(str) == false)) {
            return str;
         }

         CRX_THROW("empty string");
      }

      /// @}
   };
};

#endif
