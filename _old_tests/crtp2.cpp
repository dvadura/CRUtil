#include <stdio.h>
#include <type_traits>
#include "needs.h"
using namespace std;

class Pool;

class Pooled {
   protected:
   Pool* pool;

   public:
   virtual void reset() = 0;

   virtual void dosomething() { fprintf(stderr, "DO SOMETHING IN BASE\n\n"); }
   void setpool(Pool* p) {pool = p;}
};

extern "C" {
   typedef Pooled* (*newitem_fn_t)(Pool* pool);
}

template <class I, REQUIRES(std::is_base_of<Pooled, I>())>
inline Pooled* newitem(Pool* pool) {
   fprintf(stderr, "generic alloc\n   ");
   Pooled* res = static_cast<Pooled*>(new I);
   res->setpool(pool);
   res->reset();
   return res;
}

template <class I, REQUIRES(std::is_base_of<Pooled, I>())>
inline I* tcast(Pooled* item) {
   return reinterpret_cast<I*>(item);
}

class Pool {
   private:
   newitem_fn_t newitem;

   public:
   Pool(newitem_fn_t fn) : newitem(fn) {}
   virtual ~Pool() = default;

   Pooled* reserve() {
      Pooled* res = newitem(this);
      return res;
   }
};

// -------------------------------------------------------------------------------- 
// -------------------------------------------------------------------------------- 

class Session : public Pooled {
   public:
   virtual void reset() {
      fprintf(stderr, "reset Session\n");
   }

   virtual void dosomething() {
      fprintf(stderr, "DOIT Session\n\n");
   }
};

class XSession {
   public:
   virtual void reset() {
      fprintf(stderr, "reset XSession\n");
   }

   virtual void dosomething() {
      fprintf(stderr, "DOIT XSession\n\n");
   }
};

class CachedSession : public Session {
   public:
   virtual void reset() {
      fprintf(stderr, "reset CachedSession\n");
   }

   virtual void dosomething() {
      fprintf(stderr, "DOIT CachedSession\n\n");
   }
};

class SplicedSession : public CachedSession {
   public:
   virtual void reset() {
      fprintf(stderr, "reset SplicedSession\n");
   }

   virtual void dosomething() {
      fprintf(stderr, "DOIT SplicedSession\n\n");
   }
};

int main(int argc, char **argv) {
   Session pl2;
   CachedSession pl3;

   Pooled& x2 = pl2;
   Pooled& x3 = pl3;

   pl2.reset();

   fprintf(stderr, "---------\n");

   x2.reset();
   x3.reset();

   fprintf(stderr, "1 ---------\n");

   Pool p2(newitem<Session>);
   Pool p3(newitem<CachedSession>);

   p2.reserve()->dosomething();
   p3.reserve()->dosomething();

   Session* s;
   fprintf(stderr, "2 ---------\n");
   //s = static_cast<Session*>(p2.reserve());
   s = tcast<Session>(p2.reserve());
   s->dosomething();

   fprintf(stderr, "3 ---------\n");
   Pooled* p;
   p = p3.reserve();
   p->dosomething();
}
