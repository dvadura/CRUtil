#include "gtest/gtest.h"
#include "json.h"

using namespace std;
using namespace crunnable;
using namespace crunnable::json;

namespace badu {
   // The fixture for testing class Foo.
   class JSONTest : public ::testing::Test {
    protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     JSONTest() {
       // You can do set-up work for each test here.
     }

     virtual ~JSONTest() {
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
   TEST_F(JSONTest,NullTest) {
      const char *json = (const char*) "null";

      try {
         valueptr_t np1 = Value::parse(json);
         valueptr_t np2 = Value::parse(json);

         ASSERT_EQ(np1.use_count(), 3);
         ASSERT_EQ(np2.use_count(), 3);

         ASSERT_STREQ(np1->c_str(), "null");
         ASSERT_TRUE(np1 == JSON::VALUE_NULL);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      ASSERT_EQ(JSON::VALUE_NULL.use_count(), 1);
   }

   TEST_F(JSONTest,BooleanTrueTest) {
      const char *json = (const char*) "true";

      try {
         valueptr_t np1 = Value::parse(json);
         valueptr_t np2 = Value::parse(json);

         ASSERT_EQ(np1.use_count(), 3);
         ASSERT_EQ(np2.use_count(), 3);

         ASSERT_STREQ(np1->c_str(), "true");
         ASSERT_TRUE(np1 == JSON::VALUE_TRUE);
         ASSERT_EQ(true,np1->toBoolean());
         ASSERT_EQ(true,JSON::VALUE_TRUE->toBoolean());
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      ASSERT_EQ(JSON::VALUE_TRUE.use_count(), 1);
   }

   TEST_F(JSONTest,BooleanFalseTest) {
      const char *json = (const char*) "false";

      try {
         valueptr_t np1 = Value::parse(json);
         valueptr_t np2 = Value::parse(json);

         ASSERT_EQ(np1.use_count(), 3);
         ASSERT_EQ(np2.use_count(), 3);

         ASSERT_STREQ(np1->c_str(), "false");
         ASSERT_TRUE(np1 == JSON::VALUE_FALSE);
         ASSERT_EQ(false,np1->toBoolean());
         ASSERT_EQ(false,JSON::VALUE_FALSE->toBoolean());
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      ASSERT_EQ(JSON::VALUE_FALSE.use_count(), 1);
   }

   TEST_F(JSONTest,StringTest) {
      const char *json1 = (const char*) "\"false\\u0061\\\\\\/\n\"";
      const char *json2 = (const char*) "\"bare";
      const char *json3 = (const char*) "bare\"";
      const char *json4 = (const char*) "\"\"";

      try {
         valueptr_t np1 = Value::parse(json1);
         valueptr_t np4 = Value::parse(json4);

         ASSERT_EQ(np1.use_count(), 1);
         ASSERT_STREQ(np1->c_str(), "falsea\\/\n");
         ASSERT_STREQ(np4->c_str(), "");

         EXPECT_THROW(Value::parse(json2), CRException);
         EXPECT_THROW(Value::parse(json3), CRException);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }


   TEST_F(JSONTest,NumberTest) {
      const char *json1 = (const char*) "10.1";
      const char *json2 = (const char*) "-10";
      const char *json3 = (const char*) "10";
      const char *json4 = (const char*) "10.0";

      try {
         valueptr_t np = Value::parse(json1);
         ASSERT_EQ(np.use_count(), 1);
         ASSERT_TRUE(np->isDouble());
         ASSERT_STREQ(np->c_str(), "10.1");
         ASSERT_EQ(np->size(), 8);

         np = Value::parse(json2);
         ASSERT_EQ(np.use_count(), 1);
         ASSERT_TRUE(np->isInteger());
         ASSERT_STREQ(np->c_str(), "-10");
         ASSERT_EQ(np->size(), 8);

         np = Value::parse(json3);
         ASSERT_EQ(np.use_count(), 1);
         ASSERT_TRUE(np->isUnsigned());
         ASSERT_STREQ(np->c_str(), "10");
         ASSERT_EQ(np->size(), 16);

         np = Value::parse(json4);
         ASSERT_EQ(np.use_count(), 1);
         ASSERT_TRUE(np->isDouble());
         ASSERT_STREQ(np->c_str(), "10.0");
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(JSONTest,ArrayTest) {
      const char *json1 = (const char*) "[1,2, 3,4  ]";
      const char *json2 = (const char*) "[1,2, 3,4, ]";
      const char *json3 = (const char*) "[1,2, 3,\"data\", ]";

      try {
         valueptr_t np1 = Value::parse(json1);
         ASSERT_EQ(np1.use_count(), 1);
         ASSERT_TRUE(np1->isArray());
         ASSERT_EQ(np1->size(),4);
         ASSERT_STREQ(np1->c_str(), "[1,2,3,4]");

         valueptr_t np2 = Value::parse(json2);
         ASSERT_EQ(np2.use_count(), 1);
         ASSERT_TRUE(np2->isArray());
         ASSERT_EQ(np2->size(),4);
         ASSERT_STREQ(np2->c_str(), "[1,2,3,4]");

         valueptr_t np3 = Value::parse(json3);
         ASSERT_EQ(np3.use_count(), 1);
         ASSERT_TRUE(np3->isArray());
         ASSERT_EQ(np3->size(),4);
         ASSERT_STREQ(np3->c_str(), "[1,2,3,data]");

         Array& arr = *np3;
         ASSERT_STREQ(arr[3]->c_str(),"data");
         ASSERT_THROW(Object& obj=*np3, CRException);

         int list[10];
         for(size_t i=0; i<sizeof(list); ++i) {
            list[i] = -1;
         }

         np1->toArray().toIntList(list);
         ASSERT_EQ(list[0],1);
         ASSERT_EQ(list[1],2);
         ASSERT_EQ(list[2],3);
         ASSERT_EQ(list[3],4);
         ASSERT_EQ(list[4],-1);
         ASSERT_EQ(list[9],-1);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(JSONTest,ObjectTest) {
      const char *json1 = (const char*) "{\"one\" : 1, \"two\" : 2,}";
      const char *json2 = (const char*) "{\"one\" : 1, \"two\" : [2,3,4], \"three\" : \"three\", \"a\":{\"b\":1}}";
      const char *json3 = (const char*) "{\"one\" : 1, \"two\" : [2,[3,3,3],{\"a\":1}], \"three\" : \"three\", \"a\":{\"b\":1}}";
      const char *res=
"{\n\
  \"a\" : {\n\
    \"b\" : 1\n\
  },\n\
  \"one\" : 1,\n\
  \"three\" : \"three\",\n\
  \"two\" : [2,\n\
 [3,3,3],\n\
 {\"a\":1}]\n\
}";

      try {
         string s;
         string i="";

         valueptr_t np1 = Value::parse(json1);
         ASSERT_EQ(np1.use_count(), 1);
         ASSERT_TRUE(np1->isObject());
         ASSERT_EQ(np1->size(),2);
         ASSERT_STREQ(np1->c_str(), "{one:1,two:2}");

         valueptr_t np2 = Value::parse(json2);
         ASSERT_EQ(np2.use_count(), 1);
         ASSERT_TRUE(np2->isObject());
         ASSERT_EQ(np2->size(),4);
         ASSERT_STREQ(np2->c_str(), "{a:{b:1},one:1,three:three,two:[2,3,4]}");

         valueptr_t np3 = Value::parse(json3);
//fprintf(stderr, "-------\n%s\n--------\n", np3->toString(s,i,true,true).c_str());
         ASSERT_STREQ(np3->toString(s,i,true,true).c_str(),res);

         int result = np1->object("nothere")["foobar"]->toInt();
         ASSERT_EQ(result,0);
         ASSERT_STREQ(np1->c_str(), "{nothere:{},one:1,two:2}");
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(JSONTest, DocumentTest) {
      const char *json1 = (const char*) "{\"one\" : 1, \"two\" : 2,\"!a\":{\"b\":1}}";
      const char *json2 = (const char*) "{\"one\" : 1, \"two\" : [2,3,4], \"three\" : \"three\", \"a\":{\"b\":2,\"c\":3}}";
      const char *json3 = (const char*) "{\"one\" : 1, \"two\" : [2,[3,3,3],{\"a\":1}], \"three\" : \"three\", \"a\":{\"b\":1,\"d\":4}}";
      const char *res=
"{\n\
  \"a\" : {\n\
    \"b\" : 1,\n\
    \"d\" : 4\n\
  },\n\
  \"one\" : 1,\n\
  \"three\" : \"three\",\n\
  \"two\" : [2,\n\
 [3,3,3],\n\
 {\"a\":1}]\n\
}";
      try {
         string s;
         string i="";

         Document doc((const char*) json3);
//fprintf(stderr, "-------\n%s\n--------\n", doc.toString(s,i,true,true).c_str());
         ASSERT_STREQ(doc.toString(s,i,true,true).c_str(),res);

         doc |= json2;
         ASSERT_STREQ(doc.c_str(), "{a:{b:2,c:3,d:4},one:1,three:three,two:[2,3,4]}");

         doc |= json1;
         ASSERT_STREQ(doc.c_str(), "{a:{b:1},one:1,three:three,two:2}");

         doc -= json1;
         ASSERT_STREQ(doc.c_str(), "{three:three}");

         doc.clear();
         ASSERT_STREQ(doc.c_str(), "{}");

         doc.reset();
         EXPECT_THROW(doc.c_str(), CRException);
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(JSONTest, ObjectArrayTest) {
      const char* json1 = (const char*) "{\"aa\" : [\"x\"]}";
      const char* json2 = (const char*) "{\"aa\" : [\"x\", \"y\"]}";

      try {
         valueptr_t v1 = Value::parse(json1);
         valueptr_t v2 = Value::parse(json1);
         valueptr_t v3 = Value::parse(json2);

         ASSERT_STREQ(v1->c_str(), "{aa:[x]}");
         *v1 |= v2;
         ASSERT_STREQ(v1->c_str(), "{aa:[x]}");
         *v1 |= v3;
         ASSERT_STREQ(v1->c_str(), "{aa:[x,y]}");
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   TEST_F(JSONTest, ObjectArray2Test) {
      const char* json1 = (const char*) "{\"S500\":[\"DV.RT1\",\"DV.RT2\"]}";

      try {
         valueptr_t v1 = Value::parse(json1);

         ASSERT_STREQ(v1->c_str(), "{S500:[DV.RT1,DV.RT2]}");
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
