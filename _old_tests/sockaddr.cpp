#include "gtest/gtest.h"
#include "crunnable.h"
#include "sockaddr.h"

using namespace std;
using namespace crunnable;

namespace badu {
   // The fixture for testing class Foo.
   class SockAddrTest : public ::testing::Test {
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.

      SockAddrTest() {
         // You can do set-up work for each test here.
      }

      virtual ~SockAddrTest() {
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

   class Server : public CRunnable {
      int m_sock;

   public:
      Server(int sock) : m_sock(sock) {}

      ~Server() {
         waitStop(NS_IN_ONE_SEC);
         close(m_sock); 
      }

      virtual int run() {
         while (isTerminated() == false) {
            int client = accept(m_sock, NULL, NULL);

            if (client >= 0) {
               write(client, "got it", 7);
               shutdown(client, SHUT_RDWR);
               close(client);
            }
         }

         return 0;
      }
   };

   // Tests that the Ip4Addr does correct conversions
   TEST_F(SockAddrTest, Ip4Addr) {
      Ip4Addr a;
      Ip4Addr b("10.0.0.1");
      in_addr_t x = 0x0100000a;
      Ip4Addr c(x);
      Ip4Addr d(167772161);
      Ip4Addr z((unsigned int) 0);

      ASSERT_EQ(4,sizeof(a));
      ASSERT_EQ(4,a.addrlen());
      ASSERT_THROW(a = (char *)NULL, CRException);
      ASSERT_NE(b,c);
      ASSERT_EQ(b,d);
      ASSERT_NE(c,d);

      struct in_addr i4a;
      i4a.s_addr = x;
      Ip4Addr e(i4a);
      ASSERT_EQ(b,e);

      Ip4Addr f(0x0a000001);
      ASSERT_EQ(b,f);
      ASSERT_EQ(d,f);
      ASSERT_EQ(e,f);

      string str;
      a = b;
      a.toString(str);
      ASSERT_STREQ(str.c_str(), "10.0.0.1");
      c.toString(str);
      ASSERT_STREQ(str.c_str(), "1.0.0.10");
      z.toString(str);
      ASSERT_STREQ(str.c_str(), "0.0.0.0");

      f = "10.0.0.10";
      a = 0x0a000009;
      ASSERT_TRUE(a.match(f,24));
      ASSERT_TRUE(a.match(f,30));
      ASSERT_FALSE(a.match(f,31));
      ASSERT_FALSE(a.match(f,48));

      Ip4Addr i4("10.0.0.1");
      ASSERT_STREQ(i4.toString(str).c_str(), "10.0.0.1");
   }

   // Tests that the Ip6Addr does correct conversions
   TEST_F(SockAddrTest, Ip6Addr) {
      Ip6Addr a;
      Ip6Addr b("ffef::10.0.1.1");
      uint128_t x(0xffef000000000000UL, 0x0a000101UL);
      Ip6Addr c(x);

      ASSERT_EQ(16,sizeof(a));
      ASSERT_EQ(16,a.addrlen());
      ASSERT_THROW(a = (char *)NULL, CRException);
      ASSERT_NO_THROW(a = "10.0.1.9");

      a = b;
      ASSERT_EQ(a,b);

      struct in6_addr i6a;
      inet_pton(AF_INET6, "ffef::10.0.1.1", &i6a);
      ASSERT_EQ(b,i6a);

      b = "ffef::10.0.1.3";

      string str;
      a.toString(str);
      b.toString(str);

      ASSERT_TRUE(a.match(b,16));
      ASSERT_TRUE(a.match(b,64));
      ASSERT_TRUE(a.match(b,128-16));
      ASSERT_TRUE(a.match(b,128-2));
      ASSERT_FALSE(a.match(b,128-1));

      ASSERT_NO_THROW(a = "0::10.1.1.1");

      Ip6Addr i6("ffef::10.0.1.1");
      ASSERT_STREQ(i6.toString(str).c_str(), "ffef::a00:101");

      ASSERT_STREQ(Ip6Addr::ANY.toString(str).c_str(), "::");

      i6.setAny();
      ASSERT_STREQ(i6.toString(str).c_str(), "::");

      i6 = "::";
      ASSERT_STREQ(i6.toString(str).c_str(), "::");
      ASSERT_TRUE(i6.isAny());
      
      i6 = "ffff:1111:2222:3333:4444:ffee:1234:1234";
      ASSERT_STREQ(i6.toString(str).c_str(), "ffff:1111:2222:3333:4444:ffee:1234:1234");
      i6 = "aaaa::bbbb:7777";
      ASSERT_STREQ(i6.toString(str).c_str(), "aaaa::bbbb:7777");

      i6 = "10.0.0.1";
      ASSERT_TRUE(i6.isIP4in6());

      Ip4Addr i4(i6.ip4());
      ASSERT_STREQ(i4.toString(str).c_str(), "10.0.0.1");

      i4 = i6.ip4();
      ASSERT_STREQ(i4.toString(str).c_str(), "10.0.0.1");

      i6 = i4;
      ASSERT_TRUE(i6.isIP4in6());

      Ip6Addr z = Ip6Addr(0xfefe000000000000,0x000000000000abab);
      ASSERT_STREQ(z.toString(str).c_str(), "fefe::abab");
   }

   void validate_mask(const char *result, Ip4Addr& out);
   
   TEST_F(SockAddrTest, Ip4AddrNetworkAddress) {
      Ip4Addr a("254.255.255.255");
      Ip4Addr b("192.168.4.32");
      Ip4Addr c("172.17.2.7");
      Ip4Addr d("172.16.1.35");
      Ip4Addr result;
      string s;
      ASSERT_STREQ("254.0.0.0", a.getNetworkAddressFromSubnet(8, result).toString(s).c_str());
      ASSERT_STREQ("192.168.0.0", b.getNetworkAddressFromSubnet(16, result).toString(s).c_str());
      ASSERT_STREQ("172.17.2.0", c.getNetworkAddressFromSubnet(24, result).toString(s).c_str());
      ASSERT_STREQ("172.16.1.32", d.getNetworkAddressFromSubnet(27,result).toString(s).c_str());
      ASSERT_STREQ("172.16.1.35", d.getNetworkAddressFromSubnet(32,result).toString(s).c_str());
   }

   TEST_F(SockAddrTest, Ip6AddrNetworkAddress) {
      Ip6Addr a("fefe:ffff:ffff:ffff:ffff:ffff:ffff:abab");
      Ip6Addr b("fefe:1234:5678::abab");
      Ip6Addr c("baba:1111:2222:4444::3333");
      Ip6Addr d("eeee:aaaa:1234::4567");
      Ip6Addr result;
      string s;
 
      ASSERT_STREQ("fefe::", a.getNetworkAddressFromSubnet(16, result).toString(s).c_str());
      ASSERT_STREQ("fefe:1234:5678::", b.getNetworkAddressFromSubnet(48, result).toString(s).c_str());
      ASSERT_STREQ("baba:1111:2222:4444::", c.getNetworkAddressFromSubnet(64, result).toString(s).c_str());
      ASSERT_STREQ("eeee:aaaa:1234::4560", d.getNetworkAddressFromSubnet(124, result).toString(s).c_str());
      ASSERT_STREQ("eeee:aaaa:1234::4567", d.getNetworkAddressFromSubnet(128, result).toString(s).c_str());
   }

   TEST_F(SockAddrTest, Ip6AddrUint128) {
      Ip6Addr b("ffef::10.0.1.1");
      uint128_t x(0xffef000000000000UL, 0x0a000101UL);
      Ip6Addr c(x);

      ASSERT_EQ(b,c);
      string s;
      c.toString(s);
      ASSERT_EQ(s,string("ffef::a00:101"));

      ASSERT_TRUE(b <= c);
      ASSERT_TRUE(c <= b);
      ASSERT_FALSE(b < c);
      ASSERT_FALSE(c < b);
   }

   // Use a SockAddrIn4 and SockAddrIn6 to test SockAddrIn
   TEST_F(SockAddrTest, SockAddrIn) {
      SockAddrIn4 in4;
      SockAddrIn4 in4_1024((in_port_t) 1024);
      SockAddrIn6 in6((in_port_t) 10000);
      SockAddrIn6 in6_96((in_port_t) 96, 96);

      ASSERT_TRUE(in4.isAny());
      ASSERT_TRUE(in4.port() == 0);
      ASSERT_TRUE(in4_1024.port() == 1024);
      ASSERT_TRUE(in6.port() == 10000);
      ASSERT_TRUE(in6_96.port() == 96);

      in4.setPort(9126);
      ASSERT_TRUE(in4.port() == 9126);

      in4.setPort(26);
      ASSERT_TRUE(in4.port() == 26);

      in6.setPort(26);
      ASSERT_TRUE(in6.port() == 26);

      ASSERT_TRUE(in4.isIP4());
      ASSERT_TRUE(in6.isIP6());

      ASSERT_FALSE(in4.isIP6());
      ASSERT_FALSE(in6.isIP4());

      ASSERT_TRUE(in4.family() == AF_INET);
      ASSERT_TRUE(in6.family() == AF_INET6);

      string output;
      SockAddrIn& i4b = in4;
      in4.zero();
      i4b.toString(output);
      ASSERT_STREQ(output.c_str(), "0.0.0.0");

      in4.setPort(26);
      i4b.toString(output);
      ASSERT_STREQ(output.c_str(), "0.0.0.0:26");

      SockAddrIn& i6b = in6;
      in6.setPort(10000);
      i6b.toString(output);
      ASSERT_STREQ(output.c_str(), "::.10000");

      in6.zero();
      i6b.toString(output);
      ASSERT_STREQ(output.c_str(), "::");

      SockAddrIn& i6c = in6_96;
      i6c.toString(output);
      ASSERT_STREQ(output.c_str(), "::.96/96");

      in4.setLocal();
      ASSERT_FALSE(in4.isAddrAny());

      in6.setLocal();
      ASSERT_FALSE(in6.isAddrAny());

      in4.setAny();
      ASSERT_TRUE(in4.isAddrAny());

      in6.setAny();
      ASSERT_TRUE(in6.isAddrAny());
   }

   // Tests that the SockAddr works
   TEST_F(SockAddrTest, SockAddrIP4) {
      SockAddrIn4 listener("127.0.0.1:11000");
      SockAddrIn4 clnt;
      string      tmp;

      // Make sure that the size is what we expect
      ASSERT_EQ(listener.addrlen(), 16);

      // Make sure that we converted correctly
      ASSERT_STREQ(listener.toString(tmp).c_str(), "127.0.0.1:11000");

      // Make sure we hash correctly
      ASSERT_EQ(listener.hash64(), 2016407666861855968ULL);

      SockAddrIn4 sa32((uint32_t*) listener.addr32());
      sa32.setPort(listener.port());
      ASSERT_EQ(listener, sa32);

      int flag = 1;
      int sock;
      errno = 0;

      // Create the listener IPv4 socket and start the server
      sock = socket(listener.family(), SOCK_STREAM, 0);        
      ASSERT_EQ(errno, 0);

      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &flag, (socklen_t) sizeof(flag));
      ASSERT_EQ(errno, 0);

      bind(sock, listener.to_sa(), listener.addrlen());
      ASSERT_EQ(errno, 0);

      listen(sock, 100);
      ASSERT_EQ(errno, 0);

      Server srvr(sock);
      srvr.start(true);

      // Create the client IPv4 socket, and connect to the server
      int csock = socket(listener.family(), SOCK_STREAM, 0);
      ASSERT_EQ(errno, 0);

      bind(csock, clnt.to_sa(), clnt.addrlen());
      ASSERT_EQ(errno, 0);

      connect(csock, listener.to_sa(), listener.addrlen());    
      ASSERT_EQ(errno, 0);

      // Read and validate the response
      char buf[24];
      read(csock, buf, 7);

      ASSERT_STREQ(buf, "got it");
      shutdown(csock, SHUT_RDWR);
      close(csock);
      srvr.stop();

      clnt.setLocal();
      ASSERT_TRUE(clnt.isLocal());

      SockAddrIn4 any = "0:1";
      SockAddrIn4 loc = "127.0.0.1:1";
      ASSERT_STREQ(any.toString(tmp).c_str(), "0.0.0.0:1");
      ASSERT_STREQ(loc.toString(tmp).c_str(), "127.0.0.1:1");

      ASSERT_EQ(loc.match(any), true);
   }

   // Tests that the SockAddr works
   TEST_F(SockAddrTest, SockAddrIP6) {
      SockAddrIn6 listener("::1.10000");
      SockAddrIn6 clnt;
      string      tmp;

      // Make sure that the size is what we expect
      ASSERT_EQ(listener.addrlen(), 28);
      ASSERT_EQ(listener.hash64(), 9620721746028789629ULL);

      // Make sure that we converted correctly
      ASSERT_STREQ(listener.toString(tmp).c_str(), "::1.10000");

      string lstr;
      SockAddrIn6 ll("ffff:ac00::ffff:0.0.0.172.1024");
      SockAddrIn6 sa32((uint32_t*) ll.addr32());
      sa32.setPort(ll.port());
      ASSERT_EQ(ll.family(), sa32.family());
      ASSERT_EQ(ll.scope(), sa32.scope());
      ASSERT_EQ(ll, sa32);

      int flag = 1;
      int sock;
      errno = 0;

      // Create the listener IPv6 socket and start the server
      sock = socket(listener.family(), SOCK_STREAM, 0);        
      ASSERT_EQ(errno, 0);

      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &flag, (socklen_t) sizeof(flag));
      ASSERT_EQ(errno, 0);

      bind(sock, listener.to_sa(), listener.addrlen());
      ASSERT_EQ(errno, 0);

      listen(sock, 100);
      ASSERT_EQ(errno, 0);

      Server srvr(sock);
      srvr.start(true);

      // Create the client IPv4 socket, and connect to the server
      int csock = socket(listener.family(), SOCK_STREAM, 0);   
      ASSERT_EQ(errno, 0);

      bind(csock, clnt.to_sa(), clnt.addrlen());           
      ASSERT_EQ(errno, 0);

      connect(csock, listener.to_sa(), listener.addrlen());    
      ASSERT_EQ(errno, 0);

      // Read and validate the response
      char buf[24];
      read(csock, buf, 7);

      ASSERT_STREQ(buf, "got it");
      shutdown(csock, SHUT_RDWR);
      close(csock);
      srvr.stop();

      clnt.setLocal();
      ASSERT_TRUE(clnt.isLocal());
   }

   TEST_F(SockAddrTest, AddressConversions) {
      string tmp;

      SockAddrIn4 s4("10.0.0.1", 80);
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.0.0.1:80");

      s4 = "10.1.1.1";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.1.1.1");
      ASSERT_EQ(s4.port(), 0);
      ASSERT_EQ(s4.family(), AF_INET);

      SockAddrIn& s4ref = s4;
      s4ref = "10.1.1.2:80";
      ASSERT_STREQ(s4ref.toString(tmp).c_str(), "10.1.1.2:80");
      ASSERT_EQ(s4ref.port(), 80);
      ASSERT_EQ(s4ref.family(), AF_INET);

      s4 = "10.1.1.1.1024";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.1.1.1:1024");
      ASSERT_EQ(s4.port(), 1024);
      ASSERT_EQ(s4.family(), AF_INET);

      s4 = "172:1024";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "0.0.0.172:1024");
      ASSERT_EQ(s4.port(), 1024);
      ASSERT_EQ(s4.family(), AF_INET);

      s4 = "172.1024";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "0.0.0.172:1024");
      ASSERT_EQ(s4.port(), 1024);
      ASSERT_EQ(s4.family(), AF_INET);

      SockAddrIn6 s6("::.1024");
      ASSERT_STREQ(s6.toString(tmp).c_str(), "::.1024");
      ASSERT_EQ(s6.port(), 1024);
      ASSERT_EQ(s6.family(), AF_INET6);

      s6 = s4;
      ASSERT_STREQ(s6.toString(tmp).c_str(), "::ffff:0.0.0.172.1024");
      ASSERT_EQ(s6.port(), 1024);
      ASSERT_EQ(s6.family(), AF_INET6);

      s6 = "2222::2.2.2.2";
      ASSERT_STREQ(s6.toString(tmp).c_str(), "2222::202:202");
      ASSERT_EQ(s6.port(), 0);
      ASSERT_EQ(s6.family(), AF_INET6);

      s6 = "2222::2.2.2.2.1024";
      ASSERT_STREQ(s6.toString(tmp).c_str(), "2222::202:202.1024");
      ASSERT_EQ(s6.port(), 1024);
      ASSERT_EQ(s6.family(), AF_INET6);

      s6 = "ffef::1.1024";
      ASSERT_STREQ(s6.toString(tmp).c_str(), "ffef::1.1024");
      ASSERT_EQ(s6.port(), 1024);
      ASSERT_EQ(s6.family(), AF_INET6);

      SockAddrIn& s6ref = s6;
      s6ref = "ffef::2.1020";
      ASSERT_STREQ(s6ref.toString(tmp).c_str(), "ffef::2.1020");
      ASSERT_EQ(s6ref.port(), 1020);
      ASSERT_EQ(s6ref.family(), AF_INET6);

      ASSERT_NO_THROW(s6 = "2222::2.2.2.2:1024");
      ASSERT_THROW(s6 = (char*)NULL, CRException);

      s6 = "10.0.0.1:80";
      s4 = s6;
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.0.0.1:80");

      s4 = (uint32_t) 0;
      ASSERT_STREQ(s4.toString(tmp).c_str(), "0.0.0.0");

      s4 = "0.0.0.0";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "0.0.0.0");

      s4 = "0.0.0.0:0";
      ASSERT_STREQ(s4.toString(tmp).c_str(), "0.0.0.0");

      s6 = (uint128_t) 0;
      ASSERT_STREQ(s6.toString(tmp).c_str(), "::");

      s6 = "::.0";
      ASSERT_STREQ(s6.toString(tmp).c_str(), "::");

      s6 = "::";
      ASSERT_STREQ(s6.toString(tmp).c_str(), "::");
      
      ASSERT_THROW(s4 = "::2002", CRException);
      SockAddrIn *assign_v6 = SockAddrIn::getAddress("::2");
      ASSERT_FALSE(assign_v6->isIP4());
      ASSERT_TRUE(assign_v6->isIP6());
   }

   TEST_F(SockAddrTest, QuadConversions) {
      string tmp;

      IP4Quad q("<2.2.2.2.80,2.2.2.2:80>");
      ASSERT_STREQ(q.toString(tmp).c_str(), "<2.2.2.2:80,2.2.2.2:80>");

      ASSERT_THROW(q = "<172.17.5.20.52265,0::172.17.2.2.80>", CRException);

      IP4Quad r = q;
      ASSERT_TRUE(r == q);

      IP4Quad q2 = "<172.17.5.20.52265,172.17.2.2.80>";
      IP4Quad q3 = "<172.17.2.2.80,172.17.5.20.52265>";

      IP4Quad q1 = "<172.17.5.20.52265,172.17.2.2.80>";
      IP4Quad q4 = "<172.17.5.20.52265,172.17.2.2.800>";
      ASSERT_TRUE(q1 == q2);

      ASSERT_FALSE(q1 < q2);
      ASSERT_FALSE(q2 < q1);

      ASSERT_FALSE(q1 < q3);
      ASSERT_TRUE(q3 < q1);

      ASSERT_FALSE(q4 < q1);
      ASSERT_TRUE(q1 < q4);

      q2.swap();
      ASSERT_STREQ(q2.toString(tmp).c_str(), "<172.17.2.2:80,172.17.5.20:52265>");

      SockAddrIn6 s6("::2002");
      IP6Quad q6any(SockAddrIn6::ANY, s6); 
      ASSERT_FALSE(q6any.isIP4());
      ASSERT_TRUE(q6any.isIP6());
   }

   TEST_F(SockAddrTest, SockAddrInAssignment) {
      string tmp;

      SockAddrIn4 s4("10.0.0.1:80");
      SockAddrIn& s4ref = s4;

      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.0.0.1:80");

      SockAddrIn4 s5("10.0.0.2:8000");
      SockAddrIn& s5ref = s5;

      s4ref = s5ref;
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.0.0.2:8000");

      SockAddrIn6 s6("10.2.2.2:9000");
      SockAddrIn& s6ref = s6;

      s4ref = s6ref;
      ASSERT_STREQ(s4.toString(tmp).c_str(), "10.2.2.2:9000");

      s6 = "::10.3.3.3:10000";
      ASSERT_THROW(s4ref = s6ref, CRException);
   }

   TEST_F(SockAddrTest, SockAddrInHash) {
      IP4Quad q4 = "<172.17.2.2.443,172.17.5.21.42266>";
      IP4Quad s4 = q4.swap();

      IP6Quad q6 = "<::ffff:0.0.0.172.1024,::ffff:0.0.0.172.1024>";
      IP6Quad s6 = q6.swap();

      ASSERT_EQ(q4.hash32(), s4.hash32());
      ASSERT_EQ(q6.hash32(), s6.hash32());

      q6 = "<::ffff:0.0.0.172.1023,::ffff:0.1.4.172.1025>";
      s6 = q6.swap();

      ASSERT_EQ(q6.hash32(), s6.hash32());
   }
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
