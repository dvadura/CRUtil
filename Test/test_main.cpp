#define CATCH_CONFIG_MAIN
#include "catch2.hpp"
#include "semaphore.h"
#include "condition.h"
#include "crexception.h"

// Static definitions now live in their respective .cpp files:
// - crexception.cpp: CRException::s_tmap, s_tlock
// - semaphore.cpp: Semaphore::VERBTAG
// - condition.cpp: Condition::CONDKEY, CONDKEY_INIT, SEMCONDKEY
