#include "gtest/gtest.h"
#include "crunnable.h"
#include "socket.h"

using namespace std;
using namespace crunnable;

namespace badu {
   class Server : public CRunnable, public Condition {
   protected:
      Socket &sk;

   public:
      Server(Socket &sock) : sk(sock) {
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

   class TCPServer : public Server {
   public:
      TCPServer(IPSocket &sock) : Server(sock) {}

      virtual ~TCPServer() {
         setCancelTimeout(NS_IN_ONE_MSEC);
      }

      virtual int run() {
         service();

         try {
            TCPSocket& tsk = reinterpret_cast<TCPSocket&>(sk);

            tsk.reuse();
            tsk.bind();
            tsk.listen();

            raise();

            while (isTerminated() == false) {
               TCPSocket *client = tsk.accept(true);

               if (client == NULL) {
                  continue;
               }

               client->write("got it");
               delete client;
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

   class TCPServerDG : public Server {
   public:
      TCPServerDG(IPSocket &sock) : Server(sock) {}

      virtual ~TCPServerDG() {
         setCancelTimeout(NS_IN_ONE_MSEC);
      }

      virtual int run() {
         service();

         try {
            TCPSocket& tsk = reinterpret_cast<TCPSocket&>(sk);

            tsk.reuse();
            tsk.bind();
            tsk.listen();

            raise();

            while (isTerminated() == false) {
               TCPSocket *client = tsk.accept(true);

               if (client == NULL) {
                  continue;
               }

               char buf[100];
               ssize_t len;

               memset(buf, '\0', sizeof(buf));
               if ((len=client->recv(buf, 100)) >= 0) {
                  buf[len] = '\0';
                  client->send(buf, (size_t) len);
               }
               
               delete client;
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

   class UDPServer : public Server {
   public:
      UDPServer(IPSocket &sock) : Server(sock) {}

      virtual ~UDPServer() {
         setCancelTimeout(NS_IN_ONE_MSEC);
      }

      virtual int run() {
         service();

         try {
            UDPSocket& usk = reinterpret_cast<UDPSocket&>(sk);

            usk.reuse();
            usk.bind();

            raise();

            while (isTerminated() == false) {
               char buf[100];
               string ip;
               ssize_t len;

               memset(buf, '\0', sizeof(buf));
               if ((len=usk.recv(buf, 100)) >= 0) {
                  buf[len] = '\0';
                  usk.send(buf, len);
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

   class UnixServer : public Server {
   public:
      UnixServer(UnixSocket &sock) : Server(sock) {
         sock.mkpath();
      }

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

               client->write("got it");
               delete client;
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
   class SocketTest : public ::testing::Test {
      protected:
      SockAddrIn4 ip4;
      SockAddrIn6 ip6;

      // You can remove any or all of the following functions if its body
      // is empty.
      SocketTest() : ip4("127.0.0.1:11000"), ip6("::1:11000")
      {
         return;
      }

      virtual ~SocketTest() {
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

   // Tests that the UDP Socket IPV4 works, using datagram
   TEST_F(SocketTest, UDPSocket4) {
      char buf[1024];

      // Standard datagram
      try {
         UDP4Socket sk(ip4);
         UDPServer* server = new UDPServer(sk);

         // Create the client IPv4 socket, and connect to the server
         UDP4Socket clnt(SockAddrIn4::ANY, ip4);
         clnt.open().bind(true);

         // wait for the server to be ready
         server->waitFor();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the UDP Socket IPV4 works, using connected datagram
   TEST_F(SocketTest, UDPSocket4Connected) {
      char buf[1024];

      // Connected datagram with filter IP4
      try {
         UDP4Socket sk(ip4);
         UDPServer* server = new UDPServer(sk);

         // Create the client IPv4 socket, and connect to the server
         UDP4Socket clnt("127.0.0.1:0", ip4);
         clnt.open();
         clnt.reuse().bind();
         clnt.connect();

         // wait for the server to be ready
         server->waitFor();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the TCP Socket IPV4 works, using datagram
   TEST_F(SocketTest, TCPSocket4DGM) {
      char buf[1024];

      // Connected datagram with filter
      try {
         TCP4Socket sk(ip4);
         TCPServerDG* server = new TCPServerDG(sk);

         // Create the client IPv4 socket, and connect to the server
         TCP4Socket clnt(ip4);
         clnt.open();
         clnt.reuse().bind(true);

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the TCP Socket IPV4 works, using stream read/write
   TEST_F(SocketTest, TCPSocket4) {
      char buf[1024];

      try {
         TCP4Socket sk(ip4);
         TCPServer* server = new TCPServer(sk);

         // Create the client IPv4 socket, and connect to the server
         TCP4Socket clnt(ip4);
         clnt.open();
         clnt.bind(true);

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         int size = clnt.read(buf, 1023, true);
         ASSERT_EQ(size, 6);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "got it");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }


   // ###########################################################################################
   // IPV6 Tests
   // ###########################################################################################

   // Tests that the UDP Socket IPV6 works, using datagram
   TEST_F(SocketTest, UDPSocket6) {
      char buf[1024];

      // Standard datagram IP6
      try {
         UDP6Socket sk(ip6);
         UDPServer* server = new UDPServer(sk);

         // Create the client IPv4 socket, and connect to the server
         UDP6Socket clnt(SockAddrIn6::ANY, ip6);
         clnt.open().bind(true);

         // wait for the server to be ready
         server->waitFor();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the UDP Socket IPV6 works, using connected datagram
   TEST_F(SocketTest, UDPSocket6Connected) {
      char buf[1024];

      // Connected datagram with filter
      try {
         UDP6Socket sk(ip6);
         UDPServer* server = new UDPServer(sk);

         ASSERT_FALSE(ip6.isAddrAny());

         // Create the client IPv6 socket, and connect to the server
         UDP6Socket clnt("::1:10001", ip6);
         clnt.open();
         clnt.reuse().bind();
         clnt.connect();

         // wait for the server to be ready
         server->waitFor();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the TCP Socket IPV6 works, using datagram
   TEST_F(SocketTest, TCPSocket6DGM) {
      char buf[1024];

      // Connected datagram with filter
      try {
         TCP6Socket sk(ip6);
         TCPServerDG* server = new TCPServerDG(sk);

         // Create the client IPv4 socket, and connect to the server
         TCP6Socket clnt(ip6);
         clnt.open();
         clnt.bind(true);

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         clnt.send("01234567890123456789", 20);
         buf[0] = '\0';
         int size = clnt.recv(buf, 20);
         ASSERT_EQ(size, 20);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "01234567890123456789");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the TCP Socket IPV6 works, using stream read/write
   TEST_F(SocketTest, TCPSocket6) {
      char buf[1024];

      try {
         // Create the client IPv4 socket, and connect to the server
         TCP6Socket sk(ip6);
         TCPServer* server = new TCPServer(sk);

         // Create the client IPv4 socket, and connect to the server
         TCP6Socket clnt(ip6);
         clnt.open();
         clnt.bind(true);

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         int size = clnt.read(buf, 1023, true);
         ASSERT_EQ(size, 6);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "got it");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }
   }

   // Tests that the TCP Socket IPV6 works with get socket, using stream read/write
   TEST_F(SocketTest, TCPSocket6GetSocket) {
      char buf[1024];

      try {
         // Create the client IPv4 socket, and connect to the server
         TCPSocket *sk = TCPSocket::getSocket("::1", 11000);
         TCPServer* server = new TCPServer(*sk);

         // Create the client IPv4 socket, and connect to the server
         TCP6Socket clnt(ip6);
         clnt.open();
         clnt.bind(true);

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         int size = clnt.read(buf, 1023, true);
         ASSERT_EQ(size, 6);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "got it");

         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         ASSERT_TRUE(false);
      }
   }
  
   // ###########################################################################################
   // Unix Socket Tests
   // ###########################################################################################

   // Tests that the Unix Socket works with get socket, using stream read/write
   TEST_F(SocketTest, UnixSocketRead) {
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
         ssize_t size = ck.read(buf, 1023, true);
         ASSERT_EQ(size, 6);

         buf[size] = '\0';
         ASSERT_STREQ(buf, "got it");

         // Clean up the client
         ck.close();
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         ASSERT_TRUE(false);
      }
   }
  
   // Tests that the Unix Socket works with get socket, using stream read/write
   TEST_F(SocketTest, UnixSocketLoop) {
      char buf[1024];

      try {
         // Create the client IPv4 socket, and connect to the server
         SockAddrUnix srvr("/tmp/usock/svr/server_socket");
         SockAddrUnix clnt("/tmp/usock/usr/client_socket");

         UnixSocket* sk = UnixSocket::getSocket(srvr);
         UnixServer* server = new UnixServer(*sk);

         // wait for the server to be ready
         server->waitFor();

         for (int i=0; i<20; i++) {
            // Create the client IPv4 socket, and connect to the server
            UnixSocket ck(clnt,srvr);
            ck.mkpath();
            ck.open();
            ck.bind();
            ck.connect();

            // Read and validate the response
            ssize_t size = ck.read(buf, 1023, true);
            ASSERT_EQ(size, 6);

            buf[size] = '\0';
            ASSERT_STREQ(buf, "got it");
         }

         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         ASSERT_TRUE(false);
      }
   }
  
#if 0
   /* This test was used to verify proper addresses were coming out of the 
    * sockets after cloning/accept. To run this test, ::1001 needs to be 
    * assigned to your box, thus the reason it is disabled. 
    *
    * ip addr add ::1001/128 dev lo
    *
    * This is included as a reference.
    */
   TEST_F(SocketTest, TCPSocket6AcceptSocketNonStandard) {
      char buf[1024];
      
      /* This goes in TCPServer to print out the socket address to visually
       * verify they are correct. Quick and dirty....
       * printf("POST ACCEPT===\n");
       * client->getLocalAddress().toString(tmp1);
       * printf("local: %s\n", tmp1.c_str());
       * client->getPeerAddress().toString(tmp1);
       * printf("peer: %s\n", tmp1.c_str());
       * tsk.getLocalAddress().toString(tmp1);
       * printf("tsk local: %s\n", tmp1.c_str());
       * tsk.getPeerAddress().toString(tmp1);
       * printf("tsk peer: %s\n", tmp1.c_str()); 
      */


      try {
         // Create the client IPv4 socket, and connect to the server
         TCPSocket *sk = TCPSocket::getSocket("::1001", 11000);
         SockAddrIn6 ip6socket("::1001.11000");

         TCPServer* server = new TCPServer(*sk);

         // Create the client IPv4 socket, and connect to the server
         TCP6Socket clnt(ip6socket);
         clnt.open();
         clnt.bind();

         // wait for the server to be ready
         server->waitFor();
         clnt.connect();

         // Read and validate the response
         int size = clnt.read(buf, 1023, true);
         buf[size] = '\0';

         ASSERT_STREQ(buf, "got it");
         ASSERT_EQ(size, 6);
         //
         // Clean up the server
         delete server;
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
         ASSERT_TRUE(false);
      }
   }
#endif
}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
