/*

crc32.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1992 Tatu Ylonen, Espoo, Finland
All rights reserved

Created: Tue Feb 11 14:37:27 1992 ylo

Functions for computing 32-bit CRC.

*/

/*
* $Id: crc32.h,v 1.1.1.1 1996/02/18 21:38:11 ylo Exp $
* $Log: crc32.h,v $
* Revision 1.1.1.1  1996/02/18 21:38:11  ylo
* 	Imported ssh-1.2.13.
*
* Revision 1.2  1995/07/13  01:21:45  ylo
* 	Removed "Last modified" header.
* 	Added cvs log.
*
* $Endlog$
*/

#ifndef SHARED_CRC32_H
#define SHARED_CRC32_H

#pragma once

// This is a third party library written for C.
// We can ignore it.
// NOLINTBEGIN

/* This computes a 32 bit CRC of the data in the buffer, and returns the
CRC.  The polynomial used is 0xedb88320. */
unsigned int crc32(const unsigned char* s, unsigned int len, unsigned int startVal = 0);

// NOLINTEND

#endif // SHARED_CRC32_H
