#include <stdio.h>
#include <type_traits>
#include "needs.h"
using namespace std;

template <typename T>
struct crtp
{
   T& tcast() { return static_cast<T&>(*this); }
   T const& tcast() const { return static_cast<T const&>(*this); }
};

template <typename T>
class PItem : public crtp<T>
{
   private:
   PItem(){};
   friend T;

   public:
   typedef PItem<T>&  p_item_t;
   typedef PItem<T>* pp_item_t;

   void reset() {
      fprintf(stderr, "pitem reset\n   ");
      this->tcast().reset();
   }

   PItem<T>* alloc() {
      PItem<T>* item = this->tcast().alloc();
      fprintf(stderr, "pitem alloc\n   ");
      item->reset();
      return item;
   }
};

//class Pooled : public IPoolItem<CRPool, Pooled> {
class Pooled : public PItem<Pooled> {
   public:
   virtual void reset() = 0;
   virtual PItem<Pooled>* alloc() = 0;
};

class Session : public Pooled {
   public:
   virtual void reset() {
      fprintf(stderr, "reset Session\n");
   }

   virtual PItem<Pooled>* alloc() {
      return new Session();
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

   virtual PItem<Pooled>* alloc() {
      return NULL;
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

   virtual PItem<Pooled>* alloc() {
      return new CachedSession();
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

   virtual PItem<Pooled>* alloc() {
      return new SplicedSession();
   }

   virtual void dosomething() {
      fprintf(stderr, "DOIT SplicedSession\n\n");
   }
};

// in order for T to be a pooled item, it must define T::p_item_t and T::pp_item_t, which
// are provided by PItem template. So, to be poolable just inherit in your base from Pooled
// and implement the required virtual functions.
#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

template<class T>// REQUIRES(std::is_base_of<Pooled, T>())>
class Pool {
   static_assert(std::is_base_of<Pooled, T>(), "CRPool: pooled items must derive from base class Pooled");

   typedef typename T::p_item_t  rp_item_t;
   typedef typename T::pp_item_t pp_item_t;

   private:
   T type;

   inline T* tcast(pp_item_t item) {
      return static_cast<T*>(item);
   }

   pp_item_t alloc() {
      return static_cast<rp_item_t>(type).alloc();
   }

   public:
   Pool() = default;
   virtual ~Pool() = default;

   T* reserve() {
      T* res = tcast(alloc());
      res->reset();
      return res;
   }
};

template<class T, NEEDS(std::is_base_of<Pooled, T>())>
Pool<T>*
allocPool(const char* pname, size_t min, size_t max, size_t target)
{
   Pool<T>* result = new Pool<T>();
   return result;
}


int main(int argc, char **argv) {
   Session pl2;
   CachedSession pl3;

   PItem<Pooled>& x2 = pl2;
   PItem<Pooled>& x3 = pl3;

   pl2.reset();

   fprintf(stderr, "---------\n");

   x2.reset();
   x3.reset();

   fprintf(stderr, "---------\n");

   x2.alloc();
   x3.alloc();

   fprintf(stderr, "1 ---------\n");

   Pool<Session> p2;
   Pool<CachedSession> p3;

   p2.reserve()->dosomething();
   p3.reserve()->dosomething();

   Session* s;
   fprintf(stderr, "2 ---------\n");
   s = p2.reserve();
   s->dosomething();

   fprintf(stderr, "3 ---------\n");
   s = p3.reserve();
   s->dosomething();

   fprintf(stderr, "4 ---------\n");
   Pool<Session>* ps = allocPool<Session>("first", 0, 0, 0);
   s = ps->reserve();
   s->dosomething();

   fprintf(stderr, "5 ---------\n");
   Pool<CachedSession>* cs = allocPool<CachedSession>("first", 0, 0, 0);
   s = cs->reserve();
   s->dosomething();

   fprintf(stderr, "6 ---------\n");
   Pool<SplicedSession>* ss = allocPool<SplicedSession>("first", 0, 0, 0);
   s = ss->reserve();
   s->dosomething();
}
