#include "gtest/gtest.h"
#include "rwlock.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class RWLockTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     RWLockTest() {
       // You can do set-up work for each test here.
     }

     virtual ~RWLockTest() {
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

   // Tests that the RWLock::try* method throws if not re-entrant
   TEST_F(RWLockTest, TryLockPThrow) {
      RWLock lock;

      EXPECT_NO_THROW({
         lock.tryrdlock();
      });

      // can't get another read lock on our lock
      ASSERT_THROW(lock.tryrdlock(), CRException);

      // can convert it to a write lock
      ASSERT_TRUE(lock.trywrlock());

      // can unlock it, we own it.
      ASSERT_NO_THROW(lock.unlock());

      ASSERT_TRUE(lock.trywrlock());
      ASSERT_THROW(lock.tryrdlock(), CRException);
      ASSERT_THROW(lock.trywrlock(), CRException);
   }

   // Tests that the RWLock::try* method throws if not re-entrant
   TEST_F(RWLockTest, LockPThrow) {
      RWLock lock;

      EXPECT_NO_THROW({
         lock.rdlock();
      });

      // can't get another read lock on our lock
      ASSERT_THROW(lock.rdlock(), CRException);
      ASSERT_THROW(lock.tryrdlock(), CRException);

      // can convert it to a write lock
      ASSERT_NO_THROW(lock.wrlock());
      ASSERT_THROW(lock.trywrlock(), CRException);

      // can unlock it, we own it.
      ASSERT_NO_THROW(lock.unlock());

      ASSERT_NO_THROW(lock.wrlock());
      ASSERT_THROW(lock.rdlock(), CRException);
      ASSERT_THROW(lock.wrlock(), CRException);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
