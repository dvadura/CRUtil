#include "gtest/gtest.h"
#include "clist.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class CListTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      CListTest() {
         // You can do set-up work for each test here.
      }

      virtual ~CListTest() {
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
   TEST_F(CListTest, ClistIndexing) {
      CList<int> list(1);
      CList<int> list2(1);

      ASSERT_FALSE(list.empty());
      list.clear();
      ASSERT_TRUE(list.empty());

      list.add(1);
      list.add(2);
      ASSERT_TRUE(list.size() == 2);

      ASSERT_TRUE(list.front() == 1);
      ASSERT_TRUE(list.size() == 2);

      list.push_back(list.remove_front());
      ASSERT_TRUE(list.front() == 2);
      ASSERT_TRUE(list.size()  == 2);

      list.push_front(0);
      ASSERT_TRUE(list.front() == 0);
      ASSERT_TRUE(list.size() == 3);

      list2.add(4);
      list2.add(4);
      list2.add(4);

      try {
         list.splice(std::move(list2));
      }
      catch (CRException& crx) {
         CRX_REPORT_CATCH(stderr, crx);
      }

      ASSERT_TRUE(list.size() == 7);
      ASSERT_TRUE(list2.size() == 0);
      ASSERT_TRUE(list2.empty() == true);

      ASSERT_TRUE(list.remove(4) == 4);
      ASSERT_FALSE(list.size() == 5);
      ASSERT_TRUE(list.size() == 4);

      ASSERT_THROW(list[10], CRException);
      int tmp = list.pfpb();
      ASSERT_TRUE(list.size() == 4);
      ASSERT_TRUE(tmp == 0);
      ASSERT_TRUE(list.front() == 2);

      list.clear();
      ASSERT_TRUE(list.waitFor(1000000L) == false);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
