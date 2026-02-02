#include "gtest/gtest.h"
#include "crunnable.h"
#include "log.h"
#include "pipe.h"

#define LOGTHIS 1
#define DONTLOG 0

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class LogTest : public ::testing::Test {
      protected:
      Pipe pipe;

      // You can remove any or all of the following functions if its body
      // is empty.

      LogTest() {
      }

      virtual ~LogTest() {
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

      virtual const char *
      read(string& line) {
         usleep(5000);
         pipe.read(line);
         CRS::trim(line);

         return line.c_str();
      }

      // Objects declared here can be used by all tests in the test case for Foo.
   };

   // Tests that the Log works
   TEST_F(LogTest, Logger) {
      string line;

      try {
         // open the pipe for testing log output
         pipe.open();
         pipe.fdout().block(false);

         // Get us a log object
         Log *log = Log::global(pipe.in());

         // perform the tests.
         ASSERT_STREQ(read(line), "GLOG: ALT: Initialize logging service, name GLOG-lg{stderr,5}, fd=2, hidedbg=0.");

         LG_WARN(log, "First warning %s", "foo");
         ASSERT_STREQ(read(line), "GLOG: WRN: First warning foo.");

         LG_ERROR(log, "First error %d", 1);
         ASSERT_STREQ(read(line), "GLOG: ERR: First error 1.");

         LG_NOTICE(log, "First notice %d", 2);
         ASSERT_STREQ(read(line), "GLOG: NOT: First notice 2.");

         LG_INFO(log, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "");

         log->level(L_INFO);
         LG_INFO(log, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: INF: First info 3.");

         log->level(L_VOMIT);
         LG_DEBUG(log, "First debug %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: DBG: First debug 3.");

         LG_VOMIT(log, "First vomit %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: VOM: First vomit 3.");

         log->hidedebug(true);

         LG_VOMIT(log, "First vomit %d", 3, 4);
         ASSERT_STREQ(read(line), "");

         LG_DEBUG(log, "Next we sing");
         ASSERT_STREQ(read(line), "");

         LG_INFO(log, "Last info %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: INF: Last info 3.");

         log->level(L_INFO);
         LG_INFO(log, "Last info %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: INF: Last info 3.");

         ASSERT_TRUE(log->hidedebug());

         log->hidedebug(false);
         ASSERT_FALSE(log->hidedebug());

         LG_INFO(log, "Last info %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: INF: Last info 3.");

         // stop logging and see what we get.
         delete log;
         ASSERT_STREQ(read(line), "GLOG: ALT: Shutdown logging service.");

         // close the pipe
         pipe.close();
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(LogTest, BigLogmsg) {
      string line = "This is a long line that is at least 80 characters and we will make it even bigger.";

      try {
         string rline;
         string lline;

         // open the pipe
         pipe.open();
         pipe.fdout().block(false);

         // Get a logger
         Log *log = Log::global(pipe.in());
         ASSERT_FALSE(log==NULL);

         // perform the tests.
         ASSERT_STREQ(read(rline), "GLOG: ALT: Initialize logging service, name GLOG-lg{stderr,5}, fd=2, hidedbg=0.");

         lline = line;
         lline += ' ';
         lline += line;
         lline += ' ';
         lline += line;

         LG_ERROR(log, lline.c_str());
         rline = "GLOG: ERR: ";
         rline += lline;
         lline = rline;
         rline.clear();
         ASSERT_STREQ(read(rline), lline.c_str());

         lline = line;
         while (lline.size() < 1024) {
            lline += ' ';
            lline += line;
         }

         LG_ERROR(log, lline.c_str());
         while (lline.size() > 1024) lline.pop_back();
         lline += "|>";

         rline  = "GLOG: ERR: ";
         rline += lline;
         lline  = rline;
         rline.clear();
         ASSERT_STREQ(read(rline), lline.c_str());

         // stop logging and see what we get.
         delete log;
         ASSERT_STREQ(read(line), "GLOG: ALT: Shutdown logging service.");

         // close the pipe
         pipe.close();
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the Log works
   TEST_F(LogTest, LoggerClasses) {
      string line;

      try {
         // open the pipe for testing log output
         pipe.open();
         pipe.fdout().block(false);

         // Get us a log object
         Log *log = Log::global(pipe.in());
         ASSERT_FALSE(log==NULL);

         // perform the tests.
         ASSERT_STREQ(read(line), "GLOG: ALT: Initialize logging service, name GLOG-lg{stderr,5}, fd=2, hidedbg=0.");

         LC_WARN(log, 2, "First warning %s", "foo");
         ASSERT_STREQ(read(line), "GLOG: WRN: First warning foo.");

         LC_ERROR(log, 3, "First error %d", 1);
         ASSERT_STREQ(read(line), "GLOG: ERR: First error 1.");

         LC_NOTICE(log, 4, "First notice %d", 2);
         ASSERT_STREQ(read(line), "GLOG: NOT: First notice 2.");

         // blocked by log level
         LC_INFO(log, 5, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "");


         log->hide(2).hide(4);
         LC_WARN(log, 2, "First warning %s", "foo");
         ASSERT_STREQ(read(line), "");

         LC_ERROR(log, 3, "First error %d", 1);
         ASSERT_STREQ(read(line), "GLOG: ERR: First error 1.");

         LC_NOTICE(log, 4, "First notice %d", 2);
         ASSERT_STREQ(read(line), "");

         // blocked by log level
         LC_INFO(log, 5, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "");


         log->hideall();
         LC_WARN(log, 2, "First warning %s", "foo");
         ASSERT_STREQ(read(line), "");

         LC_ERROR(log, 3, "First error %d", 1);
         ASSERT_STREQ(read(line), "");

         LC_NOTICE(log, 4, "First notice %d", 2);
         ASSERT_STREQ(read(line), "");

         // blocked by log level
         LC_INFO(log, 5, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "");

         // no class messages do not get hidden
         LG_NOTICE(log, "Last notice %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: NOT: Last notice 3.");


         // lets try again but this time show all
         log->showall();

         LC_WARN(log, 2, "First warning %s", "foo");
         ASSERT_STREQ(read(line), "GLOG: WRN: First warning foo.");

         LC_ERROR(log, 3, "First error %d", 1);
         ASSERT_STREQ(read(line), "GLOG: ERR: First error 1.");

         LC_NOTICE(log, 4, "First notice %d", 2);
         ASSERT_STREQ(read(line), "GLOG: NOT: First notice 2.");

         // blocked by log level
         LC_INFO(log, 5, "First info %d", 3, 4);
         ASSERT_STREQ(read(line), "");

         // no class messages do not get hidden
         LG_NOTICE(log, "Last notice %d", 3, 4);
         ASSERT_STREQ(read(line), "GLOG: NOT: Last notice 3.");

         // using conditional log
         LG_NOTICE_IF(LOGTHIS, log, "logged it");
         ASSERT_STREQ(read(line), "GLOG: NOT: logged it.");

         // using conditional log
         LG_NOTICE_IF(DONTLOG, log, "logged it");
         ASSERT_STREQ(read(line), "");


         // stop logging and see what we get.
         delete log;
         ASSERT_STREQ(read(line), "GLOG: ALT: Shutdown logging service.");

         // close the pipe
         pipe.close();
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
