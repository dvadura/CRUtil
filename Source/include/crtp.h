/*
 * Copyright (c) 2010-2026 by Dennis Vadura, All rights reserved.
 * Licensed under terms in <distribution-root>/LICENSE.txt
 */

/** @template  crtp<t>
 *
 * @brief   template helper for CRTP
 *
 * @details Implements CRTP helpers to simplify the static_cast<> casts used
 *          by CRTP pattern.
 *
 * @author  Dennis Vadura, mailto:dennis.vadura@gmail.com
 * @see     https://github.com/dvadura/CRUtil
 */

#ifndef __CRTP_INC__
#define __CRTP_INC__

namespace crunnable {
   template <typename T>
   struct crtp {
      inline T  const& tcast() const { return static_cast<T const&>(*this); }
      inline T&        tcast()       { return static_cast<T&>(*this); }
   };
};

#endif
