/*-
 * Copyright (c) 1996-2009 Peter Kennard and aRt&D Lab
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of the source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Any redistribution solely in binary form must conspicuously
 *    reproduce the following disclaimer in documentation provided with the
 *    binary redistribution.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'', WITHOUT ANY WARRANTIES, EXPRESS
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  LICENSOR SHALL
 * NOT BE LIABLE FOR ANY LOSS OR DAMAGES RESULTING FROM THE USE OF THIS
 * SOFTWARE, EITHER ALONE OR IN COMBINATION WITH ANY OTHER SOFTWARE.
 * 
 * 	$Id$
 */
#ifndef __artd_Crc_h
#define __artd_Crc_h

#include "artd/jlib_util.h"

ARTD_BEGIN

class ARTD_API_JLIB_UTIL CCITTCrc16
{
public:
	CCITTCrc16();
	void reset();
	void            addByte(unsigned char abyte);
	void            addBytes(const void *data, int len);
	inline uint16_t getCrc() 
		{ return(crc_); }
	void            test();

protected:
	uint16_t crc_;
private:
	class Impl;
	Impl &impl();
};

class ARTD_API_JLIB_UTIL SmallCCITTCrc16
{
public:
	SmallCCITTCrc16();
	void            reset();
	void            addByte(unsigned char abyte);
	void            addBytes(const void *data, int len);
	unsigned short  getCrc() { return(crc_); }
protected:
	unsigned short crc_;
};

class ARTD_API_JLIB_UTIL Crc16
{
public:
	
	Crc16();
	void reset();
	void            addByte(unsigned char abyte);
	void            addBytes(const void *data, int len);
	uint16_t        getCrc(); 
	void            test();

protected:
	uint16_t crc_;
private:
	class Impl;
	Impl &impl();
};

class ARTD_API_JLIB_UTIL Crc32
{
public:
	Crc32();
	void            reset();
	void            addByte(unsigned char abyte);
	void            addBytes(const void *data, int len);
	uint32_t        getCrc(); 
	void            test();

protected:
	uint32_t crc_;
private:
	class ARTD_API_JLIB_UTIL Impl;
	Impl &impl();
};

#ifdef DO_CRC_TEST

class ARTD_API_JLIB_UTIL CrcTestReference
{
public:

	void initCrc32()
	{
		// init values for CRC-32:
		order = 32;
		polynom = 0x4c11db7;
		direct = true;
		crcinit = 0xffffffff;
		crcxor = 0xffffffff;
		refin = true;
		refout = true;
		configure(true);
	}
	void initCCITTCrc16()
	{
		order   = 16;
		polynom = 0x1021;
		direct  = false; 
		// direct = true;
		crcinit = 0xFFFF;
		crcxor  = 0;
		refin   = false;
		refout  = false;
		configure(true);
	}

	CrcTestReference();

	void            reset();
	unsigned long   crctablefast(unsigned char* p, unsigned long len);
	unsigned long   crctable(unsigned char* p, unsigned long len);
	unsigned long   crcbitbybit(unsigned char* p, unsigned long len); 
	unsigned long   crcbitbybitfast(unsigned char* p, unsigned long len);
	unsigned long   getCrc() { return(crc_); }
	void            test();

	/** 'order' [1..32] is the CRC polynom order, counted without the leading '1' bit
	 * 'polynom' is the CRC polynom without leading '1' bit
	 * 'crcinit' is the initial CRC value belonging to that algorithm
	 * 'crcxor' is the final XOR value
	 * 'direct' [false,true] specifies the kind of algorithm: true=direct, no augmented zero bits
	 * 'refin' [false,true] specifies if a data byte is reflected before processing (UART) or not
	 * 'refout' [false,true] specifies if the CRC will be reflected before XOR
	 */

	int             order;
	unsigned long   polynom;
	unsigned long   crcinit;
	unsigned long   crcxor;
	bool            direct;
	bool            refin;
	bool            refout;
	bool            tableBuilt_;

protected:

	int configure(bool buildTable);
	static unsigned long reflect(unsigned long crc, int numbits);
	void generate_crc_table();

	// internal values:

	unsigned long crc_;
	unsigned long crcmask;
	unsigned long crchighbit;
	unsigned long crcinit_direct;
	unsigned long crcinit_nondirect;
	unsigned long crctab[256];

};
#endif // DO_CRC_TEST

ARTD_END

#endif // __artd_Crc_h
