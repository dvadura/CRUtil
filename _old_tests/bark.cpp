#include "gtest/gtest.h"
#include "bark.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class BarkTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.
      BarkTest() { }

      virtual ~BarkTest() {
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
   };

   // Tests that the Socket works
   TEST_F(BarkTest, Bark) {
      try {
         Bark watchdog;
         watchdog.setTimeout(30);
         sleep(10);
         ASSERT_EQ(20,watchdog.remain());
      }
      catch (CRException& e) {
         //CRX_REPORT_CATCH(stderr, e);
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
