#include "gtest/gtest.h"
#include "condition.h"
#include "crunnable.h"

using namespace std;
using namespace crunnable;

namespace badu {
   class Worker : public CRunnable {
      private:

      public:
      Worker() {
         start();
      }

      virtual ~Worker() {
         waitStop();
      }

      virtual int run() {
         pushNameTID("worker");

         resetCPUAffinityMask();
         addCPU(1);
         setCPUAffinity();

         service();
         //daemon(SCHED_FIFO,CRDAEMON_DEFAULT_PRIORITY);

         try {
            while(isTerminated() == false) {
               fprintf(stderr, "worker is working\n");
               sleep(1);
            }
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }
   };

   class Loop : public CRunnable {
      private:
      Condition m_trigger;

      public:
      Loop() : m_trigger(true) {
         start(true);
      }

      virtual ~Loop() {
         waitStop();
      }

      virtual int run() {
         pushNameTID("looper");

         resetCPUAffinityMask();
         addCPU(1);
         setCPUAffinity();

         daemon(SCHED_FIFO,CRDAEMON_DEFAULT_PRIORITY);

         try {
            m_trigger.waitFor(0L);

            while (isTerminated() == false) {
               for (size_t i=0; i<500000000; ++i);
               fprintf(stderr, "looper: one loop\n");
               sched_yield();
            }
         }
         catch (CRException& e) {
            CRX_REPORT_CATCH(stderr, e);
         }

         return 0;
      }

      void notify() {m_trigger.notify();};
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
   TEST_F(ThreadTest, PriorityTest) {
      cpu_set_t cset;

      CPU_ZERO(&cset);
      CPU_SET(1, &cset);
      sched_setaffinity(getpid(), sizeof(cpu_set_t), &cset);

      Worker work;
      sleep(0);
      Loop   looper;
      looper.notify();

      sleep(10);
      fprintf(stderr, "main thread stopping\n");
      looper.stop();
      work.stop();
   };
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
