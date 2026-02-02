#include "gtest/gtest.h"
#include "epoll.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class EPollTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      EPollTest() {
         // Create the listener IPv4 socket and start the server
      }

      virtual ~EPollTest() {
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

   // Tests that Epoll descriptor set manipulation works
   // No need to test that epoll works as advertised!
   TEST_F(EPollTest, EPoller) {
      try {
         EPoll epoll;

         epoll.assertNotOpen("EPOLL: NOT OPEN");
         epoll.open();

         epoll.assertOpen("EPOLL: OPEN");
         ASSERT_EQ(epoll.fdcount(), 0);
         ASSERT_EQ(epoll.wait(100000000), 0);

         epoll.close();
         epoll.assertNotOpen("EPOLL: NOT OPEN AFTER CLOSE");
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      try {
         Pipe pipe;
         
         pipe.assertNotOpen("PIPE: NOT OPEN");
         pipe.open();


         pipe.assertOpen("PIPE: OPEN");
         pipe.close();

         pipe.assertNotOpen("PIPE: NOT OPEN AFTER CLOSE");
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      try {
         FD fd;
         ASSERT_THROW(fd.close(), CRException);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      try {
         EPoll epoll;
         Pipe  pipe;
         FD    fd;

         pipe.open();
         pipe.assertOpen("test");

         // check that we can't do anything before we open
         ASSERT_FALSE(epoll.isOpen());
         ASSERT_THROW(epoll.add(pipe), CRException);

         epoll.open();
         ASSERT_THROW(epoll.open(), CRException);
         ASSERT_EQ(epoll.add(pipe).fdcount(), 2);
         ASSERT_THROW(epoll.add(0,fd), CRException);

         ASSERT_EQ(epoll.getFD(3), &pipe.fdout());
         epoll.remove(pipe);
         ASSERT_EQ(epoll.fdcount(), 0);
         pipe.close();

         ASSERT_EQ(pipe.in(), -1);
         ASSERT_EQ(pipe.out(), -1);
         ASSERT_FALSE(pipe.isOpen());

         ASSERT_THROW(epoll.add(pipe).fdcount(), CRException);
         epoll.remove(pipe);
         ASSERT_EQ(epoll.fdcount(), 0);

         ASSERT_TRUE(epoll.isOpen());
         epoll.close();
      }
      catch (crunnable::CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         sleep(1);
         throw e;
      }
   }

   TEST_F(EPollTest, EPollDouble) {
      try {
         char buf[Pipe::SYS_PAGESIZE];
         EPoll epoll1;
         EPoll epoll2;
         Pipe  pipe;

         pipe.open();
         epoll1.open();
         epoll2.open();

         epoll1.add((EPOLLIN) & ~(EPOLLOUT), pipe.fdout());
         epoll2.add((EPOLLIN) & ~(EPOLLOUT), pipe.fdout());

         pipe.write("data");
         ASSERT_EQ(epoll1.wait(0,false),1);
         ASSERT_EQ(epoll2.wait(0,false),1);
         ASSERT_EQ(pipe.read(buf,4),4);
         ASSERT_EQ(epoll1.wait(10,false),0);
         ASSERT_EQ(epoll2.wait(10,false),0);

         epoll1.close();
         epoll2.close();
         pipe.close();
      }
      catch (crunnable::CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         sleep(1);
         throw e;
      }
   }

   TEST_F(EPollTest, EPollMigrate) {
      try {
         char buf[Pipe::SYS_PAGESIZE];
         EPoll epoll1;
         EPoll epoll2;
         Pipe  pipe;

         pipe.open();
         epoll1.open();
         epoll2.open();

         epoll1.add((EPOLLIN) & ~(EPOLLOUT), pipe.fdout());

         pipe.write("data");
         ASSERT_EQ(epoll1.wait(0,false),1);
         epoll1.remove(pipe.fdout());
         ASSERT_EQ(epoll1.wait(10,false),0);
         epoll1.close();
         
         epoll2.add((EPOLLIN) & ~(EPOLLOUT), pipe.fdout());
         ASSERT_EQ(epoll2.wait(0,false),1);
         ASSERT_EQ(pipe.read(buf,4),4);
         ASSERT_EQ(epoll2.wait(10,false),0);

         epoll2.close();
         pipe.close();
      }
      catch (crunnable::CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         sleep(1);
         throw e;
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
