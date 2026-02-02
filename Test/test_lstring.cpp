#include "catch2.hpp"
#include "lstring.h"

using namespace crunnable;

TEST_CASE("Obfuscate XOR is its own inverse", "[obfuscate]") {
   constexpr Obfuscate o;

   SECTION("single character round-trip") {
      for (std::size_t i = 0; i < 256; ++i) {
         char c = static_cast<char>(i);
         char encoded = o(c, i);
         char decoded = o(encoded, i);
         REQUIRE(decoded == c);
      }
   }
}

TEST_CASE("Obfuscate::encode produces non-plaintext data", "[obfuscate]") {
   constexpr auto enc = Obfuscate::encode("hello");

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

TEST_CASE("Obfuscate::decode recovers original string", "[obfuscate]") {
   constexpr auto enc = Obfuscate::encode("this is a test");

   std::string result;
   Obfuscate::decode(enc.data, result);

   REQUIRE(result == "this is a test");
}

TEST_CASE("Obfuscate encode/decode round-trip for empty string", "[obfuscate]") {
   constexpr auto enc = Obfuscate::encode("");

   std::string result;
   Obfuscate::decode(enc.data, result);

   REQUIRE(result.empty());
}

TEST_CASE("Obfuscate encode/decode round-trip for single char", "[obfuscate]") {
   constexpr auto enc = Obfuscate::encode("A");

   std::string result;
   Obfuscate::decode(enc.data, result);

   REQUIRE(result == "A");
}

TEST_CASE("lstring stores correct size", "[lstring]") {
   constexpr auto s = Obfuscate::encode("test");

   // N includes the null terminator, so "test" -> lstring<5>
   REQUIRE(sizeof(s.data) == 5);
}

TEST_CASE("cstr legacy alias works identically to Obfuscate::encode", "[lstring]") {
   constexpr auto a = Obfuscate::encode("legacy test");
   constexpr auto b = cstr("legacy test");

   for (std::size_t i = 0; i < sizeof(a.data); ++i) {
      REQUIRE(a.data[i] == b.data[i]);
   }
}
