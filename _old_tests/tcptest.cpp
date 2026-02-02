#include "crunnable.h"
#include "socket.h"
#include "crtimer.h"
#include "signal.h"
#include <string>

#define IPADDR "172.17.2.2"
//#define IPADDR "127.0.0.1"
#define PORT 10000

using namespace std;
using namespace crunnable;

class Server {
   public:
   TCPSocket& sock;
   uint64_t delay;

   Server(TCPSocket& s, uint64_t d) : sock(s), delay(d) {
      fprintf(stderr, "server created\n");
      if (sock.getLocalAddress().isAny() == true) {
         fprintf(stderr, "swapped addrs\n");
         sock.swap();
      }
   }

   int run() {
      CRTime now;

      try {
         sock.open();
         sock.bind();
         sock.listen();
         fprintf(stderr, "socket open, bound and listening\n\n");

         while (true) {
            char buf[1024];
            char mode;
            int  nn=0;
            fprintf(stderr, "before accept\n");
            TCPSocket* client = sock.accept();

            fprintf(stderr, "accepted one, reading\n");

            // we wait to make sure client request is fully received, before attempting to
            // read in non-blocking mode
            now.msleep(5);
            client->block(false);
            int n = client->read(buf, 60, true);
            client->block(true);

            buf[n] = '\0';
            mode  = buf[0];
            if (mode == 'w') {
               fprintf(stderr, "doing pause read\n");
               nn = client->read(buf+n, 10, true);
            }

            delay = (uint64_t) atol(buf+2);
            fprintf(stderr, "got request: %s, 2nd read returns %d, mode=%c delay=%lu\n", buf, nn, mode, delay);

            if (delay > 0) {
               now.msleep(delay);
            }

            try {
               const char *msg = "take me to your leader take me to your leader take me to your leader!";
               int res = client->write(msg, strlen(msg), false);
               client->shutdown(SHUT_WR);
               fprintf(stderr, "wrote response, called shutdown and sleeping for 60s before close, res=%d\n\n", res);
               now.msleep(60*1000);
               fprintf(stderr, "first timeout expired 60s, res=%d\n\n", res);
               now.msleep(60*1000);
               client->close();
            }
            catch (CRException& we) {
               string msg;
               fprintf(stderr, "Server::run(): - write: %s\n", we.toString(msg).c_str());
            }
            catch (exception& ex) {
               fprintf(stderr, "Server::run(): - runtime_error: %s\n", ex.what());
            }
         }
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Server::run(): - exception: %s\n", e.toString(msg).c_str());
      }

      return 0;
   };
};

class Client {
   public:
   TCPSocket& sock;
   uint64_t delay;

   Client(TCPSocket& s, uint64_t d) : sock(s), delay(d) {
      fprintf(stderr, "client created\n");
   };

   bool connect() {
      try {
         sock.open();
         sock.block();
         sock.bind(true);
         sock.connect();
         fprintf(stderr, "connected, writing request, %d\n", sock.isBlocking());
         return true;
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::connect() - exception: %s\n", e.toString(msg).c_str());
      }

      return false;
   }

   char* test_shut_before_read(char* buf) {
      int n = 0;

      if (connect() == false) {
         buf[0] = '\0';
         return(buf);
      }

      sprintf(buf, "%c %lu hello world", 'w', delay);

      try {
         sock.write(buf, strlen(buf), true);
         sock.shutdown(SHUT_WR);
         n = sock.read(buf, 1024, true);
         sock.close();
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::SbeforeR() - exception: %s\n", e.toString(msg).c_str());
      }

      buf[n] = '\0';
      return buf;
   };

   char* test_shut_after_read(char* buf) {
      int n = 0;

      if (connect() == false) {
         buf[0] = '\0';
         return(buf);
      }

      sprintf(buf, "%c %lu hello world", '-', delay);

      try {
         sock.write(buf, strlen(buf), true);
         n = sock.read(buf, 1024, true);
         sock.shutdown(SHUT_WR);
         sock.close();
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::SafterR() - exception: %s\n", e.toString(msg).c_str());
      }

      buf[n] = '\0';
      return buf;
   };

   char* test_just_read(char* buf) {
      int n = 0;

      if (connect() == false) {
         buf[0] = '\0';
         return(buf);
      }

      sprintf(buf, "%c %lu hello world", '-', delay);

      try {
         sock.write(buf, strlen(buf), true);
         n = sock.read(buf, 1024, true);
         sock.close();
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::JRead() - exception: %s\n", e.toString(msg).c_str());
      }

      buf[n] = '\0';
      return buf;
   };

   char* test_no_read(char* buf) {
      int n = 0;

      if (connect() == false) {
         buf[0] = '\0';
         return(buf);
      }

      sprintf(buf, "%c %lu hello world", '-', delay);

      try {
         sock.write(buf, strlen(buf), true);

         if (delay > 0) {
            CRTime now;
            now.msleep(delay);
         }

         sock.close();
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::JRead() - exception: %s\n", e.toString(msg).c_str());
      }

      buf[n] = '\0';
      return buf;
   };

   char* test_and_sleep(char* buf) {
      int n = 0;

      if (connect() == false) {
         buf[0] = '\0';
         return(buf);
      }

      sprintf(buf, "%c %lu hello world", '-', 0);

      try {
         sock.write(buf, strlen(buf), true);

         if (delay > 0) {
            CRTime now;
            now.msleep(delay);
         }
      }
      catch (CRException& e) {
         string msg;
         fprintf(stderr, "Client::JRead() - exception: %s\n", e.toString(msg).c_str());
      }

      buf[n] = '\0';
      return buf;
   };
};

int 
main(int argc, char** argv) 
{
   bool server = false;
   uint64_t delay = 0L;

   if (argc > 1 && strncmp("-s",argv[1],2) == 0) {
      server = true;
      ++argv;
      --argc;
   }

   if (argc > 1 && strchr(argv[1],'.') == NULL) {
      delay = (uint64_t) atol(argv[1]);
      ++argv;
      --argc;
   }

   SockAddrIn4 ip4((argc > 1) ? argv[argc-1] : IPADDR, PORT);
   TCP4Socket  sk(ip4);

   string s;
   fprintf(stderr, "server ip: %s, running as %s, with delay = %lu\n", ip4.toString(s).c_str(), (server ? "server" : "client"), delay);

   signal(SIGPIPE, SIG_IGN);
   if (server == true) {
      Server s(sk, delay);
      s.run();
      exit(0);
   }
      
   Client c(sk, delay);
   char buf[512];
   char *res;

   res = c.test_and_sleep(buf);
   fprintf(stderr, "got response (%lu): %s\n", strlen(res), res);

   res = c.test_shut_before_read(buf);
   fprintf(stderr, "got response (%lu): %s\n", strlen(res), res);

   res = c.test_just_read(buf);
   fprintf(stderr, "got response (%lu): %s\n", strlen(res), res);

   res = c.test_shut_after_read(buf);
   fprintf(stderr, "got response (%lu): %s\n", strlen(res), res);

   return 0;
}
