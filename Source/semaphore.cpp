/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE-APACHE-2.0.txt
 */

/**
 * @file   semaphore.cpp
 *
 * @brief  Static variable initialization for Semaphore class
 *
 * @details This file contains the static member variable initializers for the Semaphore
 *          class, including the verbose tag pointer used for debug output.
 */

#include "semaphore.h"
#include "condition.h"
#include "crtimer.h"

using namespace crutil;

// Semaphore static member initializers
// Default to nullptr - no verbosity in production builds
const char* Semaphore::VERBTAG = nullptr;
