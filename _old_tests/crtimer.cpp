#include "gtest/gtest.h"
#include "crtimer.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class CRTimersTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     CRTimersTest() {
       // You can do set-up work for each test here.
     }

     virtual ~CRTimersTest() {
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

   // Tests that the CRTime comparison opps
   TEST_F(CRTimersTest, ComparisonOperations) {
      CRTime x((uint64_t) 0);
      CRTime y((uint64_t) 0);

      EXPECT_EQ(x,y);
      y = x.now();
      y.usleep(100);
      y.now();
      uint64_t diff = y.usec()-x.usec();

      EXPECT_GT(y,x);
      EXPECT_GE(diff,100);

      CRTime z1;
      CRTime z2;

      EXPECT_NE(z1,z2);
      EXPECT_GT(z2,z1);
      EXPECT_LT(z1,z2);
      EXPECT_GE(z2,z1);
      EXPECT_LE(z1,z2);
   }

   // Tests timer conversion
   TEST_F(CRTimersTest, ConversionOperations) {
      CRTime x;
      CRTime y;
      uint64_t diff = y.nsec()-x.nsec();

      EXPECT_NE(x,y);
      EXPECT_GT(diff, 0);

      y -= x;
      EXPECT_EQ(y,diff);
   }

   // Tests timer delay
   TEST_F(CRTimersTest, DelayOperations) {
      CRTime x((uint64_t) 0);
      CRTime r((uint64_t) 0);

      ASSERT_EQ(x.t2ns(),0L);

      uint64_t start = x.now().msec();
      int delay = x.ndelay(NS_IN_ONE_MSEC*5, &r);
      uint64_t diff = x.msec()-start;

      EXPECT_EQ(delay,0);
      EXPECT_GT(diff,0);
      EXPECT_LT(diff,1000);

      delay = x.ndelay(NS_IN_ONE_MSEC*10, &r);
      CRTime y;
      diff = y.nsec()-x.nsec();

      EXPECT_EQ(delay,0);
      EXPECT_GT(diff,0);

      // diff may be large if run on system that are throttled so accomodate
      EXPECT_LT(diff,6000);

      y.now();
      usleep(10000);
      diff = y.diff();
      EXPECT_GT(diff,0);
      EXPECT_GT(diff,10000000);
      EXPECT_LT(diff,10500000);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
