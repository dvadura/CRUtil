#include "gtest/gtest.h"
#include "pipe.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class PipeTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      PipeTest() {
         // Create the listener IPv4 socket and start the server
      }

      virtual ~PipeTest() {
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
   TEST_F(PipeTest, Pipette) {
      char buf[Pipe::SYS_PAGESIZE];
      Pipe pipe;

      try {
         pipe.open();

         pipe.assertOpen("test");
         ASSERT_TRUE(pipe.isOpen());
         ASSERT_GE(pipe.in(), 0);

         ASSERT_EQ(pipe.capacity(), Pipe::MIN_SIZE);
         ASSERT_EQ(pipe.capacity(true), Pipe::MIN_SIZE);
         ASSERT_EQ(pipe.write("pipe test"),9);
         ASSERT_EQ(pipe.read(buf,9), 9);
         buf[9] = '\0';
         ASSERT_STREQ(buf, "pipe test");

         ASSERT_EQ(pipe.write("pipe test", (size_t) 10), 10);
         ASSERT_EQ(pipe.size(), 10);
         ASSERT_EQ(pipe.read(buf,10), 10);
         ASSERT_STREQ(buf, "pipe test");
         ASSERT_EQ(pipe.size(), 0);

         // note this has to use the version that expects a given length because
         // pipe output file descriptor is blocking.
         string result;
         ASSERT_EQ(pipe.write("pipe test", (size_t) 10), 10);
         ASSERT_EQ(pipe.read(result,(size_t)10), 10);
         ASSERT_STREQ(result.c_str(), "pipe test");

         // now do the same thing using a non-blocking FD on the pipe output
         // notice we no longer need to tell the read the length.
         //
         // NOTE: this will not be reliable in general as there is a race between the
         //       write and the read. In this case we always win the write race, so the
         //       pipe always has 10 bytes in it.
         //
         pipe.fdout().block(false);
         ASSERT_EQ(pipe.write("pipe test", (size_t) 10), 10);
         ASSERT_EQ(pipe.read(result), 10);
         ASSERT_STREQ(result.c_str(), "pipe test");

         pipe.close();
         ASSERT_FALSE(pipe.isOpen());
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         throw e;
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
