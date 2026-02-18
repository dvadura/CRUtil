#include "catch2.hpp"
#include "lstring.h"

using namespace crutil;

TEST_CASE("LString::encode produces non-plaintext data", "[lstring]") {
   constexpr auto enc = LString::encode("hello");

   // encoded bytes should differ from the original literal
   bool all_same = true;
   const char* plain = "hello";
   for (std::size_t i = 0; i < 5; ++i) {
      if (enc.data[i] != plain[i]) {
         all_same = false;
         break;
      }
   }
   REQUIRE_FALSE(all_same);
}

TEST_CASE("LString::decode recovers original string", "[lstring]") {
   constexpr auto enc = LString::encode("this is a test");

   std::string result = LString::decode(enc);

   REQUIRE(result == "this is a test");
}

TEST_CASE("LString encode/decode round-trip for empty string", "[lstring]") {
   constexpr auto enc = LString::encode("");

   std::string result = LString::decode(enc);

   REQUIRE(result.empty());
}

TEST_CASE("LString encode/decode round-trip for single char", "[lstring]") {
   constexpr auto enc = LString::encode("A");

   std::string result = LString::decode(enc);

   REQUIRE(result == "A");
}

TEST_CASE("lstring stores correct size", "[lstring]") {
   constexpr auto s = LString::encode("test");

   // N includes the null terminator, so "test" -> lstring<5>
   REQUIRE(sizeof(s.data) == 5);
}
