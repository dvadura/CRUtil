#include "gtest/gtest.h"
#include "config0.h"

using namespace std;
using namespace crunnable;
using namespace json;

namespace badu {
   // The fixture for testing class Foo.
   class ConfigTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     ConfigTest() {
       // You can do set-up work for each test here.
     }

     virtual ~ConfigTest() {
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

   TEST_F(ConfigTest, DocumentTest) {
      Config conf;

      const char *res=
"{\n\
  \"pfd\" : {\n\
    \"bar\" : [1, 1],\n\
    \"foo\" : [1, 2, 3, 4],\n\
    \"one\" : \"this is a string, conf3\",\n\
    \"three\" : 3,\n\
    \"two\" : 2,\n\
    \"xxx\" : 1.1\n\
  }\n\
}";

      try {
         string s, i;

         conf.initialize("data/conf1.json");
         conf.initialize("data/conf2.json");
         conf.initialize("data/conf3.json");

         Document& dom = conf.getDom();
         ASSERT_STREQ(dom.toString(s,i,true,true).c_str(),res);

         conf.initialize("data/conf4.json");
         res =
"{\n\
  \"pfd\" : {\n\
    \"bar\" : [1, 1, 1, 1],\n\
    \"foo\" : [1, 2, 3, 4],\n\
    \"one\" : \"this is a string, conf3\",\n\
    \"three\" : 3,\n\
    \"two\" : 2,\n\
    \"xxx\" : 1.1\n\
  }\n\
}";
         ASSERT_STREQ(dom.toString(s,i,true,true).c_str(),res);

         conf.initialize("data/conf5.json");
         res =
"{\n\
  \"pfd\" : {\n\
    \"bar\" : [1, 1, 1, 1],\n\
    \"foo\" : [1, 2, 3, 4],\n\
    \"one\" : \"this is a string, conf3\"\n\
  }\n\
}";
         ASSERT_STREQ(dom.toString(s,i,true,true).c_str(),res);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(ConfigTest, ObjectTest) {
      Config conf;
      const char *res=
"{\n\
  \"lb_1_0\" : {\n\
    \"four\" : false,\n\
    \"one\" : false,\n\
    \"three\" : true,\n\
    \"two\" : false\n\
  }\n\
}";

      try {
         string s, i;
         conf.initialize("data/conf6.json");
         Document& dom = conf.getDom();
         dom.toString(s,i,true,true);
         ASSERT_STREQ(s.c_str(),res);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(ConfigTest, BenObjectTest) {
      Config conf;
      const char *res=
"{\n\
  \"lb_1_0\" : {\n\
    \"CLNT\" : {\n\
      \"client\" : true,\n\
      \"plumb\" : \"1_0C\",\n\
      \"proxy\" : [{\"dip\":\"*\",\"dport\":0,\"sip\":\"*\",\"sport\":0}],\n\
      \"pxorder\" : [\"bypass\", \"local\", \"allow\", \"drop\", \"proxy\"]\n\
    },\n\
    \"INET\" : {\n\
      \"plumb\" : \"1_0I\",\n\
      \"proxy\" : [{\"dip\":\"*\",\"dport\":0,\"sip\":\"*\",\"sport\":0}],\n\
      \"pxorder\" : [\"bypass\", \"local\", \"allow\", \"drop\", \"proxy\"]\n\
    },\n\
    \"bidirection_parent\" : \"lp_1_0\",\n\
    \"id\" : 0,\n\
    \"mpls\" : false,\n\
    \"notes\" : \"dummy\",\n\
    \"proxy\" : true,\n\
    \"pxarp\" : true,\n\
    \"splice\" : [\"INET\", \"CLNT\"],\n\
    \"swBypass\" : false,\n\
    \"validations\" : [\"1_0V\"]\n\
  }\n\
}";
      try {
         string s, i;
         conf.initialize("data/conf7.json");
         Document& dom = conf.getDom();
         dom.toString(s,i,true,true);
         ASSERT_STREQ(s.c_str(),res);
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
