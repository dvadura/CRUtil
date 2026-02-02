#include "gtest/gtest.h"
#include "lflist.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class LFListTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      LFListTest() {
         // You can do set-up work for each test here.
      }

      virtual ~LFListTest() {
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

   // Tests that the CList::* method throws if not re-entrant
   TEST_F(LFListTest, LFListCreation) {
      LFList<int> list((int) 1);
      bool success;

      ASSERT_FALSE(list.empty());
      list.clear();
      ASSERT_TRUE(list.empty());

      list.push(1);
      list.push(2);
      ASSERT_TRUE(list.size() == 2);

      list.push_back(list.remove_front(&success));
      ASSERT_TRUE(list.size()  == 2);

      list.push(0);
      ASSERT_TRUE(list.size() == 3);

      int item = list.remove(&success);
      ASSERT_TRUE(item == 2);
      ASSERT_TRUE(list.size() == 2);

      list.clear();
      ASSERT_TRUE(list.waitFor(1000000L) == false);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
