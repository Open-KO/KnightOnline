#include "lzf.h"
#include <cstddef>
#include <cstdint>

// NOLINTBEGIN

unsigned int lzf_compress(
	const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len
#if LZF_STATE_ARG
	,
	LZF_STATE htab
#endif
)
{
#if !LZF_STATE_ARG
	thread_local LZF_STATE htab;
#endif
	const u8** hslot;
	const u8* ip     = (u8*) in_data;
	u8* op           = (u8*) out_data;
	const u8* in_end = ip + in_len;
	u8* out_end      = op + out_len;
	const u8* ref;

	/* off requires a type wide enough to hold a general pointer difference.
	* ISO C doesn't have that (size_t might not be enough and ptrdiff_t only
	* works for differences within a single object). We also assume that no
	* no bit pattern traps. Since the only platform that is both non-POSIX
	* and fails to support both assumptions is windows 64 bit, we make a
	* special workaround for it.
	*/
#if defined(WIN32) && defined(_M_X64)
	unsigned _int64 off; /* workaround for missing POSIX compliance */
#else
	unsigned long off;
#endif
	unsigned int hval;
	int lit;

	if (!in_len || !out_len)
		return 0;

#if INIT_HTAB
	memset(htab, 0, sizeof(htab));
#if 0
	for (hslot = htab; hslot < htab + HSIZE; hslot++)
		*hslot++ = ip;
#endif
#endif

	lit = 0;
	op++; /* start run */

	hval = FRST(ip);
	while (ip < in_end - 2)
	{
		hval   = NEXT(hval, ip);
		hslot  = htab + IDX(hval);
		ref    = *hslot;
		*hslot = ip;

		if (1
#if INIT_HTAB
			&& ref < ip /* the next test will actually take care of this, but this is faster */
#endif
			// NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
			&& (off = ip - ref - 1) < MAX_OFF && ip + 4 < in_end && ref > (u8*) in_data
#if STRICT_ALIGN
			&& ref[0] == ip[0] && ref[1] == ip[1] && ref[2] == ip[2]
#else
			&& *(u16*) ref == *(u16*) ip && ref[2] == ip[2]
#endif
		)
		{
			/* match found at *ref++ */
			size_t len    = 2;
			size_t maxlen = in_end - ip - len;
			maxlen        = maxlen > MAX_REF ? MAX_REF : maxlen;

			if (expect_false(op + 3 + 1 >= out_end)) /* first a faster conservative test */
				if (op - !lit + 3 + 1 >= out_end)    /* second the exact but rare test */
					return 0;

			op[-lit - 1]  = lit - 1;                 /* stop run */
			op           -= !lit;                    /* undo run if length is zero */

			for (;;)
			{
				if (expect_true(maxlen > 16))
				{
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;

					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;

					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;

					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
					len++;
					if (ref[len] != ip[len])
						break;
				}

				do
					len++;
				while (len < maxlen && ref[len] == ip[len]);

				break;
			}

			len -= 2; /* len is now #octets - 1 */
			ip++;

			if (len < 7)
			{
				*op++ = (u8) ((off >> 8) + (len << 5));
			}
			else
			{
				*op++ = (u8) ((off >> 8) + (7 << 5));
				*op++ = (u8) (len - 7);
			}

			*op++ = (u8) off;
			lit   = 0;
			op++; /* start run */

			ip += len + 1;

			if (expect_false(ip >= in_end - 2))
				break;

#if ULTRA_FAST || VERY_FAST
			--ip;
#if VERY_FAST && !ULTRA_FAST
			--ip;
#endif
			hval            = FRST(ip);

			hval            = NEXT(hval, ip);
			htab[IDX(hval)] = ip;
			ip++;

#if VERY_FAST && !ULTRA_FAST
			hval            = NEXT(hval, ip);
			htab[IDX(hval)] = ip;
			ip++;
#endif
#else
			ip -= len + 1;

			do
			{
				hval            = NEXT(hval, ip);
				htab[IDX(hval)] = ip;
				ip++;
			}
			while (len--);
#endif
		}
		else
		{
			/* one more literal byte we must copy */
			if (expect_false(op >= out_end))
				return 0;

			lit++;
			*op++ = *ip++;

			if (expect_false(lit == MAX_LIT))
			{
				op[-lit - 1] = lit - 1; /* stop run */
				lit          = 0;
				op++;                   /* start run */
			}
		}
	}

	if (op + 3 > out_end) /* at most 3 bytes can be missing here */
		return 0;

	while (ip < in_end)
	{
		lit++;
		*op++ = *ip++;

		if (expect_false(lit == MAX_LIT))
		{
			op[-lit - 1] = lit - 1; /* stop run */
			lit          = 0;
			op++;                   /* start run */
		}
	}

	op[-lit - 1]  = lit - 1; /* end run */
	op           -= !lit;    /* undo run if length is zero */

	return static_cast<unsigned int>(op - (u8*) out_data);
}

unsigned int lzf_decompress(
	const void* const in_data, unsigned int in_len, void* out_data, unsigned int out_len)
{
	u8 const* ip           = (const u8*) in_data;
	u8* op                 = (u8*) out_data;
	u8 const* const in_end = ip + in_len;
	u8* const out_end      = op + out_len;

	do
	{
		unsigned int ctrl = *ip++;

		if (ctrl < (1 << 5)) /* literal run */
		{
			ctrl++;

			if (op + ctrl > out_end)
			{
				SET_ERRNO(E2BIG);
				return 0;
			}

#if CHECK_INPUT
			if (ip + ctrl > in_end)
			{
				SET_ERRNO(EINVAL);
				return 0;
			}
#endif

#ifdef lzf_movsb
			lzf_movsb(op, ip, ctrl);
#else
			do
				*op++ = *ip++;
			while (--ctrl);
#endif
		}
		else /* back reference */
		{
			unsigned int len = ctrl >> 5;

			u8* ref          = op - static_cast<intptr_t>((ctrl & 0x1f) << 8) - 1;

#if CHECK_INPUT
			if (ip >= in_end)
			{
				SET_ERRNO(EINVAL);
				return 0;
			}
#endif
			if (len == 7)
			{
				len += *ip++;
#if CHECK_INPUT
				if (ip >= in_end)
				{
					SET_ERRNO(EINVAL);
					return 0;
				}
#endif
			}

			ref -= *ip++;

			if (op + len + 2 > out_end)
			{
				SET_ERRNO(E2BIG);
				return 0;
			}

			if (ref < (u8*) out_data)
			{
				SET_ERRNO(EINVAL);
				return 0;
			}

#ifdef lzf_movsb
			len += 2;
			lzf_movsb(op, ref, len);
#else
			*op++ = *ref++;
			*op++ = *ref++;

			do
				*op++ = *ref++;
			while (--len);
#endif
		}
	}
	while (ip < in_end);

	return static_cast<unsigned int>(op - (u8*) out_data);
}

// NOLINTEND
