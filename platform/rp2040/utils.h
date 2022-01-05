// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2016-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _UTILS_H_
#define _UTILS_H_

/*- Definitions -------------------------------------------------------------*/
#define PACK            __attribute__((packed))
#define WEAK            __attribute__((weak))
#define INLINE          static inline __attribute__((always_inline))
#define LIMIT(a, b)     (((int)(a) > (int)(b)) ? (int)(b) : (int)(a))

#endif // _UTILS_H_

