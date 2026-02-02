#include "gtest/gtest.h"
#include "crexception.h"
#include "args.h"

using namespace std;
using namespace crunnable;

namespace badu {
   class MyArgs : public Args {
      public:
      MyArgs(int size, char** flags, char* name) : Args(size, flags, name) {}
      virtual ~MyArgs() = default;

      void put(const char* value) {
         Args::put(value);
      }
   };

   // The fixture for testing class Foo.
   class ArgsTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     ArgsTest() {
       // You can do set-up work for each test here.
     }

     virtual ~ArgsTest() {
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
   TEST_F(ArgsTest,WriteOperations) {
      const char* flags[] = {"--help", "-f", "one"};

      try {
         MyArgs args(3,(char**) flags, (char*) "TestArgs");;
         args.put("single");
      }
      catch (CRException& ignore) {
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
