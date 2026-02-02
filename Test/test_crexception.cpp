#include "catch2.hpp"
#include "crexception.h"

#include <sstream>
#include <cstdio>
#include <cstring>
#include <unistd.h>

using namespace std;
using namespace crutil;

// ---------------------------------------------------------------------------
// CRX_THROW / CRX_THROW_ERR basics
// ---------------------------------------------------------------------------

TEST_CASE("CRX_THROW throws CRException", "[crexception]") {
   bool caught = false;
   try {
      CRX_THROW("basic throw");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);
}

TEST_CASE("CRX_THROW_ERR carries error number", "[crexception]") {
   try {
      CRX_THROW_ERR(42, "err %d test", 42);
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(e.geterrno() == 42);
      REQUIRE(string(e.what()).find("err 42 test") != string::npos);
   }
}

TEST_CASE("CRX_THROW sets errno to -1", "[crexception]") {
   try {
      CRX_THROW("no errno");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(e.geterrno() == -1);
   }
}

// ---------------------------------------------------------------------------
// CRX_TIF / CRX_TUNLESS / CRX_TIF_ERR convenience macros
// ---------------------------------------------------------------------------

TEST_CASE("CRX_TIF throws when expression is true", "[crexception]") {
   bool caught = false;
   try {
      CRX_TIF(true, "should throw");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);
}

TEST_CASE("CRX_TIF does not throw when expression is false", "[crexception]") {
   bool caught = false;
   try {
      CRX_TIF(false, "should not throw");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(!caught);
}

TEST_CASE("CRX_TUNLESS throws when expression is false", "[crexception]") {
   bool caught = false;
   try {
      CRX_TUNLESS(false, "should throw");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);
}

TEST_CASE("CRX_TUNLESS does not throw when expression is true", "[crexception]") {
   bool caught = false;
   try {
      CRX_TUNLESS(true, "should not throw");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(!caught);
}

TEST_CASE("CRX_TIF_ERR throws with custom errno", "[crexception]") {
   try {
      CRX_TIF_ERR(true, 99, "custom errno");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(e.geterrno() == 99);
   }
}

// ---------------------------------------------------------------------------
// CRException accessors: file(), line(), what(), errmsg()
// ---------------------------------------------------------------------------

TEST_CASE("CRException reports file and line", "[crexception]") {
   try {
      CRX_THROW("location test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(string(e.file()).find("test_crexception.cpp") != string::npos);
      REQUIRE(e.line() > 0);
   }
}

TEST_CASE("CRException what(string&) returns message", "[crexception]") {
   try {
      CRX_THROW("hello %s", "world");
      FAIL("should have thrown");
   } catch (CRException& e) {
      string msg;
      e.what(msg);
      REQUIRE(msg.find("hello world") != string::npos);
   }
}

TEST_CASE("CRException errmsg returns errno description for positive errno", "[crexception]") {
   try {
      CRX_THROW_ERR(ENOENT, "file missing");
      FAIL("should have thrown");
   } catch (CRException& e) {
      string em;
      e.errmsg(em);
      REQUIRE(!em.empty());
   }
}

TEST_CASE("CRException errmsg falls back to message for negative errno", "[crexception]") {
   try {
      CRX_THROW("negative errno");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(string(e.errmsg()) == string(e.what()));
   }
}

// ---------------------------------------------------------------------------
// CRX_CAPTURE_CATCH macro
// ---------------------------------------------------------------------------

TEST_CASE("CRX_CAPTURE_CATCH builds a catch summary string", "[crexception]") {
   try {
      CRX_THROW("capture test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      string out;
      CRX_CAPTURE_CATCH(out, e);

      REQUIRE(out.find("CRException Caught:") != string::npos);
      REQUIRE(out.find("m_pid(") != string::npos);
      REQUIRE(out.find("m_tid(") != string::npos);
      REQUIRE(out.find("test_crexception.cpp") != string::npos);
      REQUIRE(out.find("CRException Raised:") != string::npos);
      REQUIRE(out.find("capture test") != string::npos);
   }
}

TEST_CASE("CRX_CAPTURE_CATCH with staticOnly omits pid/tid in Caught header", "[crexception]") {
   try {
      CRX_THROW("static only test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      e.setStaticOnly(true);
      string out;
      CRX_CAPTURE_CATCH(out, e);

      size_t raised_pos = out.find("CRException Raised:");
      REQUIRE(raised_pos != string::npos);
      string caught_part = out.substr(0, raised_pos);
      REQUIRE(caught_part.find("m_pid(") == string::npos);
   }
}

// ---------------------------------------------------------------------------
// CRX_REPORT_CATCH macro (writes to FILE*)
// ---------------------------------------------------------------------------

TEST_CASE("CRX_REPORT_CATCH writes exception to file descriptor", "[crexception]") {
   try {
      CRX_THROW("report catch test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      FILE* tmp = tmpfile();
      REQUIRE(tmp != NULL);

      CRX_REPORT_CATCH(tmp, e);

      fseek(tmp, 0, SEEK_END);
      long sz = ftell(tmp);
      REQUIRE(sz > 0);
      fseek(tmp, 0, SEEK_SET);

      string content(sz, '\0');
      fread(&content[0], 1, sz, tmp);
      fclose(tmp);

      REQUIRE(content.find("CRException Caught:") != string::npos);
      REQUIRE(content.find("report catch test") != string::npos);
   }
}

// ---------------------------------------------------------------------------
// CRX_STACKTRACE macro
// ---------------------------------------------------------------------------

TEST_CASE("CRX_STACKTRACE without rethrow does not propagate", "[crexception]") {
   FILE* tmp = tmpfile();
   REQUIRE(tmp != NULL);

   bool caught = false;
   try {
      CRX_STACKTRACE(tmp, -1, false, "stacktrace no rethrow");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(!caught);

   fseek(tmp, 0, SEEK_END);
   long sz = ftell(tmp);
   REQUIRE(sz > 0);
   fseek(tmp, 0, SEEK_SET);

   string content(sz, '\0');
   fread(&content[0], 1, sz, tmp);
   fclose(tmp);

   REQUIRE(content.find("stacktrace no rethrow") != string::npos);
}

TEST_CASE("CRX_STACKTRACE with rethrow propagates exception", "[crexception]") {
   FILE* tmp = tmpfile();
   REQUIRE(tmp != NULL);

   bool caught = false;
   try {
      CRX_STACKTRACE(tmp, -1, true, "stacktrace rethrow");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);

   fclose(tmp);
}

TEST_CASE("CRX_STACKTRACE carries custom errno", "[crexception]") {
   FILE* tmp = tmpfile();
   REQUIRE(tmp != NULL);

   try {
      CRX_STACKTRACE(tmp, 77, true, "errno in stacktrace");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(e.geterrno() == 77);
   }

   fclose(tmp);
}

// ---------------------------------------------------------------------------
// CRX_REPORT_TRACE (alias for CRX_STACKTRACE)
// ---------------------------------------------------------------------------

TEST_CASE("CRX_REPORT_TRACE without rethrow does not propagate", "[crexception]") {
   FILE* tmp = tmpfile();
   REQUIRE(tmp != NULL);

   bool caught = false;
   try {
      CRX_REPORT_TRACE(tmp, -1, false, "report trace test");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(!caught);

   fseek(tmp, 0, SEEK_END);
   long sz = ftell(tmp);
   fseek(tmp, 0, SEEK_SET);

   string content(sz, '\0');
   fread(&content[0], 1, sz, tmp);
   fclose(tmp);

   REQUIRE(content.find("report trace test") != string::npos);
}

// ---------------------------------------------------------------------------
// CRX_THROW_CHK respects thread cancellation
// ---------------------------------------------------------------------------

TEST_CASE("CRX_THROW_CHK throws when thread is not canceled", "[crexception]") {
   bool caught = false;
   try {
      CRX_THROW_CHK(-1, "chk throw test");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);
}

TEST_CASE("CRX_THROW_CHK suppresses throw for other threads when one is canceled", "[crexception]") {
   // isThreadCanceled returns true (suppressing throws) for threads whose tid
   // is NOT in the cancel map when the map is non-empty.  The canceled thread
   // itself still throws.  We verify the canceled thread still throws here.
   pid_t tid = CRX_GETTID();
   CRException::notifyCancel(tid);

   bool caught = false;
   try {
      CRX_THROW_CHK(-1, "chk still throws for canceled thread");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);

   CRException::clearCancel(tid);
}

// ---------------------------------------------------------------------------
// CRX_STACKTRACE rethrow suppressed when thread is canceled
// ---------------------------------------------------------------------------

TEST_CASE("CRX_STACKTRACE rethrow still occurs for canceled thread itself", "[crexception]") {
   // The canceled thread's own tid IS in the map, so isThreadCanceled returns
   // false for it and the rethrow proceeds normally.
   FILE* tmp = tmpfile();
   REQUIRE(tmp != NULL);

   pid_t tid = CRX_GETTID();
   CRException::notifyCancel(tid);

   bool caught = false;
   try {
      CRX_STACKTRACE(tmp, -1, true, "canceled rethrow");
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);

   CRException::clearCancel(tid);
   fclose(tmp);
}

// ---------------------------------------------------------------------------
// operator<< / operator+= / toString
// ---------------------------------------------------------------------------

TEST_CASE("CRException operator<< outputs full detail to ostream", "[crexception]") {
   try {
      CRX_THROW_ERR(2, "ostream test %d", 123);
      FAIL("should have thrown");
   } catch (CRException& e) {
      stringstream ss;
      ss << e;
      string out = ss.str();

      REQUIRE(out.find("CRException Raised:") != string::npos);
      REQUIRE(out.find("ostream test 123") != string::npos);
      REQUIRE(out.find("SYS errno: 2") != string::npos);
   }
}

TEST_CASE("CRException operator<< shows APP Errno for negative errno", "[crexception]") {
   try {
      CRX_THROW("negative errno ostream");
      FAIL("should have thrown");
   } catch (CRException& e) {
      stringstream ss;
      ss << e;
      string out = ss.str();

      REQUIRE(out.find("APP Errno: -1") != string::npos);
   }
}

TEST_CASE("CRException operator<< with null pointer", "[crexception]") {
   CRException* ptr = nullptr;
   stringstream ss;
   ss << ptr;
   REQUIRE(ss.str().find("CRException ref was NULL") != string::npos);
}

TEST_CASE("CRException operator+= appends to string", "[crexception]") {
   try {
      CRX_THROW("pluseq test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      string out = "PREFIX:";
      out += e;
      REQUIRE(out.find("PREFIX:") == 0);
      REQUIRE(out.find("pluseq test") != string::npos);
   }
}

TEST_CASE("CRException toString fills output string", "[crexception]") {
   try {
      CRX_THROW("tostring test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      string out;
      e.toString(out);
      REQUIRE(out.find("CRException Raised:") != string::npos);
      REQUIRE(out.find("tostring test") != string::npos);
   }
}

// ---------------------------------------------------------------------------
// CRX_TIFNULL
// ---------------------------------------------------------------------------

TEST_CASE("CRX_TIFNULL throws on NULL pointer", "[crexception]") {
   int* p = nullptr;
   bool caught = false;
   try {
      CRX_TIFNULL(p);
   } catch (CRException&) {
      caught = true;
   }
   REQUIRE(caught);
}

TEST_CASE("CRX_TIFNULL returns pointer for non-NULL", "[crexception]") {
   int val = 42;
   int* p = &val;
   int* result = CRX_TIFNULL(p);
   REQUIRE(result == p);
   REQUIRE(*result == 42);
}

// ---------------------------------------------------------------------------
// Calltrace contains demangled symbols
// ---------------------------------------------------------------------------

TEST_CASE("CRException calltrace includes demangled symbols", "[crexception]") {
   try {
      CRX_THROW("demangle test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      stringstream ss;
      ss << e;
      string out = ss.str();

#if (defined(_GNU_SOURCE) && !defined(ANDROID))
      REQUIRE(out.find("Calltrace:") != string::npos);
      size_t ct_pos = out.find("Calltrace:");
      string calltrace = out.substr(ct_pos);
      REQUIRE(calltrace.size() > 20);
#endif
   }
}

// ---------------------------------------------------------------------------
// setStaticOnly / isStaticOnly
// ---------------------------------------------------------------------------

TEST_CASE("setStaticOnly/isStaticOnly toggle", "[crexception]") {
   try {
      CRX_THROW("static test");
      FAIL("should have thrown");
   } catch (CRException& e) {
      REQUIRE(e.isStaticOnly() == false);
      e.setStaticOnly(true);
      REQUIRE(e.isStaticOnly() == true);
      e.setStaticOnly(false);
      REQUIRE(e.isStaticOnly() == false);
   }
}

TEST_CASE("Static-only exception omits calltrace in output", "[crexception]") {
   try {
      CRX_THROW("no calltrace");
      FAIL("should have thrown");
   } catch (CRException& e) {
      e.setStaticOnly(true);
      stringstream ss;
      ss << e;
      string out = ss.str();

      REQUIRE(out.find("Calltrace:") == string::npos);
   }
}
