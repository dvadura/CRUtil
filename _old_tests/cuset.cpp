#include "gtest/gtest.h"
#include "cuset.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class CUSetTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      CUSetTest() {
         // You can do set-up work for each test here.
      }

      virtual ~CUSetTest() {
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

   // Tests that the CUSet::* method throws if not re-entrant
   TEST_F(CUSetTest, CUSetIndexing) {
      CUSet<int> set;
      CUSet<int> set2;

      set.add(1);
      set2.add(1);
      ASSERT_FALSE(set.empty());
      set.clear();
      ASSERT_TRUE(set.empty());

      set.add(1);
      set.add(2);
      ASSERT_TRUE(set.size() == 2);

      ASSERT_TRUE(set.contains(2));

      set2.add(4);
      set2.add(4);
      set2.add(4);

      try {
         set.splice(std::move(set2));
      }
      catch (CRException& crx) {
         CRX_REPORT_CATCH(stderr, crx);
      }

      ASSERT_TRUE(set.size() == 3);
      ASSERT_TRUE(set2.size() == 0);
      ASSERT_TRUE(set2.empty() == true);

      ASSERT_TRUE(set.remove(4) == 1);
      ASSERT_TRUE(set.remove(4) == 0);
      ASSERT_TRUE(set.size() == 2);

      set.clear();
      ASSERT_TRUE(set.waitFor(1000000L) == false);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
