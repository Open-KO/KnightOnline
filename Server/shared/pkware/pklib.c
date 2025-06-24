/*****************************************************************************/
/* pklib.c                                Copyright (c) Ladislav Zezula 2003 */
/*****************************************************************************/

#include <assert.h>
#include <string.h>

#include "pklib.h"

//#define USE_PKLIB_FIXES 1

#if ((1200 < _MSC_VER) && (_MSC_VER < 1400))
#pragma optimize("", off)               // Fucking Microsoft VS.NET 2003 compiler !!!
// (_MSC_VER=1310)
#endif

//-----------------------------------------------------------------------------
// Defines

#define DICT_OFFSET   0x204
#define UNCMP_OFFSET  (pWork->dsize_bytes + DICT_OFFSET)

//-----------------------------------------------------------------------------
// Tables

const static unsigned char DistBits[] =
{
	0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

const static unsigned char DistCode[] =
{
	0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E, 0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A,
	0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02, 0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C,
	0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04, 0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
	0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10, 0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00
};

const static unsigned char ExLenBits[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

const static unsigned short LenBase[] =
{
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	0x0008, 0x000A, 0x000E, 0x0016, 0x0026, 0x0046, 0x0086, 0x0106
};

const static unsigned char LenBits[] =
{
	0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07
};

const static unsigned char LenCode[] =
{
	0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14, 0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00
};

const static unsigned char ChBitsAsc[] =
{
	0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x08, 0x07, 0x0C, 0x0C, 0x07, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x04, 0x0A, 0x08, 0x0C, 0x0A, 0x0C, 0x0A, 0x08, 0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08,
	0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x08, 0x0C, 0x0B, 0x07, 0x09, 0x0B,
	0x0C, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08, 0x08, 0x06, 0x0B, 0x09, 0x06, 0x07, 0x06, 0x06,
	0x07, 0x0B, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x09, 0x09, 0x0B, 0x08, 0x0B, 0x09, 0x0C, 0x08,
	0x0C, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x0B, 0x07, 0x05, 0x06, 0x05, 0x05,
	0x06, 0x0A, 0x05, 0x05, 0x05, 0x05, 0x08, 0x07, 0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0C, 0x0D,
	0x0D, 0x0D, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D
};

const static unsigned short ChCodeAsc[] =
{
	0x0490, 0x0FE0, 0x07E0, 0x0BE0, 0x03E0, 0x0DE0, 0x05E0, 0x09E0,
	0x01E0, 0x00B8, 0x0062, 0x0EE0, 0x06E0, 0x0022, 0x0AE0, 0x02E0,
	0x0CE0, 0x04E0, 0x08E0, 0x00E0, 0x0F60, 0x0760, 0x0B60, 0x0360,
	0x0D60, 0x0560, 0x1240, 0x0960, 0x0160, 0x0E60, 0x0660, 0x0A60,
	0x000F, 0x0250, 0x0038, 0x0260, 0x0050, 0x0C60, 0x0390, 0x00D8,
	0x0042, 0x0002, 0x0058, 0x01B0, 0x007C, 0x0029, 0x003C, 0x0098,
	0x005C, 0x0009, 0x001C, 0x006C, 0x002C, 0x004C, 0x0018, 0x000C,
	0x0074, 0x00E8, 0x0068, 0x0460, 0x0090, 0x0034, 0x00B0, 0x0710,
	0x0860, 0x0031, 0x0054, 0x0011, 0x0021, 0x0017, 0x0014, 0x00A8,
	0x0028, 0x0001, 0x0310, 0x0130, 0x003E, 0x0064, 0x001E, 0x002E,
	0x0024, 0x0510, 0x000E, 0x0036, 0x0016, 0x0044, 0x0030, 0x00C8,
	0x01D0, 0x00D0, 0x0110, 0x0048, 0x0610, 0x0150, 0x0060, 0x0088,
	0x0FA0, 0x0007, 0x0026, 0x0006, 0x003A, 0x001B, 0x001A, 0x002A,
	0x000A, 0x000B, 0x0210, 0x0004, 0x0013, 0x0032, 0x0003, 0x001D,
	0x0012, 0x0190, 0x000D, 0x0015, 0x0005, 0x0019, 0x0008, 0x0078,
	0x00F0, 0x0070, 0x0290, 0x0410, 0x0010, 0x07A0, 0x0BA0, 0x03A0,
	0x0240, 0x1C40, 0x0C40, 0x1440, 0x0440, 0x1840, 0x0840, 0x1040,
	0x0040, 0x1F80, 0x0F80, 0x1780, 0x0780, 0x1B80, 0x0B80, 0x1380,
	0x0380, 0x1D80, 0x0D80, 0x1580, 0x0580, 0x1980, 0x0980, 0x1180,
	0x0180, 0x1E80, 0x0E80, 0x1680, 0x0680, 0x1A80, 0x0A80, 0x1280,
	0x0280, 0x1C80, 0x0C80, 0x1480, 0x0480, 0x1880, 0x0880, 0x1080,
	0x0080, 0x1F00, 0x0F00, 0x1700, 0x0700, 0x1B00, 0x0B00, 0x1300,
	0x0DA0, 0x05A0, 0x09A0, 0x01A0, 0x0EA0, 0x06A0, 0x0AA0, 0x02A0,
	0x0CA0, 0x04A0, 0x08A0, 0x00A0, 0x0F20, 0x0720, 0x0B20, 0x0320,
	0x0D20, 0x0520, 0x0920, 0x0120, 0x0E20, 0x0620, 0x0A20, 0x0220,
	0x0C20, 0x0420, 0x0820, 0x0020, 0x0FC0, 0x07C0, 0x0BC0, 0x03C0,
	0x0DC0, 0x05C0, 0x09C0, 0x01C0, 0x0EC0, 0x06C0, 0x0AC0, 0x02C0,
	0x0CC0, 0x04C0, 0x08C0, 0x00C0, 0x0F40, 0x0740, 0x0B40, 0x0340,
	0x0300, 0x0D40, 0x1D00, 0x0D00, 0x1500, 0x0540, 0x0500, 0x1900,
	0x0900, 0x0940, 0x1100, 0x0100, 0x1E00, 0x0E00, 0x0140, 0x1600,
	0x0600, 0x1A00, 0x0E40, 0x0640, 0x0A40, 0x0A00, 0x1200, 0x0200,
	0x1C00, 0x0C00, 0x1400, 0x0400, 0x1800, 0x0800, 0x1000, 0x0000
};

//-----------------------------------------------------------------------------
// Local variables

static char Copyright[] =
	"PKWARE Data Compression Library for Win32\r\n"
	"Copyright 1989-1995 PKWARE Inc.  All Rights Reserved\r\n"
	"Patent No. 5,051,745\r\n"
	"PKWARE Data Compression Library Reg. U.S. Pat. and Tm. Off.\r\n"
	"Version 1.11\r\n";

//***********************************************
//* Compression (implode)
//***********************************************

void SortBuffer(TCmpStruct * pWork, unsigned char * uncmp_data, unsigned char * work_end)
{
	unsigned short * pin0DC8;
	unsigned char  * puncmp;
	unsigned long    offs1, offs2;
	unsigned long    ndwords;
	unsigned int     add;

	// Fill 0x480 dwords (0x1200 bytes)
	ndwords = (unsigned long)((pWork->out_buff - (char *)pWork->offs0DC8 + 1) >> 2);
	if (ndwords <= 1)
		ndwords = 1;
	memset(pWork->offs0DC8, 0, ndwords << 2);

	for (puncmp = uncmp_data; work_end > puncmp; puncmp++)
		pWork->offs0DC8[(puncmp[0] * 4) + (puncmp[1] * 5)]++;

	add = 0;
	for (pin0DC8 = pWork->offs0DC8; pin0DC8 < &pWork->offs1FC8; pin0DC8++)
	{
		add += *pin0DC8;
		*pin0DC8 = (unsigned short)add;
	}

	for (work_end--; work_end >= uncmp_data; work_end--)
	{
		offs1 = (work_end[0] * 4) + (work_end[1] * 5);  // EAX
		offs2 = (unsigned long)(work_end - pWork->work_buff);          // EDI

		pWork->offs0DC8[offs1]--;
		pWork->offs49D0[pWork->offs0DC8[offs1]] = (unsigned short)offs2;
	}
}

void FlushBuf(TCmpStruct * pWork)
{
	unsigned char save_ch1;
	unsigned char save_ch2;
	unsigned int size = 0x800;

	pWork->write_buf(pWork->out_buff, &size, pWork->param);

	save_ch1 = pWork->out_buff[0x800];
	save_ch2 = pWork->out_buff[pWork->out_bytes];
	pWork->out_bytes -= 0x800;

	memset(pWork->out_buff, 0, 0x802);

	if (pWork->out_bytes != 0)
		pWork->out_buff[0] = save_ch1;
	if (pWork->out_bits != 0)
		pWork->out_buff[pWork->out_bytes] = save_ch2;
}

void OutputBits(TCmpStruct * pWork, unsigned int nbits, unsigned long bit_buff)
{
	unsigned int out_bits;

	// If more than 8 bits to output, do recursion
	if (nbits > 8)
	{
		OutputBits(pWork, 8, bit_buff);
		bit_buff >>= 8;
		nbits -= 8;
	}

	// Add bits to the last out byte in out_buff;
	out_bits = pWork->out_bits;
	pWork->out_buff[pWork->out_bytes] |= (unsigned char)(bit_buff << out_bits);
	pWork->out_bits += nbits;

	// If 8 or more bits, increment number of bytes
	if (pWork->out_bits > 8)
	{
		++pWork->out_bytes;
		bit_buff >>= (8 - out_bits);

		pWork->out_buff[pWork->out_bytes] = (unsigned char)bit_buff;
		pWork->out_bits &= 7;
	}
	else
	{
		pWork->out_bits &= 7;
		if (pWork->out_bits == 0)
			++pWork->out_bytes;
	}

	// If there is enough compressed bytes, flush them
	if (pWork->out_bytes >= 0x800)
		FlushBuf(pWork);
}

unsigned long FindRep(TCmpStruct * pWork, unsigned char * srcbuff)
{
	unsigned short  esp12;
	unsigned char * esp14;
	unsigned short  esp18;
	unsigned char * srcbuff2;
	unsigned char   esp20;

	unsigned char  * srcbuff3;
	unsigned short * pin0DC8;
	unsigned char  * pin27CC;
	unsigned short * pin49D0;
	unsigned long    nreps = 1;         // EAX
	unsigned long    ebx, esi;
	unsigned short   di;

	pin0DC8 = &pWork->offs0DC8[5 * srcbuff[1] + 4 * srcbuff[0]];
	esi = (unsigned long)(srcbuff - pWork->dsize_bytes - pWork->work_buff + 1);

	esp18 = *pin0DC8;
	pin49D0 = pWork->offs49D0 + esp18;
	if (*pin49D0 < esi)
	{
		do
		{
			++pin49D0;
			++esp18;
		} while (*pin49D0 < esi);
		*pin0DC8 = esp18;
	}
	//---------------------------------------------------------------------------
	srcbuff2 = srcbuff - 1;
	pin49D0 = pWork->offs49D0 + esp18;
	pin27CC = pWork->work_buff + *pin49D0;
	if (srcbuff2 <= pin27CC)
		return 0;
	//---------------------------------------------------------------------------
	srcbuff3 = srcbuff;
	for (;;)
	{
		if (srcbuff3[nreps - 1] == pin27CC[nreps - 1] && *srcbuff3 == *pin27CC)
		{
			//
			// The following code does not work when compiled with MSVC.NET 2003
			// optimizing compiler. We have to switch the optimizations off to make it work
			// I found that in debug version (where the optimizations are off), the value
			// of "pin27CC" gets incremented twice (once at below, once in the "for" loop)
			//

			pin27CC++;
			srcbuff3++;

			for (ebx = 2; ebx < DICT_OFFSET; ebx++)
			{
				pin27CC++;
				srcbuff3++;
				if (*pin27CC != *srcbuff3)
					break;
			}

			srcbuff3 = srcbuff;
			if (ebx >= nreps)
			{
				pWork->offs0000 = (unsigned int)(srcbuff3 - pin27CC + ebx - 1);
				if ((nreps = ebx) > 10)
					break;
			}
		}

		pin49D0++;
		esp18++;
		pin27CC = pWork->work_buff + *pin49D0;

		if (srcbuff2 > pin27CC)
			continue;

		return (nreps >= 2) ? nreps : 0;
	}
	//---------------------------------------------------------------------------
	if (ebx == DICT_OFFSET)
	{
		pWork->offs0000--;
		return ebx;
	}
	//---------------------------------------------------------------------------
	pin49D0 = pWork->offs49D0 + esp18;
	if (pWork->work_buff + pin49D0[1] >= srcbuff2)
		return nreps;
	//---------------------------------------------------------------------------
	di = 0;
	pWork->offs09BC[0] = 0xFFFF;
	pWork->offs09BC[1] = di;
	esp12 = 1;

	do
	{
		esi = di;
		if (srcbuff[esp12] != srcbuff[esi])
		{
			di = pWork->offs09BC[esi];
			if (di != 0xFFFF)
				continue;
		}
		pWork->offs09BC[++esp12] = ++di;
	} while (esp12 < nreps);
	//---------------------------------------------------------------------------
	esi = nreps;
	pin27CC = pWork->work_buff + pin49D0[0] + nreps;
	esp14 = pin27CC;

	for (;;)   // 0040268B
	{
		esi = pWork->offs09BC[esi];
		if (esi == 0xFFFF)
			esi = 0;

		pin49D0 = pWork->offs49D0 + esp18;
		do
		{
			pin49D0++;
			esp18++;
			pin27CC = pWork->work_buff + pin49D0[0];
			if (pin27CC >= srcbuff2)
				return nreps;
		} while (pin27CC + esi < esp14);
		//---------------------------------------------------------------------------
		esp20 = srcbuff[nreps - 2];
		if (esp20 == pin27CC[nreps - 2])
		{
			if (pin27CC + esi != esp14)
			{
				esp14 = pin27CC;
				esi = 0;
			}
		}
		else
		{
			pin49D0 = pWork->offs49D0 + esp18;
			do
			{
				pin49D0++;
				esp18++;
				pin27CC = pWork->work_buff + pin49D0[0];
				if (pin27CC >= srcbuff2)
					return nreps;
			} while (pin27CC[nreps - 2] != esp20 || pin27CC[0] != *srcbuff);

			esp14 = pin27CC + 2;
			esi = 2;
		}
		//---------------------------------------------------------------------------
		for (; esp14[0] == srcbuff[esi]; esp14++)
		{
			if (++esi >= DICT_OFFSET)
				break;
		}

		if (esi < nreps)
			continue;
		pWork->offs0000 = (unsigned int)(srcbuff - pin27CC - 1);
		if (esi <= nreps)
			continue;
		nreps = esi;
		if (esi == DICT_OFFSET)
			return nreps;

		do
		{
			if (srcbuff[esp12] != srcbuff[di])
			{
				di = pWork->offs09BC[di];
				if (di != 0xFFFF)
					continue;
			}
			pWork->offs09BC[++esp12] = ++di;
		} while (esp12 < esi);
	}
}

void WriteCmpData(TCmpStruct *pWork)
{
	unsigned char *uncmp_begin = &pWork->work_buff[UNCMP_OFFSET]; // edi@1
	signed int bytes_required; // ebp@2
	int total_loaded; // ebx@2
	unsigned int loaded; // eax@3
	unsigned int nreps1; // ebx@19
	unsigned int save_offs0000; // ebp@25
	unsigned int nreps; // [sp+10h] [bp-14h]@25
	unsigned char * uncmp_end; // [sp+14h] [bp-10h]@1
	unsigned int esp18 = 0;
	int esp20 = 0; // [sp+20h] [bp-4h]@1

	// Store the compression type and dictionary size
	pWork->out_buff[0] = pWork->ctype;
	pWork->out_buff[1] = pWork->dsize_bits;
	pWork->out_bytes = 2;

	// Reset output buffer to zero
	memset(&pWork->out_buff[2], 0, sizeof(pWork->out_buff) - 2);
	pWork->out_bits = 0;

	do
	{
		bytes_required = 4096;
		total_loaded = 0;

		while (1)
		{
			unsigned int bytes_required_ = bytes_required; // don't want the original to be modified by the callback

			loaded = pWork->read_buf(
				(char *)&pWork->work_buff[UNCMP_OFFSET + total_loaded],
				&bytes_required_,
				pWork->param);

			if (loaded == 0)
				break;

			total_loaded += loaded;
			bytes_required -= loaded;

			if (bytes_required == 0)
				break;
		}

		if (bytes_required != 0)
		{
			if (total_loaded == 0 && esp20 == 0)
			{
				OutputBits(pWork, pWork->nChBits[0x305], pWork->nChCodes[0x305]);

				if (pWork->out_bits)
					++pWork->out_bytes;

				pWork->write_buf(pWork->out_buff, &pWork->out_bytes, pWork->param);
				return;
			}

			esp18 = 1;
		}

		uncmp_end = &pWork->work_buff[pWork->dsize_bytes + total_loaded];
		if (esp18 != 0)
			uncmp_end += DICT_OFFSET;

		//
		// Warning: Passing "uncmp_end + 1" to the SortBuffer function may cause
		// the output to be unpredictable in Storm.dll's compression. Because Storm.dll
		// does not pass the zeroed buffer to the "implode" function, the byte after
		// uncmp_end contains random data. This causes difference within dictionary
		// created in SortBuffer function and may also cause different compressed output.
		// We always zero the data before compression, so this thing never occurs.
		// Funny is that it is actually not a bug, because if we decompress the data back,
		// we'll get the identical data with the original input.
		//
		switch (esp20)
		{
		case 0:
			SortBuffer(pWork, uncmp_begin, uncmp_end + 1);
			++esp20;
			if (pWork->dsize_bytes != 0x1000)
				++esp20;
			break;

		case 1:
			SortBuffer(pWork, uncmp_begin - pWork->dsize_bytes + DICT_OFFSET, uncmp_end + 1);
			esp20++;
			break;

		default:
			SortBuffer(pWork, uncmp_begin - pWork->dsize_bytes, uncmp_end + 1);
			break;
		}

		while (uncmp_end > uncmp_begin)
		{
			nreps1 = FindRep(pWork, uncmp_begin);
			if (nreps1 != 0)
			{
				while (1)
				{
					if (nreps1 == 2 && pWork->offs0000 >= 0x100)
						goto LABEL_29;

					if (&uncmp_begin[nreps1] > uncmp_end)
						break;

					if (nreps1 >= 8 || uncmp_begin + 1 >= uncmp_end)
						goto LABEL_41;

					save_offs0000 = pWork->offs0000;
					nreps = nreps1;
					nreps1 = FindRep(pWork, uncmp_begin + 1);

					if (nreps >= nreps1
						|| (nreps + 1 >= nreps1 && save_offs0000 <= 0x80))
					{
						nreps1 = nreps;
						pWork->offs0000 = save_offs0000;
					LABEL_41:
						OutputBits(pWork, pWork->nChBits[nreps1 + 0xFE], pWork->nChCodes[nreps1 + 0xFE]);
						if (nreps1 == 2)
						{
							OutputBits(pWork, pWork->dist_bits[pWork->offs0000 >> 2], pWork->dist_codes[pWork->offs0000 >> 2]);
							uncmp_begin += 2;
							OutputBits(pWork, 2u, pWork->offs0000 & 3);
						}
						else
						{
							OutputBits(
								pWork,
								pWork->dist_bits[pWork->offs0000 >> ((unsigned char)(pWork->dsize_bits & 0xff))],
								pWork->dist_codes[pWork->offs0000 >> ((unsigned char)(pWork->dsize_bits & 0xff))]);

							uncmp_begin += nreps1;
							OutputBits(pWork, pWork->dsize_bits, pWork->offs0000 & pWork->dsize_mask);
						}
						goto LABEL_30;
					}

					OutputBits(pWork, pWork->nChBits[*uncmp_begin], pWork->nChCodes[*uncmp_begin]);
					*uncmp_begin++;

					if (nreps1 == 0)
						goto LABEL_29;
				}
				nreps1 = uncmp_end - uncmp_begin;

				if ((uncmp_end - uncmp_begin) >= 2
					&& (uncmp_end - uncmp_begin != 2 || pWork->offs0000 < 0x100))
					goto LABEL_41;
			}
		LABEL_29:
			OutputBits(pWork, pWork->nChBits[*uncmp_begin], pWork->nChCodes[*uncmp_begin]);
			*uncmp_begin++;
		LABEL_30:
			;
		}

		if (esp18 == 0)
		{
			uncmp_begin -= 0x1000;
			memcpy(pWork->work_buff, &pWork->work_buff[0x1000], pWork->dsize_bytes + DICT_OFFSET);
		}
	} while (esp18 == 0);

	OutputBits(pWork, pWork->nChBits[0x305], pWork->nChCodes[0x305]);
	if (pWork->out_bits)
		++pWork->out_bytes;

	pWork->write_buf(pWork->out_buff, &pWork->out_bytes, pWork->param);
}

//-----------------------------------------------------------------------------
// Main imploding function

unsigned int PKEXPORT implode(
	unsigned int(*read_buf)(char *buf, unsigned int *size, void *param),
	void(*write_buf)(char *buf, unsigned int *size, void *param),
	char         *work_buf,
	void         *param,
	unsigned int *type,
	unsigned int *dsize)
{
	TCmpStruct * pWork = (TCmpStruct *)work_buf;
	unsigned short nChCode;
	unsigned int nCount;
	unsigned int i;

	// Initialize the work buffer. This is not in the Pklib,
	// but it seems to be a bug. Storm always pre-fills the data with zeros,
	// and always compresses one block only. So the bug will not appear.
	// But when a larger data block (size > 0x1000) is compressed, 
	// it may fail.
#if USE_PKLIB_FIXES
	memset(pWork, 0, sizeof(TCmpStruct));
#endif

	// Fill the work buffer information
	pWork->read_buf = read_buf;
	pWork->write_buf = write_buf;
	pWork->dsize_bytes = *dsize;
	pWork->ctype = *type;
	pWork->param = param;
	pWork->dsize_bits = 4;
	pWork->dsize_mask = 0x0F;

	// Test dictionary size
	switch (*dsize)
	{
	case 0x1000:
		pWork->dsize_bits++;
		pWork->dsize_mask |= 0x20;
		// No break here !!!

	case 0x0800:
		pWork->dsize_bits++;
		pWork->dsize_mask |= 0x10;
		// No break here !!!

	case 0x0400:
		break;

	default:
		return CMP_INVALID_DICTSIZE;
	}

	// Test the compression type
	switch (*type)
	{
	case CMP_BINARY: // We will compress data with binary compression type
		for (nChCode = 0, nCount = 0; nCount < 0x100; nCount++)
		{
			pWork->nChBits[nCount] = 9;
			pWork->nChCodes[nCount] = (unsigned short)nChCode;
			nChCode += 2;
		}
		break;


	case CMP_ASCII: // We will compress data with ASCII compression type
		for (nCount = 0; nCount < 0x100; nCount++)
		{
			pWork->nChBits[nCount] = (unsigned char)(ChBitsAsc[nCount] + 1);
			pWork->nChCodes[nCount] = (unsigned short)(ChCodeAsc[nCount] * 2);
		}
		break;

	default:
		return CMP_INVALID_MODE;
	}

	for (i = 0; i < 0x10; i++)
	{
		int nCount2 = 0;    // EBX 

		if ((1 << ExLenBits[i]) == 0)
			continue;

		do
		{
			pWork->nChBits[nCount] = (unsigned char)(ExLenBits[i] + LenBits[i] + 1);
			pWork->nChCodes[nCount] = (unsigned short)((nCount2 << (LenBits[i] + 1)) | ((LenCode[i] & 0xFFFF00FF) * 2) | 1);

			nCount2++;
			nCount++;
		} while ((1 << ExLenBits[i]) > nCount2);
	}

	// Copy the distance codes and distance bits and perform the compression
	memcpy(&pWork->dist_codes, DistCode, sizeof(DistCode));
	memcpy(&pWork->dist_bits, DistBits, sizeof(DistBits));
	WriteCmpData(pWork);

	return CMP_NO_ERROR;
}

//***********************************************
//* Decompression (explode)
//***********************************************

void GenDecodeTabs(long count, unsigned char * bits, const unsigned char * pCode, unsigned char * buffer2)
{
	long i;

	for (i = count - 1; i >= 0; i--)             // EBX - count
	{
		unsigned long idx1 = pCode[i];
		unsigned long idx2 = 1 << bits[i];

		do
		{
			buffer2[idx1] = (unsigned char)i;
			idx1 += idx2;
		} while (idx1 < 0x100);
	}
}

void GenAscTabs(TDcmpStruct * pWork)
{
	const unsigned short * pChCodeAsc = &ChCodeAsc[0xFF];
	unsigned long  acc, add;
	unsigned short count;

	for (count = 0x00FF; pChCodeAsc >= ChCodeAsc; pChCodeAsc--, count--)
	{
		unsigned char * pChBitsAsc = pWork->ChBitsAsc + count;
		unsigned char bits_asc = *pChBitsAsc;

		if (bits_asc <= 8)
		{
			add = (1 << bits_asc);
			acc = *pChCodeAsc;

			do
			{
				pWork->offs2C34[acc] = (unsigned char)count;
				acc += add;
			} while (acc < 0x100);
		}
		else if ((acc = (*pChCodeAsc & 0xFF)) != 0)
		{
			pWork->offs2C34[acc] = 0xFF;

			if (*pChCodeAsc & 0x3F)
			{
				bits_asc -= 4;
				*pChBitsAsc = bits_asc;

				add = (1 << bits_asc);
				acc = *pChCodeAsc >> 4;
				do
				{
					pWork->offs2D34[acc] = (unsigned char)count;
					acc += add;
				} while (acc < 0x100);
			}
			else
			{
				bits_asc -= 6;
				*pChBitsAsc = bits_asc;

				add = (1 << bits_asc);
				acc = *pChCodeAsc >> 6;
				do
				{
					pWork->offs2E34[acc] = (unsigned char)count;
					acc += add;
				} while (acc < 0x80);
			}
		}
		else
		{
			bits_asc -= 8;
			*pChBitsAsc = bits_asc;

			add = (1 << bits_asc);
			acc = *pChCodeAsc >> 8;
			do
			{
				pWork->offs2EB4[acc] = (unsigned char)count;
				acc += add;
			} while (acc < 0x100);
		}
	}
}

//-----------------------------------------------------------------------------
// Skips given number of bits in bit buffer. Result is stored in pWork->bit_buff
// If no data in input buffer, returns true

int WasteBits(TDcmpStruct * pWork, unsigned long nBits)
{
	// If number of bits required is less than number of (bits in the buffer) ?
	if (nBits <= pWork->extra_bits)
	{
		pWork->extra_bits -= nBits;
		pWork->bit_buff >>= nBits;
		return 0;
	}

	// Load input buffer if necessary
	pWork->bit_buff >>= pWork->extra_bits;
	if (pWork->in_pos == pWork->in_bytes)
	{
		pWork->in_pos = sizeof(pWork->in_buff);
		if ((pWork->in_bytes = pWork->read_buf((char *)pWork->in_buff, &pWork->in_pos, pWork->param)) == 0)
			return 1;
		pWork->in_pos = 0;
	}

	// Update bit buffer
	pWork->bit_buff |= (pWork->in_buff[pWork->in_pos++] << 8);
	pWork->bit_buff >>= (nBits - pWork->extra_bits);
	pWork->extra_bits = (pWork->extra_bits - nBits) + 8;
	return 0;
}

//-----------------------------------------------------------------------------
// Returns : 0x000 - 0x0FF : One byte from compressed file.
//           0x100 - 0x305 : Copy previous block (0x100 = 1 byte)
//           0x306         : Out of buffer (?)

unsigned long DecodeLit(TDcmpStruct * pWork)
{
	unsigned long nBits;                // Number of bits to skip
	unsigned long value;                // Position in buffers

	// Test the current bit in byte buffer. If is not set, simply return the next byte.
	if (pWork->bit_buff & 1)
	{
		// Skip current bit in the buffer
		if (WasteBits(pWork, 1))
			return 0x306;

		// The next bits are position in buffers
		value = pWork->position2[(pWork->bit_buff & 0xFF)];

		// Get number of bits to skip
		if (WasteBits(pWork, pWork->LenBits[value]))
			return 0x306;

		if ((nBits = pWork->ExLenBits[value]) != 0)
		{
			unsigned long val2 = pWork->bit_buff & ((1 << nBits) - 1);

			if (WasteBits(pWork, nBits))
			{
				if ((value + val2) != 0x10E)
					return 0x306;
			}
			value = pWork->LenBase[value] + val2;
		}
		return value + 0x100;           // Return number of bytes to repeat
	}

	// Waste one bit
	if (WasteBits(pWork, 1))
		return 0x306;

	// If the binary compression type, read 8 bits and return them as one byte.
	if (pWork->ctype == CMP_BINARY)
	{
		value = pWork->bit_buff & 0xFF;
		if (WasteBits(pWork, 8))
			return 0x306;
		return value;
	}

	// When ASCII compression ...
	if (pWork->bit_buff & 0xFF)
	{
		value = pWork->offs2C34[pWork->bit_buff & 0xFF];

		if (value == 0xFF)
		{
			if (pWork->bit_buff & 0x3F)
			{
				if (WasteBits(pWork, 4))
					return 0x306;

				value = pWork->offs2D34[pWork->bit_buff & 0xFF];
			}
			else
			{
				if (WasteBits(pWork, 6))
					return 0x306;

				value = pWork->offs2E34[pWork->bit_buff & 0x7F];
			}
		}
	}
	else
	{
		if (WasteBits(pWork, 8))
			return 0x306;

		value = pWork->offs2EB4[pWork->bit_buff & 0xFF];
	}

	return WasteBits(pWork, pWork->ChBitsAsc[value]) ? 0x306 : value;
}

//-----------------------------------------------------------------------------
// Retrieves the number of bytes to move back 

unsigned long DecodeDist(TDcmpStruct * pWork, unsigned long dwLength)
{
	unsigned long pos = pWork->position1[(pWork->bit_buff & 0xFF)];
	unsigned long nSkip = pWork->DistBits[pos];     // Number of bits to skip

	// Skip the appropriate number of bits
	if (WasteBits(pWork, nSkip) == 1)
		return 0;

	if (dwLength == 2)
	{
		pos = (pos << 2) | (pWork->bit_buff & 0x03);

		if (WasteBits(pWork, 2) == 1)
			return 0;
	}
	else
	{
		pos = (pos << pWork->dsize_bits) | (pWork->bit_buff & pWork->dsize_mask);

		// Skip the bits
		if (WasteBits(pWork, pWork->dsize_bits) == 1)
			return 0;
	}
	return pos + 1;
}

unsigned long Expand(TDcmpStruct * pWork)
{
	unsigned int  copyBytes;            // Number of bytes to copy
	unsigned long oneByte;              // One byte from compressed file
	unsigned long dwResult;

	pWork->outputPos = 0x1000;          // Initialize output buffer position

	// If end of data or error, terminate decompress
	while ((dwResult = oneByte = DecodeLit(pWork)) < 0x305)
	{
		// If one byte is greater than 0x100, means "Repeat n - 0xFE bytes"
		if (oneByte >= 0x100)
		{
			unsigned char * source;          // ECX
			unsigned char * target;          // EDX
			unsigned long  copyLength = oneByte - 0xFE;
			unsigned long  moveBack;

			// Get length of data to copy
			if ((moveBack = DecodeDist(pWork, copyLength)) == 0)
			{
				dwResult = 0x306;
				break;
			}

			// Target and source pointer
			target = &pWork->out_buff[pWork->outputPos];
			source = target - moveBack;
			pWork->outputPos += copyLength;

			while (copyLength-- > 0)
				*target++ = *source++;
		}
		else
			pWork->out_buff[pWork->outputPos++] = (unsigned char)oneByte;

		// If number of extracted bytes has reached 1/2 of output buffer,
		// flush output buffer.
		if (pWork->outputPos >= 0x2000)
		{
			// Copy decompressed data into user buffer
			copyBytes = 0x1000;
			pWork->write_buf((char *)&pWork->out_buff[0x1000], &copyBytes, pWork->param);

			// If there are some data left, keep them alive
			memcpy(pWork->out_buff, &pWork->out_buff[0x1000], pWork->outputPos - 0x1000);
			pWork->outputPos -= 0x1000;
		}
	}

	copyBytes = pWork->outputPos - 0x1000;
	pWork->write_buf((char *)&pWork->out_buff[0x1000], &copyBytes, pWork->param);
	return dwResult;
}


//-----------------------------------------------------------------------------
// Main exploding function.

unsigned int explode(
	unsigned int(*read_buf)(char *buf, unsigned  int *size, void *param),
	void(*write_buf)(char *buf, unsigned  int *size, void *param),
	char         *work_buf,
	void         *param)
{
	TDcmpStruct * pWork = (TDcmpStruct *)work_buf;

	// Set the whole work buffer to zeros
	memset(pWork, 0, sizeof(TDcmpStruct));

	// Initialize work struct and load compressed data
	pWork->read_buf = read_buf;
	pWork->write_buf = write_buf;
	pWork->param = param;
	pWork->in_pos = sizeof(pWork->in_buff);
	pWork->in_bytes = pWork->read_buf((char *)pWork->in_buff, &pWork->in_pos, pWork->param);
	if (pWork->in_bytes <= 4)
		return CMP_BAD_DATA;

	pWork->ctype = pWork->in_buff[0]; // Get the compression type
	pWork->dsize_bits = pWork->in_buff[1]; // Get the dictionary size
	pWork->bit_buff = pWork->in_buff[2]; // Initialize 16-bit bit buffer
	pWork->extra_bits = 0;                 // Extra (over 8) bits
	pWork->in_pos = 3;                 // Position in input buffer

	// Test for the valid dictionary size
	if (4 > pWork->dsize_bits || pWork->dsize_bits > 6)
		return CMP_INVALID_DICTSIZE;

	pWork->dsize_mask = 0xFFFF >> (0x10 - pWork->dsize_bits); // Shifted by 'sar' instruction

	if (pWork->ctype != CMP_BINARY)
	{
		if (pWork->ctype != CMP_ASCII)
			return CMP_INVALID_MODE;

		memcpy(pWork->ChBitsAsc, ChBitsAsc, sizeof(pWork->ChBitsAsc));
		GenAscTabs(pWork);
	}

	memcpy(pWork->LenBits, LenBits, sizeof(pWork->LenBits));
	GenDecodeTabs(0x10, pWork->LenBits, LenCode, pWork->position2);
	memcpy(pWork->ExLenBits, ExLenBits, sizeof(pWork->ExLenBits));
	memcpy(pWork->LenBase, LenBase, sizeof(pWork->LenBase));
	memcpy(pWork->DistBits, DistBits, sizeof(pWork->DistBits));
	GenDecodeTabs(0x40, pWork->DistBits, DistCode, pWork->position1);
	if (Expand(pWork) != 0x306)
		return CMP_NO_ERROR;

	return CMP_ABORT;
}

/*****************************************************************************/
/* crc32.c                                Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Pkware Data Compression Library Version 1.11                              */
/* Dissassembled method crc32 - cdecl version                                */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 09.04.03  1.00  Lad  The first version of crc32.c                         */
/* 02.05.03  1.00  Lad  Stress test done                                     */
/*****************************************************************************/

static unsigned long crc_table[] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
	0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
	0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
	0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
	0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
	0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
	0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
	0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
	0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
	0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525,
	0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
	0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
	0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
	0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6,
	0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
	0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
	0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7,
	0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
	0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
	0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330,
	0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

unsigned long PKEXPORT crc32pk(char* buffer, unsigned int* psize, unsigned long* old_crc)
{
	unsigned int  size = *psize;
	unsigned long crc_value = *old_crc;

	while (size-- != 0)
	{
		unsigned long ch;
		ch = *buffer++ ^ (char) crc_value;
		crc_value >>= 8;

		crc_value = crc_table[ch & 0x0FF] ^ crc_value;
	}
	return crc_value;
}
