#include "gtest/gtest.h"
#include "bigint.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class ABigintTest : public ::testing::Test {
   protected:
     // You can remove any or all of the following functions if its body
     // is empty.

     ABigintTest() {
       // You can do set-up work for each test here.
     }

     virtual ~ABigintTest() {
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

   // Tests biginteger construction, using the pair based implementation
   TEST_F(ABigintTest, ConstructionOpsPair) {
      uint128p_t a;
      uint128p_t b("18446744073709551617");
      uint128p_t c(9223372036854775808UL);
      uint128p_t d(1,1);
      uint128p_t e(d);

      ASSERT_FALSE(a.isZero());
      ASSERT_EQ(b,d);
      ASSERT_EQ(d,e);

      c *= 2;
      ++c;
      ASSERT_EQ(c,b);

      string s;
      ASSERT_TRUE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);

      uint128p_t f(s);
      ASSERT_EQ(f,b);
   }

   TEST_F(ABigintTest, AssignmentOpsPair) {
      uint128p_t b = "18446744073709551617";
      uint128p_t c(9223372036854775808UL);

      uint128p_t d = b;
      uint128p_t e = 1;

      c <<= 1;
      c += e;

      ASSERT_EQ(b,d);
      ASSERT_EQ(b,c);

      string s = "18446744073709551617";
      c = s;
      ASSERT_TRUE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);
   }

   TEST_F(ABigintTest, ConversionOpsPair) {
      uint128p_t a("18446744073709551617");

      bool v1 = a;
      uint8_t v2 = a;
      uint16_t v3 = a;
      uint32_t v4 = a;
      uint64_t v5 = a;

      ASSERT_EQ(v1,1);
      ASSERT_EQ(v2,1);
      ASSERT_EQ(v3,1);
      ASSERT_EQ(v4,1);
      ASSERT_EQ(v5,1);
   }

   TEST_F(ABigintTest, LogicalOpsPair) {
      uint128p_t a("18446744073709551617");
      uint128p_t b(0);

      ASSERT_EQ(!a,false);
      ASSERT_EQ(a && true,true);
      ASSERT_EQ(!b,true);
      ASSERT_EQ(a&&b,false);
      ASSERT_EQ(b&&a,false);
      ASSERT_EQ(a||b,true);
      ASSERT_EQ(b||a,true);
   }

   TEST_F(ABigintTest, ComparisonOpsPair) {
      uint128p_t a(1,1);
      uint128p_t b(1,0);
      uint128p_t c(10);
      
      ASSERT_NE(a,b);
      ASSERT_EQ(a,b+1);
      ASSERT_EQ(a-1,b);

      ASSERT_TRUE(a>b);
      ASSERT_TRUE(b<a);

      ASSERT_TRUE(a>=b);
      ASSERT_TRUE(b<=a);

      ASSERT_TRUE(a>=(b+1));
      ASSERT_TRUE(b<=(a-1));

      ASSERT_NE(c,11);
      ASSERT_EQ(c,10);

      ASSERT_TRUE(c>9);
      ASSERT_TRUE(9<c);

      ASSERT_TRUE(c>=10);
   }

   TEST_F(ABigintTest, BooleanOpsPair) {
      uint128p_t a(0);
      uint128p_t b(0x10101010UL,0x10101010UL);
      uint128p_t c(a);

      a |= b;
      ASSERT_EQ(a,b);

      a ^= b;
      ASSERT_EQ(a,c);

      b &= c;
      ASSERT_EQ(b,c);

      b |= 0xf0f0f0f0UL;
      a |= b;
      ASSERT_EQ(a,0xf0f0f0f0UL);

      a = ~b;
      a |= 0xf0f0f0f0;
      ASSERT_TRUE((~a).isZero());
   }

   TEST_F(ABigintTest, BitShiftOpsPair) {
      uint128p_t a(1);
      uint128p_t b(a);
      uint128p_t c(0x8000000000000000UL,0UL);

      b <<= 127;
      ASSERT_EQ(b,c);
      b <<= 1;
      ASSERT_EQ(b,0);

      b = a << 63;
      ASSERT_TRUE(b.isLow());
      b <<= 1;
      ASSERT_FALSE(b.isLow());

      a >>= 1;
      ASSERT_EQ(a,0);

      c >>= 200;
      ASSERT_EQ(c,0);
   }

   TEST_F(ABigintTest, AddOpsPair) {
      uint128p_t a(0);
      uint128p_t b("18446744073709551617");
      uint128p_t c;

      ++a;
      ASSERT_EQ(a,1);
      c = a++;
      ASSERT_EQ(a,2);
      ASSERT_EQ(c,1);

      a += 1;
      ASSERT_EQ(a,3);
      ASSERT_EQ(a+1,4);
      ASSERT_EQ(a,3);

      a += 0xfffffffffffffffc;
      ASSERT_TRUE(a.isLow());
      ++a;
      ASSERT_FALSE(a.isLow());
      ASSERT_EQ(a+1,b);

      a = "340282366920938463463374607431768211455";
      ASSERT_EQ(a+1,0);
   }

   TEST_F(ABigintTest, SubtractOpsPair) {
      uint128p_t a(0);
      uint128p_t b("340282366920938463463374607431768211455");
      uint128p_t c(1,0);

      --a;
      ASSERT_EQ(a,b);
      a = 0;
      a--;
      ASSERT_EQ(a,b);

      ASSERT_FALSE(c.isLow());
      c -= 1;
      ASSERT_TRUE(c.isLow());
      ASSERT_EQ(c,0xffffffffffffffffUL);
      ASSERT_EQ(c-0xffffffffffffffffUL,0);
   }

   TEST_F(ABigintTest, MultiplyOpsPair) {
      uint128p_t a(0);
      uint128p_t b(1);
      uint128p_t c(10);

      a *= 10;
      a *= 1;
      ASSERT_EQ(a,0);

      b *= 2*2*2*2*2*2*2*2;
      ASSERT_EQ(b,256);

      b =  "10000000000000000000000000000000000000";
      c *= 1000000;
      c *= 1000000;
      c *= 1000000;
      c *= 1000000;
      c *= 1000000;
      c *= 1000000;
      
      ASSERT_EQ(c,b);
      c *= 10;
      b =  "100000000000000000000000000000000000000";
      ASSERT_EQ(c,b);

      c *= 10;
      b  = "319435266158123073073250785136463577088";
      ASSERT_EQ(c,b);
   }

   TEST_F(ABigintTest, DivideOpsPair) {
      uint128p_t a("340282366920938463463374607431768211455");
      uint128p_t b("10000000000000000000000");
      uint128p_t c;
      string s;

      c = a / b;
      ASSERT_EQ(c,34028236692093846UL);

      c = a % b;
      c.toString(s);
      ASSERT_EQ(c, "3463374607431768211455");

      c = a / 2 / 2 / 2 / 2;
      c.toString(s);
      ASSERT_EQ(c, "21267647932558653966460912964485513215");

      ASSERT_THROW(a/0, CRException);
   }

#ifdef INT128_INTRINSIC
   // Tests biginteger construction, and printing, using the intrinsic implementation
   TEST_F(ABigintTest, ConstructionOpsIntrinsic) {
      uint128_t a;
      uint128_t b("18446744073709551617");
      uint128_t c(9223372036854775808UL);
      uint128_t d(1,1);
      uint128_t e(d);

      ASSERT_FALSE(a.isZero());
      ASSERT_EQ(b,d);
      ASSERT_EQ(d,e);

      c <<= 1;
      ++c;
      ASSERT_EQ(c,b);

      string s;
      ASSERT_TRUE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);

      uint128_t f(s);
      ASSERT_EQ(f,b);
   };

   TEST_F(ABigintTest, AssignmentOpsIntrinsic) {
      uint128_t b = "18446744073709551617";
      uint128_t c(9223372036854775808UL);

      uint128_t d = b;
      uint128_t e = 1;

      c <<= 1;
      c += e;

      ASSERT_EQ(b,d);
      ASSERT_EQ(b,c);

      string s = "18446744073709551617";
      c = s;
      ASSERT_TRUE(strcmp(c.toString(s).c_str(), "18446744073709551617") == 0);
   }

   TEST_F(ABigintTest, ConversionOpsIntrinsic) {
      uint128_t a("18446744073709551617");

      bool v1 = a;
      uint8_t v2 = a;
      uint16_t v3 = a;
      uint32_t v4 = a;
      uint64_t v5 = a;

      ASSERT_EQ(v1,1);
      ASSERT_EQ(v2,1);
      ASSERT_EQ(v3,1);
      ASSERT_EQ(v4,1);
      ASSERT_EQ(v5,1);
   }

   TEST_F(ABigintTest, LogicalOpsIntrinsic) {
      uint128_t a("18446744073709551617");
      uint128_t b(0);

      ASSERT_EQ(!a,false);
      ASSERT_EQ(a && true,true);
      ASSERT_EQ(!b,true);
      ASSERT_EQ(a&&b,false);
      ASSERT_EQ(b&&a,false);
      ASSERT_EQ(a||b,true);
      ASSERT_EQ(b||a,true);
   }

   TEST_F(ABigintTest, BooleanOpsIntrinsic) {
      uint128_t a(0);
      uint128_t b(0x10101010UL,0x10101010UL);
      uint128_t c(a);

      a |= b;
      ASSERT_EQ(a,b);

      a ^= b;
      ASSERT_EQ(a,c);

      b &= c;
      ASSERT_EQ(b,c);

      b |= 0xf0f0f0f0UL;
      a |= b;
      ASSERT_EQ(a,0xf0f0f0f0UL);

      a = ~b;
      a |= 0xf0f0f0f0;
      ASSERT_TRUE((~a).isZero());
   }

   TEST_F(ABigintTest, BitShiftOpsIntrinsic) {
      uint128_t a(1);
      uint128_t b(a);
      uint128_t c(0x8000000000000000UL,0UL);

      b <<= 127;
      ASSERT_EQ(b,c);
      b <<= 1;
      ASSERT_EQ(b,0);

      b = a << 63;
      ASSERT_TRUE(b.isLow());
      b <<= 1;
      ASSERT_FALSE(b.isLow());

      a >>= 1;
      ASSERT_EQ(a,0);

      c >>= 200;
      ASSERT_EQ(c,0);
   }

   TEST_F(ABigintTest, AddOpsIntrinsic) {
      uint128_t a(0);
      uint128_t b("18446744073709551617");
      uint128_t c;

      ++a;
      ASSERT_EQ(a,1);
      c = a++;
      ASSERT_EQ(a,2);
      ASSERT_EQ(c,1);

      a += 1;
      ASSERT_EQ(a,3);
      ASSERT_EQ(a+1,4);
      ASSERT_EQ(a,3);

      a += 0xfffffffffffffffc;
      ASSERT_TRUE(a.isLow());
      ++a;
      ASSERT_FALSE(a.isLow());
      ASSERT_EQ(a+1,b);

      a = "340282366920938463463374607431768211455";
      ASSERT_EQ(a+1,0);
   }

   TEST_F(ABigintTest, SubtractOpsIntrinsic) {
      uint128_t a(0);
      uint128_t b("340282366920938463463374607431768211455");
      uint128_t c(1,0);

      --a;
      ASSERT_EQ(a,b);
      a = 0;
      a--;
      ASSERT_EQ(a,b);

      ASSERT_FALSE(c.isLow());
      c -= 1;
      ASSERT_TRUE(c.isLow());
      ASSERT_EQ(c,0xffffffffffffffffUL);
      ASSERT_EQ(c-0xffffffffffffffffUL,0);
   }

   TEST_F(ABigintTest, MultiplyOpsIntrinsic) {
      uint128_t a(0);
      uint128_t b(1);
      uint128_t c(10);

      a *= 10;
      a *= 1;
      ASSERT_EQ(a,0);

      b *= 2*2*2*2*2*2*2*2;
      ASSERT_EQ(b,256);

      b =  "10000000000000000000000000000000000000";
      c *= 1000000;
      c *= 1000000;
      c *= 1000000;

      a  = 1000000;
      c *= a;
      c *= a;
      c *= a;
      
      ASSERT_EQ(c,b);
      c *= 10;
      b =  "100000000000000000000000000000000000000";
      ASSERT_EQ(c,b);

      c *= 10;
      b  = "319435266158123073073250785136463577088";
      ASSERT_EQ(c,b);
   }

   TEST_F(ABigintTest, DivideOpsIntrinsic) {
      uint128_t a("340282366920938463463374607431768211455");
      uint128_t b("10000000000000000000000");
      uint128_t c;
      string s;

      c = a / b;
      ASSERT_EQ(c,34028236692093846UL);

      c = a % b;
      c.toString(s);
      ASSERT_EQ(c, "3463374607431768211455");

      c = a / 2 / 2 / 2 / 2;
      c.toString(s);
      ASSERT_EQ(c, "21267647932558653966460912964485513215");

      ASSERT_THROW(a/0, CRException);
   }

#endif

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
