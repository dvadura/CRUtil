#include "catch2.hpp"
#include "crstring.h"
#include "semaphore.h"

using namespace std;
using namespace crunnable;

// --- strncpy ---

TEST_CASE("CRS::strncpy throws on NULL dst", "[crstring]") {
   char s[10];
   REQUIRE_THROWS_AS(CRS::strncpy(NULL, s, 2), CRException);
}

TEST_CASE("CRS::strncpy throws on NULL src", "[crstring]") {
   char d[10];
   REQUIRE_THROWS_AS(CRS::strncpy(d, NULL, 2), CRException);
}

TEST_CASE("CRS::strncpy copies empty string", "[crstring]") {
   char s[10] = "";
   char d[10];
   REQUIRE_NOTHROW(CRS::strncpy(d, s, 2));
   REQUIRE_NOTHROW(CRS::strncpy(d, s, 10));
}

TEST_CASE("CRS::strncpy truncates to buffer size", "[crstring]") {
   char s[10];
   REQUIRE_NOTHROW(CRS::strncpy(s, "this is a test of strncpy", 10));
   REQUIRE(strcmp(s, "this is a") == 0);
}

TEST_CASE("CRS::strncpy truncates small buffer", "[crstring]") {
   char s[10];
   char d[10];
   CRS::strncpy(s, "this is a test of strncpy", 10);
   REQUIRE_NOTHROW(CRS::strncpy(d, s, 3));
   REQUIRE(strcmp(d, "th") == 0);
}

// --- snprintf ---

TEST_CASE("CRS::snprintf throws on NULL dst", "[crstring]") {
   REQUIRE_THROWS_AS(CRS::snprintf(NULL, 20, "This is a test"), CRException);
}

TEST_CASE("CRS::snprintf throws on NULL format", "[crstring]") {
   char s[20];
   REQUIRE_THROWS_AS(CRS::snprintf(s, 20, NULL), CRException);
}

TEST_CASE("CRS::snprintf formats and truncates", "[crstring]") {
   char s[20];
   REQUIRE_NOTHROW(CRS::snprintf(s, 20, "This is a %d test of the %s %s %s",
                                  20, "emergency", "broadcast", "system"));
   REQUIRE(strlen(s) == 19);
   REQUIRE(strcmp(s, "This is a 20 test o") == 0);
}

// --- join / add ---

TEST_CASE("CRS::join and add build up a string", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "hello", 100);
   CRS::join(s, " there", 100);
   CRS::add(s, "darling", 100);

   REQUIRE(strlen(s) == 19);
   REQUIRE(strcmp(s, "hello there darling") == 0);
}

// --- skip ---

TEST_CASE("CRS::skip advances past matching chars", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "hello there darling", 100);

   char* p = CRS::skip(s, "hel");
   REQUIRE(strlen(p) == 15);
   REQUIRE(strcmp(p, "o there darling") == 0);
}

TEST_CASE("CRS::skip with accept=false stops at first match", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "hello there darling", 100);

   char* p = CRS::skip(s, "t", false);
   REQUIRE(strlen(p) == 13);
   REQUIRE(strcmp(p, "there darling") == 0);
}

// --- trim (c-string) ---

TEST_CASE("CRS::ltrim throws on NULL", "[crstring]") {
   REQUIRE_THROWS_AS(CRS::ltrim((char*)NULL), CRException);
}

TEST_CASE("CRS::rtrim throws on NULL", "[crstring]") {
   REQUIRE_THROWS_AS(CRS::rtrim((char*)NULL), CRException);
}

TEST_CASE("CRS::trim throws on NULL", "[crstring]") {
   REQUIRE_THROWS_AS(CRS::trim((char*)NULL), CRException);
}

TEST_CASE("CRS::ltrim on c-string", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "   hello there darling   ", 100);
   CRS::ltrim(s);
   REQUIRE(strlen(s) == 22);
   REQUIRE(strcmp(s, "hello there darling   ") == 0);
}

TEST_CASE("CRS::rtrim on c-string", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "   hello there darling   ", 100);
   CRS::rtrim(s);
   REQUIRE(strlen(s) == 22);
   REQUIRE(strcmp(s, "   hello there darling") == 0);
}

TEST_CASE("CRS::trim on c-string", "[crstring]") {
   char s[100];
   CRS::strncpy(s, "   hello there darling   ", 100);
   CRS::trim(s);
   REQUIRE(strlen(s) == 19);
   REQUIRE(strcmp(s, "hello there darling") == 0);
}

// --- trim (std::string) ---

TEST_CASE("CRS::ltrim on std::string", "[crstring]") {
   string ss("   hello there darling   ");
   CRS::ltrim(ss);
   REQUIRE(ss.size() == 22);
   REQUIRE(ss == "hello there darling   ");
}

TEST_CASE("CRS::rtrim on std::string", "[crstring]") {
   string ss("   hello there darling   ");
   CRS::rtrim(ss);
   REQUIRE(ss.size() == 22);
   REQUIRE(ss == "   hello there darling");
}

TEST_CASE("CRS::trim on std::string", "[crstring]") {
   string ss("   hello there darling   ");
   CRS::trim(ss);
   REQUIRE(ss.size() == 19);
   REQUIRE(ss == "hello there darling");
}

// --- split ---

TEST_CASE("CRS::split with mixed delimiters", "[crstring]") {
   string s("  hello there:darling");
   vector<string> v;
   CRS::split(s, v, " :");

   REQUIRE(v.size() == 3);
   REQUIRE(v[0] == "hello");
   REQUIRE(v[1] == "there");
   REQUIRE(v[2] == "darling");
}

TEST_CASE("CRS::split by whitespace with repeated tokens", "[crstring]") {
   vector<string> v;
   CRS::split(" 0 0 0 0", v);

   REQUIRE(v.size() == 4);
   REQUIRE(v[0] == "0");
   REQUIRE(v[1] == "0");
   REQUIRE(v[2] == "0");
   REQUIRE(v[3] == "0");
}

TEST_CASE("CRS::split on empty string", "[crstring]") {
   vector<string> v;
   CRS::split("", v);
   REQUIRE(v.empty());
}

// --- empty / noe ---

TEST_CASE("CRS::empty on c-string", "[crstring]") {
   REQUIRE(CRS::empty("") == true);
   REQUIRE(CRS::empty("x") == false);
}

TEST_CASE("CRS::empty on std::string", "[crstring]") {
   REQUIRE(CRS::empty(string("")) == true);
   REQUIRE(CRS::empty(string("x")) == false);
}

TEST_CASE("CRS::noe returns true for null or empty", "[crstring]") {
   REQUIRE(CRS::noe(nullptr) == true);
   REQUIRE(CRS::noe("") == true);
   REQUIRE(CRS::noe("x") == false);
}

// --- replace ---

TEST_CASE("CRS::replace substitutes all occurrences", "[crstring]") {
   string result = CRS::replace("foo bar foo", "foo", "baz");
   REQUIRE(result == "baz bar baz");
}

TEST_CASE("CRS::replace with no match returns original", "[crstring]") {
   string result = CRS::replace("hello", "xyz", "abc");
   REQUIRE(result == "hello");
}
