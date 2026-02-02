#include "gtest/gtest.h"
#include "rqlist.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class RQListTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      RQListTest() {
         // Create the listener IPv4 socket and start the server
      }

      virtual ~RQListTest() {
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

   // Tests that the Pipe works
   TEST_F(RQListTest, Basic) {
      bool success;
      int result;
      RQList<int, 16> s;
      ASSERT_EQ(s.empty(), true);
      s.push_back(5);
      result = s.remove_front(&success);
      ASSERT_EQ(success,true);
      ASSERT_EQ(result, 5);
      result = s.remove_front(&success);
      ASSERT_EQ(success, false);

fprintf(stderr, "size is %lu\n", s.size());

      for (int i=0; i < 18; ++i) {
         if (i < 16) {
            ASSERT_EQ(s.push_back(i+1), true);
         }
         else {
            ASSERT_EQ(s.push_back(i+1), false);
         }
      }

fprintf(stderr, "size is %lu\n", s.size());

      for (int i=0; i < 16; ++i) {
         success = false;
         result = s.remove_front(&success);
         ASSERT_EQ(success, true);
         ASSERT_EQ(result, i+1);
      }
   
fprintf(stderr, "size is %lu\n", s.size());

      ASSERT_EQ(s.size(), 0);
      ASSERT_EQ(s.empty(), false);
      result = s.remove_front(&success);
      ASSERT_EQ(success, true);
      ASSERT_EQ(s.empty(), true);
      result = s.remove_front(&success);
      ASSERT_EQ(success, false);
      ASSERT_EQ(s.empty(), true);
 
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
