/*
* Copyright (c) 2000-2008 Marc Alexander Lehmann <schmorp@schmorp.de>
* 
* Redistribution and use in source and binary forms, with or without modifica-
* tion, are permitted provided that the following conditions are met:
* 
*   1.  Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
* 
*   2.  Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
* CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
* CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
* ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License ("GPL") version 2 or any later version,
* in which case the provisions of the GPL are applicable instead of
* the above. If you wish to allow the use of your version of this file
* only under the terms of the GPL and not to allow others to use your
* version of this file under the BSD license, indicate your decision
* by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL. If you do not delete the
* provisions above, a recipient may use your version of this file under
* either the BSD or the GPL.
*/

#ifndef SHARED_LZF_H
#define SHARED_LZF_H

#pragma once

// This is third party code which was written for C. We can ignore it.
// NOLINTBEGIN

#define LZF_MARGIN  128
#define LZF_VERSION 0x0105 /* 1.5, API version */
#define AVOID_ERRNO 1

#ifndef HLOG
#define HLOG 16
#endif

/*
* Sacrifice very little compression quality in favour of compression speed.
* This gives almost the same compression as the default code, and is
* (very roughly) 15% faster. This is the preferred mode of operation.
*/
#ifndef VERY_FAST
#define VERY_FAST 1
#endif

/*
* Sacrifice some more compression quality in favour of compression speed.
* (roughly 1-2% worse compression for large blocks and
* 9-10% for small, redundant, blocks and >>20% better speed in both cases)
* In short: when in need for speed, enable this for binary data,
* possibly disable this for text data.
*/
#ifndef ULTRA_FAST
#define ULTRA_FAST 0
#endif

/*
* Unconditionally aligning does not cost very much, so do it if unsure
*/
#ifndef STRICT_ALIGN

#if !(defined(__i386) || defined(__amd64))
#define STRICT_ALIGN 1
#else
#define STRICT_ALIGN 0
#endif

#endif

/*
* You may choose to pre-set the hash table (might be faster on some
* modern cpus and large (>>64k) blocks, and also makes compression
* deterministic/repeatable when the configuration otherwise is the same).
*/
#ifndef INIT_HTAB
#define INIT_HTAB 0
#endif

/*
* Avoid assigning values to errno variable? for some embedding purposes
* (linux kernel for example), this is neccessary. NOTE: this breaks
* the documentation in lzf.h.
*/
#ifndef AVOID_ERRNO
#define AVOID_ERRNO 0
#endif

/*
* Wether to pass the LZF_STATE variable as argument, or allocate it
* on the stack. For small-stack environments, define this to 1.
* NOTE: this breaks the prototype in lzf.h.
*/
#ifndef LZF_STATE_ARG
#define LZF_STATE_ARG 0
#endif

/*
* Wether to add extra checks for input validity in lzf_decompress
* and return EINVAL if the input stream has been corrupted. This
* only shields against overflowing the input buffer and will not
* detect most corrupted streams.
* This check is not normally noticable on modern hardware
* (<1% slowdown), but might slow down older cpus considerably.
*/
#ifndef CHECK_INPUT
#define CHECK_INPUT 1
#endif

/*****************************************************************************/
/* nothing should be changed below */

typedef unsigned char u8;

typedef const u8* LZF_STATE[1 << (HLOG)];

#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
#include <limits.h>
#if USHRT_MAX == 65535
typedef unsigned short u16;
#elif UINT_MAX == 65535
typedef unsigned int u16;
#else
#undef STRICT_ALIGN
#define STRICT_ALIGN 1
#endif
#endif

#if ULTRA_FAST
#if defined(VERY_FAST)
#undef VERY_FAST
#endif
#endif

#if INIT_HTAB
#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif
#endif

#define HSIZE (1 << (HLOG))

/*
* don't play with this unless you benchmark!
* decompression is not dependent on the hash function
* the hashing function might seem strange, just believe me
* it works ;)
*/
#ifndef FRST
#define FRST(p)    ((((p)[0]) << 8) | (p)[1])
#define NEXT(v, p) (((v) << 8) | (p)[2])
#if ULTRA_FAST
#define IDX(h) ((((h) >> (3 * 8 - HLOG)) - (h)) & (HSIZE - 1))
#elif VERY_FAST
#define IDX(h) ((((h) >> (3 * 8 - HLOG)) - (h) * 5) & (HSIZE - 1))
#else
#define IDX(h) (((((h) ^ ((h) << 5)) >> (3 * 8 - HLOG)) - (h) * 5) & (HSIZE - 1))
#endif
#endif
/*
* IDX works because it is very similar to a multiplicative hash, e.g.
* ((h * 57321 >> (3*8 - HLOG)) & (HSIZE - 1))
* the latter is also quite fast on newer CPUs, and compresses similarly.
*
* the next one is also quite good, albeit slow ;)
* (int)(cos(h & 0xffffff) * 1e6)
*/

#if 0
/* original lzv-like hash function, much worse and thus slower */
#define FRST(p)    ((p)[0] << 5) ^ (p)[1]
#define NEXT(v, p) ((v) << 5) ^ (p)[2]
#define IDX(h)     ((h) & (HSIZE - 1))
#endif

#define MAX_LIT (1 << 5)
#define MAX_OFF (1 << 13)
#define MAX_REF ((1 << 8) + (1 << 3))

#if __GNUC__ >= 3
#define expect(expr, value) __builtin_expect((expr), (value))
#else
#define expect(expr, value) (expr)
#endif

#define expect_false(expr) expect((expr) != 0, 0)
#define expect_true(expr)  expect((expr) != 0, 1)

/*
* compressed format
*
* 000LLLLL <L+1>    ; literal
* LLLooooo oooooooo ; backref L
* 111ooooo LLLLLLLL oooooooo ; backref L+7
*
*/

unsigned int lzf_compress(
	const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len
#if LZF_STATE_ARG
	,
	LZF_STATE htab
#endif
);

/*
* Decompress data compressed with some version of the lzf_compress
* function and stored at location in_data and length in_len. The result
* will be stored at out_data up to a maximum of out_len characters.
*
* If the output buffer is not large enough to hold the decompressed
* data, a 0 is returned and errno is set to E2BIG. Otherwise the number
* of decompressed bytes (i.e. the original length of the data) is
* returned.
*
* If an error in the compressed data is detected, a zero is returned and
* errno is set to EINVAL.
*
* This function is very fast, about as fast as a copying loop.
*/

#if AVOID_ERRNO
#define SET_ERRNO(n)
#else
#include <errno.h>
#define SET_ERRNO(n) errno = (n)
#endif

#if (__i386 || __amd64) && __GNUC__ >= 3
#define lzf_movsb(dst, src, len)                                                       \
	asm("rep movsb" : "=D"(dst), "=S"(src), "=c"(len) : "0"(dst), "1"(src), "2"(len));
#endif

unsigned int lzf_decompress(
	const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len);

// NOLINTEND

#endif // SHARED_LZF_H
