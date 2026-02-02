#include "gtest/gtest.h"
#include "crpool.h"
#include "log.h"
#include "fd.h"
#include "crtoq.h"
#include "vector"

using namespace std;
using namespace crunnable;

namespace badu {
   //class Pooled : public IPoolItem<CRPool, Pooled> {
   class Pooled : public IPoolItem<Pooled> {
      public:
      Pooled(IPoolItem::pool_t* pool) : IPoolItem(pool) {}
      virtual ~Pooled() = default;

      void reset() {
      }

      void stop() {
      }

      void reuse() {
      }

      void destroy() {
      }
   };

   class CRPooled : public IPoolItem<CRPooled> {
      public:
      CRPooled(IPoolItem::pool_t* pool) : IPoolItem(pool) {
      }

      virtual ~CRPooled() {
      }

      virtual void reset() {
      }

      virtual void reuse() {
      }

      virtual void stop() {
      }

      virtual void destroy() {
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
   TEST_F(CRPoolTest, CRPool) {
      string tag="CRPool";
      Log* gl = Log::global(fileno(stderr));

      try {
         CRPool<CRPooled> pool(tag,gl,2000,50,100);
         CRToq<CRPooled*> crtoq(5000);

         pool.setReapDelayMS(2000);
         pool.ttlreport(true,2000);

         do {
            for (int i=0; i<1000; ++i) {
               CRPooled* item = pool.reserve();
               crtoq.add(item, (random()%7000)+(random()%5000));
            }

            string s;
            fprintf(stderr, "CRTQ: %s\n", crtoq.toString(s,10).c_str());

            while (crtoq.getSize() > 0) {
               CRPooled* item = pool.reserve();
               ASSERT_NE(item,nullptr);

               if (crtoq.hasExpired() == true) {
                  vector<CRPooled*> exp(2048);
                  size_t count = crtoq.getExpired(exp, 2048);
                  LG_WARNING(gl,"---> expire %lu items\n", count);

                  for (size_t j=0; j<count; ++j) {
                     pool.release(exp[j]);
                  }
                  usleep(4000);
               }
               else {
                  usleep(5000);
               }
               
               pool.release(item);
               usleep(16000);
            }
         }
         while(true);
      }
      catch (CRException& ex) {
         CRX_REPORT_CATCH(stderr, ex);
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
