#include <stdio.h>
using namespace std;

template <typename T>
struct crtp
{
   T& tcast() { return static_cast<T&>(*this); }
   T const& tcast() const { return static_cast<T const&>(*this); }
};

