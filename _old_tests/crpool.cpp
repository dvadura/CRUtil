#include "gtest/gtest.h"
#include "crpool.h"
#include "epoll.h"
#include "log.h"
#include "fd.h"

using namespace std;
using namespace crunnable;

namespace badu {
   //class Pooled : public IPoolItem<CRPool, Pooled> {
   class TPooled : public Pooled {
      public:
      TPooled() = default;
      virtual ~TPooled() = default;

      void reset() {
      }

      void stop() {
      }

      void reuse() {
      }

      void destroy() {
      }
   };

   class CRPooled : public Pooled, CRunnable {
      public:
      CRPooled() {
         start(true);
      }

      virtual ~CRPooled() {
         waitStop();
      }

      virtual void reset() {
      }

      virtual void reuse() {
      }

      virtual void stop() {
         CRunnable::stop();
      }

      virtual void destroy() {
         waitStop(NS_IN_TWO_SEC);
      }

      virtual int run() {
         FD in(fileno(stdin));
         EPoll epoll;

         epoll.open();
         epoll.add(EPOLLIN, in);

         do {
            int evcnt = epoll.wait();
            if (evcnt == 0) {
               break;
            }
         }
         while(isTerminated() == false);

         return 0;
      }
   };

   // The fixture for testing class Foo.
   class CRPoolTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      CRPoolTest() {
         // You can do set-up work for each test here.
      }

      virtual ~CRPoolTest() {
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

   // Tests that the CRPool::* method throws if not re-entrant
   TEST_F(CRPoolTest, SimplePool) {
      try {
         string tag="SimplePool";
         CRPool pool(tag, NULL, newitem<TPooled>);
         pool.setReapDelayMS(30);
         Pooled* item = pool.reserve();
         pool.release(item);
         ASSERT_THROW(pool.release(item),CRException);
         usleep(50000);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         throw e;
      }
   }

   TEST_F(CRPoolTest, BiggerPool) {
      string tag="BiggerPool";
      CRPool pool(tag, NULL, newitem<TPooled>);
      pool.setReapDelayMS(30);

      CList<Pooled*> items;

      for (int i=0; i<1024; ++i) {
         items.push_back(pool.reserve());
      }

      for (int i=0; i<1024; ++i) {
         pool.release(items.remove_front());
      }

      usleep(200000);

      ASSERT_EQ(pool.pendingfree(), 0);
      ASSERT_EQ(pool.max(), 4096);
      ASSERT_EQ(pool.sizemax(), 1024);
      ASSERT_EQ(pool.target(), 50);
      ASSERT_EQ(pool.reclaim(), 0);

      pool.trim(pool.target());
      ASSERT_EQ(pool.target(), pool.size());
      ASSERT_EQ(pool.max()-pool.target(), pool.unallocated());
   }

   TEST_F(CRPoolTest, CRPool) {
      string tag="CRPool";
      CRPool pool(tag,NULL,newitem<CRPooled>);
      pool.setReapDelayMS(30);

      CRPooled* item = crp_tcast<CRPooled>(pool.reserve());
      ASSERT_NE(item,nullptr);

      pool.release(item);
      usleep(10000);

      pool.trim(0,true);
      usleep(50000);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
