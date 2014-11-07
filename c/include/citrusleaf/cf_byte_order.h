/*
 * Copyright 2013 Aerospike. All rights reserved.
 */

#pragma once

#ifndef CF_WINDOWS
#ifdef OSX
//====================================================================
// Mac OS
//

#include <libkern/OSByteOrder.h>
#include <arpa/inet.h>

#define cf_byteswap64p(_p) (OSSwapBigToHostInt64(*(_p)))

#define cf_swap_to_be16(_n) OSSwapHostToBigInt16(_n)
#define cf_swap_to_le16(_n) OSSwapHostToLittleInt16(_n)
#define cf_swap_from_be16(_n) OSSwapBigToHostInt16(_n)
#define cf_swap_from_le16(_n) OSSwapLittleToHostInt16(_n)

#define cf_swap_to_be32(_n) OSSwapHostToBigInt32(_n)
#define cf_swap_to_le32(_n) OSSwapHostToLittleInt32(_n)
#define cf_swap_from_be32(_n) OSSwapBigToHostInt32(_n)
#define cf_swap_from_le32(_n) OSSwapLittleToHostInt32(_n)

#define cf_swap_to_be64(_n) OSSwapHostToBigInt64(_n)
#define cf_swap_to_le64(_n) OSSwapHostToLittleInt64(_n)
#define cf_swap_from_be64(_n) OSSwapBigToHostInt64(_n)
#define cf_swap_from_le64(_n) OSSwapLittleToHostInt64(_n)

#else
//====================================================================
// Linux
//

#include <netinet/in.h>
#include <asm/byteorder.h>

#define htonll(_n) (__cpu_to_be64(_n))
#define ntohll(_n) (__be64_to_cpu(_n))

#define cf_byteswap64p(_p) (__swab64p((const __u64*)(_p)))

#define cf_swap_to_be16(_n) __cpu_to_be16(_n)
#define cf_swap_to_le16(_n) __cpu_to_le16(_n)
#define cf_swap_from_be16(_n) __be16_to_cpu(_n)
#define cf_swap_from_le16(_n) __le16_to_cpu(_n)

#define cf_swap_to_be32(_n) __cpu_to_be32(_n)
#define cf_swap_to_le32(_n) __cpu_to_le32(_n)
#define cf_swap_from_be32(_n) __be32_to_cpu(_n)
#define cf_swap_from_le32(_n) __le32_to_cpu(_n)

#define cf_swap_to_be64(_n) __cpu_to_be64(_n)
#define cf_swap_to_le64(_n) __cpu_to_le64(_n)
#define cf_swap_from_be64(_n) __be64_to_cpu(_n)
#define cf_swap_from_le64(_n) __le64_to_cpu(_n)

#endif
#else // CF_WINDOWS
//====================================================================
// Windows
//

#include <stdint.h>
#include <stdlib.h>
#include <WinSock2.h>

#define cf_byteswap64p(_p) (_byteswap_uint64(*(uint64_t*)(_p)))

#define cf_swap_to_be16(_n) _byteswap_uint16(_n)
#define cf_swap_to_le16(_n) (_n)
#define cf_swap_from_be16(_n) _byteswap_uint16(_n)
#define cf_swap_from_le16(_n) (_n)

#define cf_swap_to_be32(_n) _byteswap_uint32(_n)
#define cf_swap_to_le32(_n) (_n)
#define cf_swap_from_be32(_n) _byteswap_uint32(_n)
#define cf_swap_from_le32(_n) (_n)

#define cf_swap_to_be64(_n) _byteswap_uint64(_n)
#define cf_swap_to_le64(_n) (_n)
#define cf_swap_from_be64(_n) _byteswap_uint64(_n)
#define cf_swap_from_le64(_n) (_n)

#endif // CF_WINDOWS
