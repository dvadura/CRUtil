/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE-APACHE-2.0.txt
 */

/**
 * @file   condition.cpp
 *
 * @brief  Static variable initialization for Condition class
 *
 * @details This file contains the static member variable initializers for the Condition
 *          class, including the pthread thread-specific data key (CONDKEY) and the
 *          semaphore protecting its initialization (SEMCONDKEY).
 */

#include "condition.h"

using namespace crutil;

// Condition static member initializers for thread-specific data management
pthread_key_t Condition::CONDKEY;
Semaphore     Condition::SEMCONDKEY(false, false);
bool          Condition::CONDKEY_INIT = false;
