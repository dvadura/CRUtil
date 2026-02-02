#include "gtest/gtest.h"
#include "sharedptr.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class ASharedPtrTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     ASharedPtrTest() {
       // You can do set-up work for each test here.
     }

     virtual ~ASharedPtrTest() {
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

   struct Foo {
      char *p;
      int  ix;
      Foo(char *buf) { p=buf; ix=1; }
      ~Foo() { strcpy(p,"delete foo"); }
      int x() {return ix;}
   };

   // Tests that the Semaphore::P() method throws if not re-entrant
   TEST_F(ASharedPtrTest,BaseTest) {
      char buf[100];

      SharedPtr<Foo> *sh1 = new SharedPtr<Foo>(std::shared_ptr<Foo>(new Foo(buf)));
      SharedPtr<Foo> *sh2 = new SharedPtr<Foo>(*sh1);

      ASSERT_EQ(sh2->use_count(), 2);
      ASSERT_EQ(sh1->use_count(), 2);

      EXPECT_EQ((*sh1)->x(), 1);
      EXPECT_EQ((*sh2)->x(), 1);

      buf[0] = '\0';
      delete sh2;

      EXPECT_EQ(sh1->use_count(), 1);
      EXPECT_NE(strcmp(buf,"delete foo"),0);

      delete sh1;
      EXPECT_EQ(strcmp(buf,"delete foo"),0);

      SharedPtr<Foo> sh3(std::shared_ptr<Foo>(NULL));
      ASSERT_THROW(sh3->x(), CRException);

      try {
         sh3->x();
      }
      catch (CRException& e) {
         EXPECT_EQ(strcmp(e.what(), "CRX: NULL pointer dereference"), 0);
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
