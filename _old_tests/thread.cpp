#include "gtest/gtest.h"
#include "condition.h"
#include "crunnable.h"

using namespace std;
using namespace crunnable;

namespace badu {
   class Thread : public CRunnable {
      private:
      Condition m_trigger;
      bool      m_pause;

      public:
      Thread() : m_trigger(true) {
         m_pause = false;
         start(true);
      }

      virtual ~Thread() {
         waitStop();
      }

      virtual int run() {
         service();

         try {
            m_trigger.waitFor(0L);

            if (m_pause == true) {
               pause();
            }
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }

      void notify(const bool pause=false) {m_pause=pause; m_trigger.notify();};
   };

   class CancelThread : public CRunnable {
      private:

      public:
      CancelThread() = default;
      virtual ~CancelThread() = default;

      virtual int run() {
         try {
            pause();
            while(true) sleep(1);
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }

      // nullify stop so that we force cancellation
      virtual void stop() {}
   };

   class StopThread : public CRunnable {
      private:

      public:
      StopThread() = default;
      virtual ~StopThread() = default;

      virtual int run() {
         service();
         try {
            pause();
            while(true) sleep(1);
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }
   };

   // The fixture for testing class Foo.
   class ThreadTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.
      ThreadTest() { }

      virtual ~ThreadTest() {
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
   TEST_F(ThreadTest, ThreadStartStop) {
      try {
         StopThread* t = new StopThread();
         usleep(500);
         t->stop();
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      try {
         StopThread* t = new StopThread();
         t->stop();
         usleep(500);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      int count=100;
      while (count-- > 0) {
         try {
            StopThread* t = new StopThread();
            t->stop();
            delete t;
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDelete) {
      try {
         Thread* t = new Thread();
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteWithSleep) {
      try {
         Thread* t = new Thread();
         usleep(50000);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteWithNotify) {
      try {
         Thread* t = new Thread();
         t->notify();
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteWithNotifyAndSleep) {
      try {
         Thread* t = new Thread();
         t->notify();
         usleep(50000);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteWithPause) {
      try {
         Thread* t = new Thread();
         t->notify(true);
         usleep(50000);
         t->resume(false);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteWithPauseAndStop) {
      try {
         Thread* t = new Thread();
         t->notify(true);
         t->stop();
         EXPECT_THROW(t->resume(), CRException);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadDeleteUsingCancel) {
      try {
         CancelThread* t = new CancelThread();
         t->start(true);
         while(t->isPaused() == false) usleep(50000);
         t->setCancelTimeout(NS_IN_ONE_MSEC);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

#if 0
   // FIXME: this test needs to run as root
   // Tests that the Socket works
   TEST_F(ThreadTest, ThreadPriority) {
      try {
         StopThread* t = new StopThread();
         t->start(true);
         usleep(1000);

         string str;
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "RR:90,s");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "RR:90,s");
         t->nice(-1);
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "RR:89,s");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "RR:89,s");
         t->realtime();
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "RR:70,s");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "RR:70,s");
         t->user();
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "OTHER:120,d");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "OTHER:120,d");
         t->nice(-10);
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "OTHER:110,d");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "OTHER:110,d");
         t->nice(0);
         ASSERT_STREQ(t->getPriorityString(str).c_str(), "OTHER:120,d");
         ASSERT_STREQ(t->getPriorityString(str,true).c_str(), "OTHER:120,d");
         t->setCancelTimeout(NS_IN_ONE_MSEC);
         t->stop();
         usleep(500);
         delete t;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }
#endif
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
