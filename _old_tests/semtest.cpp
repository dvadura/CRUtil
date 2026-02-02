#include "gtest/gtest.h"
#include "semaphore.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class SemaphoreTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     SemaphoreTest() {
       // You can do set-up work for each test here.
     }

     virtual ~SemaphoreTest() {
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

   // Tests that the Semaphore::PP method throws if not re-entrant
   TEST_F(SemaphoreTest, NonReentrantSemaphorePThrow) {
      Semaphore sem(false);

      EXPECT_NO_THROW({
         sem.PP;
      });

      ASSERT_THROW(sem.PP, CRException);
      
      EXPECT_NO_THROW({
         sem.VV;
      });
   }

   // Tests that the Semaphore::VV method throws if not re-entrant
   TEST_F(SemaphoreTest, NonReentrantSemaphoreVThrow) {
      Semaphore sem(true);

      EXPECT_NO_THROW({
         sem.PP;
         sem.VV;
      });

      ASSERT_THROW(sem.VV, CRException);
   }

   // Tests that the Semaphore::VV method throws if re-entrant
   TEST_F(SemaphoreTest, ReentrantSemaphoreVThrow) {
      Semaphore sem(true);

      EXPECT_NO_THROW({
         sem.PP;
         sem.PP;
         sem.VV;
         sem.VV;
      });

      ASSERT_THROW(sem.VV, CRException);
   }

   // Tests that the Semaphore::VV dumps catch message if verbose and in error
   TEST_F(SemaphoreTest, ReentrantSemaphoreVDump) {
      Semaphore sem(true, true, "RSEM");

      EXPECT_NO_THROW({
         sem.PP;
         sem.PP;
         sem.VV;
         sem.VV;
      });

      // this should cause a dump of the error.
      fprintf(stderr,"\n===================> Expect a FAIL for V with call stack dump here");
      ASSERT_THROW(sem.VV, CRException);
   }

   // Tests that the Semaphore::VV dumps catch message if verbose and in error
   TEST_F(SemaphoreTest, NormalSemaphoreVDump) {
      Semaphore sem(false, true, "NSEM");

      sem.PP;
      // this should cause a dump of the error.
      fprintf(stderr,"\n===================> Expect a FAIL for P with call stack dump here");
      ASSERT_THROW(sem.PP, CRException);
      sem.VV;

      // this should cause a dump of the error.
      fprintf(stderr,"\n===================> Expect a FAIL for V with call stack dump here");
      ASSERT_THROW(sem.VV, CRException);

#ifdef SEMTRACE
      SEMTRACEDUMP(stderr);
#endif
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
