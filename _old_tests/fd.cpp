#include "gtest/gtest.h"
#include "fd.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class AFDTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     AFDTest() {
       // You can do set-up work for each test here.
     }

     virtual ~AFDTest() {
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
   TEST_F(AFDTest,WriteOperations) {
      FileFD fd("/tmp/test.txt");
      FileFD fe;

      ASSERT_TRUE(fd.isOpen());
      ASSERT_FALSE(fe.isOpen());
      ASSERT_THROW(fe.assertOpen("test", true), CRException);

      const char *buf="This is a chance for all the men to have a big party\n";
      size_t len=strlen(buf);

      for (int i=0; i < 1000; ++i) {
         ssize_t w = fd.write(buf,len,true);
         ASSERT_EQ(w, len);
      }

      fd.close();
   }

   TEST_F(AFDTest, ReadOperations) {
      FileFD fd("/tmp/test.txt");

      ASSERT_TRUE(fd.isOpen());
      ASSERT_FALSE(fd.isStream());
      fd.block(false);
      ASSERT_FALSE(fd.isBlocking());

      char buf[101];
      ssize_t len = 0;
      ssize_t res;

      while((res=fd.read(buf,100)) > 0) {
         len += res;
      }

      ASSERT_LE(res,0);
      ASSERT_EQ(fd.size(),len);

      ASSERT_EQ(fd.read(buf,100,true), 0);
      ASSERT_TRUE(fd.isEof());

      fd.unlink();
   }

   TEST_F(AFDTest, TIFNull) {
      ASSERT_THROW(CRX_TIFNULL((void*)NULL),CRException);
      CRX_TIFNULL("not-null");
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
