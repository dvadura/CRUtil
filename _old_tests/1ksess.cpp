#include "crunnable.h"
#include "socket.h"

using namespace std;
using namespace crunnable;

class Server : public CRunnable, public Condition {
   TCPSocket &m_sock;

public:
   Server(TCPSocket &sock) : m_sock(sock) {
      start(true);
   }

   ~Server() {
      waitStop(NS_IN_ONE_MSEC);
   }

   virtual int run() {
      try {
         m_sock.open();
         m_sock.reuse();
         m_sock.bind();
         m_sock.listen();

         // Indicate we are ready to service requests
         raise();

         while (isTerminated() == false) {
            TCPSocket *client = m_sock.accept();

            if (client == NULL) {
               continue;
            }

            client->write("got it");
            client->shutdown();
            client->close();

            delete client;
         }

         m_sock.close();
      }
      catch (CRException& e) {
         CRX_REPORT_CATCH(stderr, e);
      }

      return 0;
   }
};

class Client {
   public:
   TCPSocket *m_sock;

   Client(TCPSocket *sock) : m_sock(sock) {}
   ~Client() {}


   void open() {
      m_sock->open();
      m_sock->bind();
      m_sock->connect();
      usleep(100);
   }
};

int 
main(int argc, char **argv) 
{
   SockAddrIn4 lip4("172.17.2.2:8000");;
   Client *clients[2048];
   int    num=0;

   if (argc > 1 && strcmp(argv[1],"-l") == 0) {
      TCP4Socket m_sock(lip4);

      m_sock.open();
      m_sock.reuse();
      m_sock.bind();
      m_sock.listen();

      try {
         while (true) {
            string str;
            TCPSocket *client = m_sock.accept();
            if (client != NULL) {
               fprintf(stderr, "received connection from %s\n", client->getQuadAddress().toString(str).c_str());
            }
         }
      }
      catch (CRException& ignore) {
         CRX_REPORT_CATCH(stderr, ignore);
      }

      m_sock.close();
      exit(0);
   }
   else if (argc > 1 && strcmp(argv[1],"-1") == 0) {
      clients[0] = new Client(new TCP4Socket(SockAddrIn4::ANY, lip4));
      num = 1;
   }
   else {
      for (int i=0; i<2000; ++i) {
         clients[i] = new Client(new TCP4Socket(SockAddrIn4::ANY, lip4));
      }
      num=2000;
   }

   try {
      for (int i=0; i < num; ++i) {
         fprintf(stderr, "opening client %d of %d\n", i, num);
         clients[i]->open();
      }
   }
   catch (CRException& ignore) {
      CRX_REPORT_CATCH(stderr, ignore);
   }

   pause();
}
