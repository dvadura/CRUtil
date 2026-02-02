#include "crunnable.h"
#include "crtimer.h"
#include "crpool.h"

// Model:
//
// CLNT(NetNode . NetLink) -> (NetNode . NetLink) -> SRV
//  SRV(NetNode . NetLink) -> (NetNode . NetLink) -> CLNT
//
// So each Node has a rcv, and xmit abstraction.
// There are 2 types of nodes, NetNode, and NetLink.
//
// An endpoint is a NetNode and is either a Client or Server, 
// and can send or receive data via the xmit/rcv methods.
//
// Furthermore and EndNode also has a CC mechanism that is used
// to detect and manage congestion.
//

using namespace std;
using namespace crunnable;

struct Packet {
   bool     p_ack;
   uint64_t p_len;
   uint64_t p_rwin;
   uint64_t t_start;
   uint64_t t_end;

   Packet(uint64_t now, uint64_t len=40, bool ack=false) {
      t_start = now;
      t_end = now;
      p_len = len;
      p_ack = ack;
   }

   virtual ~Packet() = default;

   void reset() {
      t_start = t_end = p_len = p_rwin = 0;
      p_ack = false;
   }

   void set_rwin(uint64_t value) {
      p_rwin = value;
   }

   void ack() {
      return p_ack;
   }

   void set_ack() {
      p_ack = true;
   }

   uint64_t elapsed() {
      return t_end-t_start;
   }

   void delay(uint64_t amount) {
      t_end += amount;
   }
};


struct NetLink {
   uint64_t n_latency;
   uint64_t j_max;
   uint64_t j_pct;
   uint64_t j_pct_prev;
   uint64_t j_prev;

   NetLink(uint64_t latency, uint64_t max, uint64_t pct, uint64_t pct_prev) {
      n_latency = latency;
      j_max = max;
      j_pct = pct;
      j_pct_prev = pct_prev;
   }

   virtual ~NetLink() = default;

   uint64_t delay() {
      return n_latency;
   }

   uint64_t jitter() {
      return j_max;
   }
};


struct NetNode {
   CList<NetLink*>   upstream;
   CList<NetLink*>   downstream;

   NetNode() {};
   virtual ~NetNode() = default;

   void add_up(NetLink* el) {
      upstream.push_back(el);
   }

   void add_down(NetLink* el) {
      downstream.push_back(el);
   }

   void xmit_up(Packet *pkt) {
   }

   void xmit_down(Packet *pkt) {
   }
};


struct CCModel {
   uint64_t iwin;
   uint64_t rwin;
   uint64_t swin;
   uint64_t rwin_max;
   uint64_t swin_max;
   uint64_t rto_thresh;
   uint64_t rto_count;
};


struct CubicCC : public CCModel {
};


struct BBRCC : public CCModel {
};


struct WTCP4CC : public CCModel {
};


struct WTCP5CC : public CCModel {
};


enum TCPState {
   SYN=0,
   SYN_ACK,
   ESTABLISHED,
   FIN,
   FIN_ACK
};


struct TCP {
   CCModel& cc;
   TCPState state;
}


struct Test {
   Network  net;
   TCP      tcp_client;
   TCP      tcp_server;

   void configure() {
   }

   void trial(uint64_t length, uint64_t mtu) {
   }
}


int main(int argc, char* argv[]) {
}
