#include "gtest/gtest.h"
#include "ainteger.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class AIntegerTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     AIntegerTest() {
       // You can do set-up work for each test here.
     }

     virtual ~AIntegerTest() {
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
   TEST_F(AIntegerTest, ComparisonOperations) {
      AInteger x;
      AInteger y(10);

      EXPECT_EQ(x,x);
      EXPECT_NE(x,y);
      EXPECT_LT(x,y);
      EXPECT_GT(y,x);

      x = 10;
      EXPECT_EQ(x.value(), 10);

      EXPECT_LE(x,y);
      EXPECT_GE(y,x);
   }

   TEST_F(AIntegerTest, ArithmeticOperations) {
      AInteger x(10);

      x += 1;
      EXPECT_EQ(x,11);
      x -= 6;
      EXPECT_EQ(x,5);
      --x;
      EXPECT_EQ(x,4);
      ++x;
      EXPECT_EQ(x,5);
      EXPECT_EQ(x++, 5);
      EXPECT_EQ(x--, 6);
      EXPECT_EQ(x,5);

      // postfix is applied first
      ++x--;
      EXPECT_EQ(x,4);

      // postfix is applied first
      --x++;
      EXPECT_EQ(x,5);
   }

   TEST_F(AIntegerTest, CASOperations) {
      AInteger x(10);

      x.cas(11,12);
      EXPECT_EQ(x,10);
      x.cas(10,12);
      EXPECT_EQ(x,12);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
