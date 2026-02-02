#include "gtest/gtest.h"
#include "crtoq.h"
#include "crtimer.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class CRToqTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     CRToqTest() {
       // You can do set-up work for each test here.
     }

     virtual ~CRToqTest() {
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

   TEST_F(CRToqTest, TimeoutTestSingle) {
      CRTime stamp;
      CRToq<void*> queue(2048,"CRTESTQ",50);

      stamp.now();
      ASSERT_EQ(queue.getCapacity(), 2048);
      ASSERT_EQ(queue.getInterval(), 50);
      
      queue.add((void*)10L, 1*queue.getInterval());

      ASSERT_EQ(queue.getDelay(),1);
      ASSERT_EQ(queue.getSize(),1);
      ASSERT_EQ(queue.getAvailable(),2047);

      stamp.msleep(2*queue.getInterval());

      ASSERT_EQ(queue.getDelay(),0);
      ASSERT_EQ(queue.getSize(),1);
      ASSERT_EQ(queue.getAvailable(),2047);

      vector<void*> data;
      size_t size = queue.getExpired(data);

      ASSERT_EQ(queue.getDelay(),0);
      ASSERT_EQ(queue.getSize(),0);
      ASSERT_EQ(queue.getAvailable(),2048);
      ASSERT_EQ(size,1);

      ASSERT_EQ(data[0],(void*)10L);
   }

   TEST_F(CRToqTest, TimeoutTestDual) {
      CRTime stamp;
      CRToq<void*> queue(2048,"CRTESTQ",50);
      const uint64_t interval = queue.getInterval();

      stamp.now();
      queue.add((void*)10L, 10*interval);
      stamp.msleep(2*interval);

      queue.add((void*)11L, 10*interval);

      stamp.msleep(15*interval);

      vector<void*> data;
      size_t size = queue.getExpired(data);

      ASSERT_EQ(queue.getDelay(),0);
      ASSERT_EQ(queue.getSize(),0);
      ASSERT_EQ(queue.getAvailable(),2048);
      ASSERT_EQ(size,2);

      ASSERT_EQ(data[0],(void*)11L);
      ASSERT_EQ(data[1],(void*)10L);
   }

   TEST_F(CRToqTest, TimeoutTestInsertion) {
      string tmp;
      CRToq<void*> queue(10);
      const uint64_t interval = queue.getInterval();

      queue.add((void*)10L, 10*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "10:10");
      ASSERT_EQ(queue.getDelay(),10);

      queue.add((void*)11L, 11*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "10:10 11:1");
      ASSERT_EQ(queue.getDelay(),11);

      queue.add((void*)1L, 1*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 10:9 11:1");
      ASSERT_EQ(queue.getDelay(),11);

      queue.add((void*)2L, 1*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 10:9 11:1");
      ASSERT_EQ(queue.getDelay(),11);

      queue.add((void*)3L, 3*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 10:7 11:1");
      ASSERT_EQ(queue.getDelay(),11);

      queue.add((void*)12L, 11*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 10:7 11:1 12:0");
      ASSERT_EQ(queue.getDelay(),11);

      queue.add((void*)13L, 10*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 10:7 13:0 11:1 12:0");
      ASSERT_EQ(queue.getDelay(),11);

      queue.remove((void*)10L);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 13:7 11:1 12:0");
      ASSERT_EQ(queue.getDelay(),11);

      queue.remove((void*)12L);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 13:7 11:1");
      ASSERT_EQ(queue.getDelay(),11);

      queue.remove((void*)11L);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "1:1 2:0 3:2 13:7");
      ASSERT_EQ(queue.getDelay(),10);

      queue.remove((void*)1L);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "2:1 3:2 13:7");
      ASSERT_EQ(queue.getDelay(),10);
   }


   TEST_F(CRToqTest, TimeoutTestTripple) {
      CRTime stamp;
      string tmp;
      CRToq<void*> queue(10,"CRTESTQ",10);
      const uint64_t interval = queue.getInterval();

      stamp.now();
      // 12:1 10:7 11:2
      queue.add((void*)10L, 10*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "10:10");
      stamp.msleep(2*interval+interval/2);

      ASSERT_EQ(queue.getDelay(),8);

      queue.add((void*)11L, 10*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "10:8 11:2");
      ASSERT_EQ(queue.getDelay(),10);

      queue.add((void*)12L, 1*interval);
      ASSERT_STREQ(queue.toString(tmp).c_str(), "12:1 10:7 11:2");
      ASSERT_EQ(queue.getDelay(),10);

      stamp.msleep(13*interval);
      ASSERT_EQ(queue.getDelay(),0);
      ASSERT_EQ(queue.getSize(),3);

      vector<void*> data;
      size_t size = queue.getExpired(data);

      ASSERT_EQ(queue.getSize(),0);
      ASSERT_EQ(queue.getAvailable(),2048);
      ASSERT_EQ(size,3);

      ASSERT_EQ(data[0],(void*)11L);
      ASSERT_EQ(data[1],(void*)10L);
      ASSERT_EQ(data[2],(void*)12L);
   }

   TEST_F(CRToqTest, TimeoutTestBig) {
      CRTime stamp;
      string tmp;
      CRToq<void*> bigqueue(10000000,"CRTESTQ",10);
      const uint64_t interval = bigqueue.getInterval();

      stamp.now();
      /// make sure that the free list is initialized
      bigqueue.waitFor();
      bigqueue.add((void*)10L, 10*interval);
      ASSERT_STREQ(bigqueue.toString(tmp).c_str(), "10:10");
      stamp.msleep(2*interval+interval/2);

      ASSERT_EQ(bigqueue.getDelay(),8);

      bigqueue.add((void*)11L, 10*interval);
      ASSERT_STREQ(bigqueue.toString(tmp).c_str(), "10:8 11:2");
      ASSERT_EQ(bigqueue.getDelay(),10);

      bigqueue.add((void*)12L, 1*interval);
      ASSERT_STREQ(bigqueue.toString(tmp).c_str(), "12:1 10:7 11:2");
      ASSERT_EQ(bigqueue.getDelay(),10);

      stamp.msleep(20*interval);
      ASSERT_EQ(bigqueue.getDelay(),0);
      ASSERT_EQ(bigqueue.getSize(),3);

      vector<void*> data;
      size_t size = bigqueue.getExpired(data);

      ASSERT_EQ(bigqueue.getSize(),0);
      ASSERT_EQ(size,3);

      ASSERT_EQ(data[0],(void*)11L);
      ASSERT_EQ(data[1],(void*)10L);
      ASSERT_EQ(data[2],(void*)12L);

      for (int i=0; i<3000; ++i) {
         bigqueue.add((void*) (i+100L), 10*interval);
      }

      stamp.msleep(20*interval);
      bigqueue.waitFor();

      size = bigqueue.getExpired(data);
      ASSERT_EQ(bigqueue.getSize(),0);
      ASSERT_EQ(size,3000);
   }

   TEST_F(CRToqTest, TimeoutTestAccuracy) {
      CRTime stamp;
      string tmp;
      CRToq<void*> bigqueue(1000,"CRTESTQ",250);
      const uint64_t interval = bigqueue.getInterval();

      stamp.now();
      /// make sure that the free list is initialized
      bigqueue.waitFor();
      bigqueue.add((void*)10L, interval);
      bigqueue.add((void*)10L, 2*interval);

      ASSERT_EQ(bigqueue.getDelay(),2);
      ASSERT_STREQ(bigqueue.toString(tmp).c_str(), "10:1 10:1");
      stamp.msleep(2*interval+10);
      ASSERT_EQ(bigqueue.getDelay(),0);
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
