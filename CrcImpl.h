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

#ifndef __artd_CrtImpl_h
#define __artd_CrtImpl_h

#include "artd/jlib_util.h"

// internal implementation header

#define TEST_CRCS 0
	
#ifdef TEST_CRCS
	#include <string.h>
#endif

ARTD_BEGIN

/*
 *                          CRC-CCITT          CRC-16                     CRC-32
 * Checksum Width            16 bits           16 bits                    32 bits
 * Generator Polynomial 10001000000100001 11000000000000101  100000100110000010001110110110111
 */


/*********************************************************************
 *
 * Function:    reflect()
 * 
 * Description: Reorder the bits of a binary sequence, by reflecting
 *				them about the middle position.
 *
 * Notes:		No checking is done that nBits <= 32.
 *
 * Returns:		The reflection of the original data.
 *
 *********************************************************************/

template<int BITWIDTH>
static uint32_t reflect(uint32_t data)
{
	uint32_t reflection = 0;
	uint32_t bit = 1 << (BITWIDTH-1);

	// reflect data about center bit
	while(bit)
	{
		// If the LSB bit is set, set the reflection of it.
		if (data & 0x01) {
			reflection |= bit;
		}
		data >>= 1;
		bit >>= 1;
	}
	return(reflection);
}

#define INL ARTD_ALWAYS_INLINE

template<class CRCType, class _SubT>
class CRCImpl
{
public:

	enum {
		BITWIDTH  = (8 * sizeof(CRCType)),
		TOPBIT = (1 << (BITWIDTH - 1))
	};

	static CRCType *crcTable()
	{
		static CRCType table[0x100+1];
		return(&table[1]);
	}
	static void initTable(void)
	{
		CRCType *ptable = crcTable();
		if(ptable[-1]) {
			return;
		}
		ptable[-1] = ~0; // initialized

		CRCType		    remainder;
		int			    dividend;
		uint8_t         bit;

		// Compute the remainder of each possible dividend.
		for (dividend = 0; dividend < 0x100; ++dividend)
		{
			//Start with the dividend followed by zeros.
			remainder = dividend << (BITWIDTH - 8);
			// Perform modulo-2 division, a bit at a time.
			for (bit = 8; bit > 0; --bit)
			{
				// Try to divide the current data bit.			
				if (remainder & _SubT::TOPBIT) {
					remainder = (remainder << 1) ^ _SubT::POLYNOMIAL;
				} else {
					remainder = (remainder << 1);
				}
			}
			// Store the result into the table.
			ptable[dividend] = remainder;
		}
	}
	void 
	addByte(uint8_t b)
	{
		const CRCType * const table = crcTable();
		CRCType	remainder = crc_;
		uint8_t data;

		// this should optimize out to theproper selection
		if(_SubT::REFLECT_DATA) {
			data = ((uint8_t)reflect<8>((uint8_t)b)) ^ (remainder >> (BITWIDTH - 8));
		} else {
			data = ((uint8_t)b) ^ (remainder >> (BITWIDTH - 8));                    
		}
		remainder = table[data] ^ (remainder << 8);
		crc_ = remainder;
	}
	void
	addBytes(const void *pdata, int nBytes)
	{
		const uint8_t *p = (const uint8_t *)pdata;
		const CRCType * const table = crcTable();
		CRCType	remainder = crc_; 
		uint8_t         data;

		// Divide the message by the polynomial, a byte at a time.
		while(nBytes > 0)
		{
			--nBytes;
			// this should optimize out to theproper selection
			if(_SubT::REFLECT_DATA) {
				data = ((uint8_t)reflect<8>(*p++)) ^ (remainder >> (BITWIDTH - 8));
			} else {
				data = (*p++) ^ (remainder >> (BITWIDTH - 8));                    
			}
			remainder = table[data] ^ (remainder << 8);
		}
		crc_ = remainder;
		return;
	} 


#ifdef TEST_CRCS
	
	INL void
	addBytesSlow(const void *pdata, int nBytes)
	{
		CRCType  remainder = crc_; // _SubT::INITIAL_REMAINDER;
		const uint8_t *p = (const uint8_t *)pdata;

		// Perform modulo-2 division, a byte at a time.
		while(nBytes > 0)
		{
			--nBytes;
			/*
			 * Bring the next byte into the remainder.
			 */
			if(_SubT::REFLECT_DATA) {
				remainder ^= (((uint8_t)reflect<8>(*p++)) << (BITWIDTH - 8));
			} else {
				remainder ^= (*p++) << (BITWIDTH - 8);
			}

			// Perform modulo-2 division, a bit at a time.
			for(int bit = 8; bit > 0; --bit)
			{
				// Try to divide the current data bit.
				if (remainder & TOPBIT) {
					remainder = (remainder << 1) ^ _SubT::POLYNOMIAL;
				} else {
					remainder = (remainder << 1);
				}
			}
		}
		crc_ = remainder;
		return;
	}
#endif 

	INL CRCImpl()
	{
		initTable();
		reset();
	}
	CRCType crc_;

	INL void reset()
	{
		crc_ = _SubT::INITIAL_REMAINDER;
	}
	CRCType getCrc()
	{
		// Return final remainder is the CRC.
		CRCType remainder = crc_;
		if(_SubT::REFLECT_REMAINDER) {
			remainder = reflect<BITWIDTH>(remainder);
		}
		return(remainder ^ _SubT::FINAL_XOR_VALUE);
	}
	
#ifdef TEST_CRCS
	void test()
	{
		reset();
		unsigned char test[] = "123456789";
		//Print the check value for the selected CRC algorithm.
//	    AD_LOG(debug) << "The check value for the %s standard is 0x%X\n", _SubT::getName(), _SubT::CHECK_VALUE);
		// Compute the CRC of the test message, slowly.
		reset();
		addBytes(test, (int)strlen((char *)test));
//		AD_LOG(debug) << "The addBytes() of \"123456789\" is 0x%X\n", getCrc());
		// Compute the CRC of the test message, more efficiently.
		reset();
		addBytesSlow(test, (int)strlen((char *)test));
//		AD_LOG(debug) << "The addBytesSlow() of \"123456789\" is 0x%X\n", getCrc());
	}
#else
	void test() {}
#endif

};

#undef INL

ARTD_END

#endif // __artd_CrcImpl_h
