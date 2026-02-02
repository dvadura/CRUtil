#include "gtest/gtest.h"
#include "crstring.h"
#include "semaphore.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class CRStringTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     CRStringTest() {
       // You can do set-up work for each test here.
     }

     virtual ~CRStringTest() {
       // You can do clean-up work that doesn't throw exceptions here.
     }

     // If the constructor and destructor are not enough for setting up
     // and cleaning up each test, you can define the following methods:

     virtual void SetUp() {
       // Code here will be called immediately after the constructor (right
       // before each test).
     }

     virtual void TearDown() {
       // Code here will be called immediately after each test (right
       // before the destructor).
     }

     // Objects declared here can be used by all tests in the test case for Foo.
   };

   // Tests that the Semaphore::P() method throws if not re-entrant
   TEST_F(CRStringTest, StrncpyOperations) {
      char s[10];
      char d[10];

      ASSERT_THROW(CRS::strncpy(NULL,s,2), CRException);
      ASSERT_THROW(CRS::strncpy(d,NULL,2), CRException);
      s[0] = '\0';
      ASSERT_NO_THROW(CRS::strncpy(d,s,2));
      ASSERT_NO_THROW(CRS::strncpy(d,s,10));

      ASSERT_NO_THROW(CRS::strncpy(s,"this is a test of strncpy",10));
      ASSERT_TRUE(strcmp(s,"this is a") == 0);

      ASSERT_NO_THROW(CRS::strncpy(d,s,3));
      ASSERT_TRUE(strcmp(d,"th") == 0);
   }

   TEST_F(CRStringTest, SnprintfOperations) {
      char s[20];

      ASSERT_THROW(CRS::snprintf(NULL, 20, "This is a test of the emergency broadcast system"), CRException);
      ASSERT_THROW(CRS::snprintf(s, 20, NULL), CRException);

      ASSERT_NO_THROW(CRS::snprintf(s, 20, "This is a %d test of the %s %s %s", 20, "emergency", "broadcast", "system"));

      ASSERT_TRUE(strlen(s) == 19);
      ASSERT_TRUE(strcmp(s,"This is a 20 test o") == 0);
   }

   TEST_F(CRStringTest, AddOperations) {
      char s[100];

      CRS::strncpy(s,"hello",100);
      CRS::join(s," there", 100);
      CRS::add(s,"darling", 100);
      
      ASSERT_TRUE(strlen(s) == 19);
      ASSERT_TRUE(strcmp(s,"hello there darling") == 0);
   }

   TEST_F(CRStringTest, SkipOperations) {
      char s[100];

      CRS::strncpy(s,"hello there darling",100);
      
      char *p = CRS::skip(s, "hel");
      ASSERT_TRUE(strlen(p) == 15);
      ASSERT_TRUE(strcmp(p,"o there darling") == 0);

      p = CRS::skip(s, "t", false);
      ASSERT_TRUE(strlen(p) == 13);
      ASSERT_TRUE(strcmp(p,"there darling") == 0);
   }

   TEST_F(CRStringTest, TrimOperations) {
      char s[100];

      ASSERT_THROW(CRS::ltrim(NULL), CRException);
      ASSERT_THROW(CRS::rtrim(NULL), CRException);
      ASSERT_THROW(CRS::trim(NULL), CRException);

      CRS::strncpy(s, "   hello there darling   ", 100);
      CRS::ltrim(s);
      ASSERT_TRUE(strlen(s) == 22);
      ASSERT_TRUE(strcmp(s,"hello there darling   ") == 0);
      
      CRS::strncpy(s,"   hello there darling   ",100);
      CRS::rtrim(s);
      ASSERT_TRUE(strlen(s) == 22);
      ASSERT_TRUE(strcmp(s,"   hello there darling") == 0);
      
      CRS::strncpy(s,"   hello there darling   ",100);
      CRS::trim(s);
      ASSERT_TRUE(strlen(s) == 19);
      ASSERT_TRUE(strcmp(s,"hello there darling") == 0);

      string dd("   hello there darling   ");
      string ss = dd;

      CRS::ltrim(ss);
      ASSERT_TRUE(ss.size() == 22);
      ASSERT_TRUE(ss == "hello there darling   ");

      ss = dd;
      CRS::rtrim(ss);
      ASSERT_TRUE(ss.size() == 22);
      ASSERT_TRUE(ss == "   hello there darling");

      ss = dd;
      CRS::trim(ss);
      ASSERT_TRUE(ss.size() == 19);
      ASSERT_TRUE(ss == "hello there darling");
   }

   TEST_F(CRStringTest, SplitOperations) {
      string s("  hello there:darling");
      vector<string> v;

      CRS::split(s, v, " :");

      ASSERT_TRUE(v.size() == 3);
      ASSERT_TRUE(v[0] == "hello");
      ASSERT_TRUE(v[1] == "there");
      ASSERT_TRUE(v[2] == "darling");

      v.clear();
      CRS::split(" 0 0 0 0", v);

      ASSERT_TRUE(v.size() == 4);
      ASSERT_TRUE(v[0] == "0");
      ASSERT_TRUE(v[1] == "0");
      ASSERT_TRUE(v[2] == "0");
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
