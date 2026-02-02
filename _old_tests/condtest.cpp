#include "gtest/gtest.h"
#include "clist.h"
#include "crunnable.h"
#include "crtimer.h"
#include "condition.h"
#include "socket.h"

using namespace std;
using namespace crunnable;

namespace badu {
   class Server : public CRunnable, public Condition {

   private:
      uint64_t m_delay;
      uint64_t m_start_delay;

   public:
      Server(int startdelay, bool bflag=false, const char* tag=NULL) 
         : Condition(tag,bflag,true), m_delay(0L), m_start_delay(startdelay) {
      }

      ~Server() {
         waitStop(NS_IN_ONE_MSEC);
      }

      virtual int run() {
         try {
            CRTime now((uint64_t) 0L);

            // Indicate we are ready to service requests
            if (m_start_delay != 0L) {
               now.nsleep(m_start_delay);
            }

            raise();
            now.msleep(10);

            while (isTerminated() == false) {
               if (m_delay != 0) {
                  now.nsleep(m_delay);
               }
               waitFor();
            }
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }

      void delay(uint64_t delay) {
         m_delay = delay;
      }
   };

   // The fixture for testing class Foo.
   class ConditionTest : public ::testing::Test {
   protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     ConditionTest() {
     }

     virtual ~ConditionTest() {
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

   TEST_F(ConditionTest, ConditionDelete) {
      try {
         // just declare it, make sure it gets created, and destroyed.
         Condition foo();
      }
      catch (CRException& report) {
         CRX_REPORT_CATCH(stderr, report);
      }
   }

   TEST_F(ConditionTest, ConditionNamedDelete) {
      try {
         // just declare it, make sure it gets created, and destroyed.
         Condition foo("NAMED",false);
      }
      catch (CRException& report) {
         CRX_REPORT_CATCH(stderr, report);
      }
   }

   // Tests that the ConditionRemoval/cleanup when a thread is blocked on the conditional
   TEST_F(ConditionTest, ConditionServerNormal1) {
      Server *srvr;

      try {
         // 1st test is we let the server hit notify before we wait
         // Test1 (n=0):
         // S: -- ws -- rc   -- d(10)  -- (d(0)--w(0))^2 -- E
         // M: --- s -- d(1) -- wc(1)> -- rc--rc -- d(1) -- X
         //
         srvr = new Server(0L,false,"CN1");

         srvr->start(true);
         usleep(1000);
         fprintf(stderr, "\n\n----------------------------\n");
         int result = srvr->waitFor(1000000);
         ASSERT_EQ(result,0);
         fprintf(stderr, "----------------------------\n\n\n");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->raise();
         srvr->raise();
         usleep(20000);
         fprintf(stderr, "----------------------------\n\n\n");

         delete srvr;
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
         // wait so that the output is dumped.
         sleep(1);
      }
   }

   TEST_F(ConditionTest, ConditionServerNormal2) {
      Server *srvr;

      try {
         // 1st test is we let the server hit notify before we wait
         // Test2 (n=0):
         // S: -- ws -- rc     -- d(10)  -- (d(0)--w(0))^2  -- E
         // M: --- s -- wc(1)> ------------ rc--rc -- d(20) -- X
         //
         srvr = new Server(0L,false,"CN2");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->start(true);
         int result = srvr->waitFor(1000000);
         ASSERT_EQ(result,0);
         fprintf(stderr, "----------------------------\n\n\n");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->raise();
         srvr->raise();
         usleep(20000);
         fprintf(stderr, "----------------------------\n\n\n");

         delete srvr;
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
         // wait so that the output is dumped.
         sleep(1);
      }
   }

   TEST_F(ConditionTest, ConditionServerNormal3) {
      Server *srvr;

      try {
         // 1st test is we let the server hit notify before we wait
         srvr = new Server(1000000L,false,"CN3");
         srvr->start(true);

         fprintf(stderr, "\n\n----------------------------\n");
         int result = srvr->waitFor();
         ASSERT_EQ(result,0);
         fprintf(stderr, "----------------------------\n\n\n");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->raise();
         srvr->raise();
         usleep(20000);
         fprintf(stderr, "----------------------------\n\n\n");

         delete srvr;
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
         // wait so that the output is dumped.
         sleep(1);
      }
   }

   TEST_F(ConditionTest, ConditionServerNormal4) {
      Server *srvr;

      try {
         // 1st test is we let the server hit notify before we wait
         srvr = new Server(1000000L,false,"CN4");
         srvr->start(true);

         fprintf(stderr, "\n\n----------------------------\n");
         int result = srvr->waitFor(100000L);
         ASSERT_EQ(result,110);
         fprintf(stderr, "----------------------------\n\n\n");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->raise();
         srvr->raise();
         usleep(20000);
         fprintf(stderr, "----------------------------\n\n\n");

         delete srvr;
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
         // wait so that the output is dumped.
         sleep(1);
      }
   }

   TEST_F(ConditionTest, ConditionBroadcast1) {
      Server *srvr;

      try {
         srvr = new Server(0L,true,"CB1");
         srvr->start(true);

         fprintf(stderr, "\n\n----------------------------\n");
         int result = srvr->waitFor();
         ASSERT_EQ(result,0);
         fprintf(stderr, "----------------------------\n\n\n");

         fprintf(stderr, "\n\n----------------------------\n");
         srvr->raise();
         srvr->raise();
         usleep(20000);
         fprintf(stderr, "----------------------------\n\n\n");
         delete srvr;
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
         // wait so that the output is dumped.
         sleep(1);
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
