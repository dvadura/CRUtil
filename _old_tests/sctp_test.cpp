#include "crunnable.h"
#include "log.h"

using namespace std;
using namespace crunnable;

extern "C" int main(int argc, char** argv) {
  Log* glog = new Log("foo", 0, 9 , false);
  sleep(5);
  delete glog;
  return 0;
}
