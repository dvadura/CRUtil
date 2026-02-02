#include "crunnable.h"
#include "clist.h"
#include "crpool.h"
#include "socket.h"

using namespace std;
using namespace crunnable;

class Acceptor;

class Server : public CRunnable, public Condition {
   friend class Acceptor;

   private:
   TCPSocket &m_sock;
   size_t m_size;
   FD  m_null;

   public:
   Server(TCPSocket &sock) : m_sock(sock) {
      start(true);
   }

   ~Server() {
      waitStop(NS_IN_ONE_MSEC);
   }

   TCPSocket* accept() {return m_sock.accept();}

   void service() {
      TCPSocket*  client = NULL;
      uint64_t    size = 0;
      long        len = 0;
      Pipe        pipe;

      if (isTerminated() == false) {
         client = m_srv.accept();
      }

      if (client == NULL) {
         return;
      }

      client.read(&size, sizeof(size));

      do {
         long res = splice(m_null,NULL,pipe.fdin(),NULL,size-len,SPLICE_F_NONBLOCK|SPLICE_F_MOVE);

         if (res > 0) {
            len += res;
         }
         else if (res < 0) {
            fprintf(stderr, "Splice error %d\n");
         }
      }
      while (len != size);

      do {
         long res = splice(pipe.fdout(),NULL,m_Sock.fd(),NULL,len,SPLICE_F_NONBLOCK|SPLICE_F_MOVE);

         if (res > 0) {
            len -= res;
         }
         else if (res < 0) {
            fprintf(stderr, "Splice error %d\n");
         }
      }
      while (len > 0);
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

class Acceptor : public CRunnable {
   private:
   Server& m_srv;

   public:
   Acceptor(Server srv) : m_srv(srv) = default;
   virtual ~Acceptor() = default;

   virtual int run() {
      while (isTerminated() == false) {
         m_srv.service();
      }
   };
}

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
   Client *clients[65536];
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
      for (int i=0; i<50000; ++i) {
         clients[i] = new Client(new TCP4Socket(SockAddrIn4::ANY, lip4));
      }
      num=50000;
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
