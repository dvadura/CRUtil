#include "gtest/gtest.h"
#include "crhttp.h"
#include "crunnable.h"
#include "condition.h"
#include "socket.h"

using namespace std;
using namespace crunnable;

namespace badu {
static const char* TEXT = "GET /index.html?foo=x&bar=y HTTP/1.1\r\n"
       "Host: 192.241.213.46:6880\r\n"
       "Upgrade-Insecure-Requests: 1\r\n" 
       "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
       "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_3) AppleWebKit/602.4.8 (KHTML, like Gecko) Version/10.0.3 Safari/602.4.8\r\n"
       "Accept-Language: en-us\r\n"
       "Accept-Encoding: gzip, deflate\r\n"
       "Connection: keep-alive\r\n\r\n";

   class Server : public CRunnable, public Condition {
   protected:
      Socket &sk;

   public:
      Server(Socket &sock) : CRunnable((size_t) 32), sk(sock) {
         if (sk.getLocalAddress().isAny() == true) {
            sk.swap();
         }
         sk.open();
         start(true);
      }

      virtual ~Server() {
         waitStop(NS_IN_ONE_MSEC, 0);
         sk.close();
      }

      virtual int run() = 0;
   };

   class UnixServer : public Server {
   private:
      CRHttpRequest m_req;

   public:
      UnixServer(UnixSocket &sock) : Server(sock) {}

      virtual ~UnixServer() {
         setCancelTimeout(NS_IN_ONE_MSEC);
      }

      virtual int run() {
         service();

         try {
            UnixSocket& usk = reinterpret_cast<UnixSocket&>(sk);

            usk.bind();
            usk.listen();

            raise();

            while (isTerminated() == false) {
               UnixSocket* client = usk.accept(true);

               if (client == NULL) {
                  continue;
               }

               try {
                  size_t tmp;
                  char buf[64];

                  m_req.parse(client);
                  m_req.terminate();
                  CRSnprintf(buf, "%s", m_req.header(CRHttp::HOST,tmp));
                  client->write(buf);
                  delete client;
               }
               catch (CRException& e) {
                 CRX_REPORT_CATCH(stderr, e);
               }
            }
         }
         catch (CRException& e) {
            if (isTerminated() == false) {
               CRX_REPORT_CATCH(stderr, e);
            }
         }

         return 0;
      }
   };

   // The fixture for testing class Foo.
   class HTTPTest : public ::testing::Test {
      public:
      protected:
      // You can remove any or all of the following functions if its body
      // is empty.
      HTTPTest() { }

      virtual ~HTTPTest() {
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
   };

   // Tests that the parser works
   TEST_F(HTTPTest, TEXT) {
      CRHttpRequest req;

      try {
         try {
            ssize_t res = req.parse(TEXT, strlen(TEXT));
            ASSERT_EQ(res,381);
         }
         catch (CRException& e) {
           CRX_REPORT_CATCH(stderr, e);
         }
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   };

   TEST_F(HTTPTest, SOCK) {
      char buf[1024];

      try {
         // Create the client IPv4 socket, and connect to the server
         SockAddrUnix clnt("/tmp/unix_unit_test.clnt");
         SockAddrUnix srvr("/tmp/unix_unit_test.srvr");

         UnixSocket* sk = UnixSocket::getSocket(srvr);
         UnixServer* server = new UnixServer(*sk);

         // wait for the server to be ready
         server->waitFor();

         // Create the client IPv4 socket, and connect to the server
         UnixSocket ck(clnt,srvr);
         ck.sopen();
         ck.bind();
         ck.connect();

         // Read and validate the response
         ssize_t size = ck.write(TEXT, strlen(TEXT)+1, true);
         ASSERT_EQ(size, 382);

         size = ck.read(buf, 1023, true);
         buf[size] = '\0';
         ASSERT_STREQ(buf, "192.241.213.46:6880");

         // Clean up the server
         ck.close();
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         ASSERT_TRUE(false);
      }
   };

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
