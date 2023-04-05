/*-
 * Copyright (c) 1991-2009 Peter Kennard and aRt&D Lab
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
 *  $Id$
 */

#ifndef __artd_PrimFormatter_h
#define __artd_PrimFormatter_h

#include <string.h> // memcpy
#include "artd/jlib_util.h"
#include "artd/int_types.h"
#include "artd/base_types.h"

ARTD_BEGIN

class ARTD_API_JLIB_UTIL PrimFormatter
{
public:

	PrimFormatter() {};
	~PrimFormatter() {};

	// ******* fixed byte length integer types *******

	// "stock" integer types
	
	static unsigned char *  add_int16(unsigned char *pout, int16_t val);
	static unsigned char *  add_int32(unsigned char *pout, int32_t val);
	static unsigned char *  add_int64(unsigned char *pout, int64_t val);

	static inline unsigned char *  add_uint16(unsigned char *pout, uint16_t val)
		{ return(add_int16(pout,val)); }
	static inline unsigned char *  add_uint32(unsigned char *pout, uint32_t val)
		{ return(add_int32(pout,val)); }
	static inline unsigned char *  add_uint64(unsigned char *pout, uint64_t val)
		{ return(add_int64(pout,val)); }
	
	static ARTD_ALWAYS_INLINE unsigned char *addBytes(unsigned char *out, const unsigned char *source, int length) {
		::memcpy(out, source, length);
		return(out + length);
	}
	// ******* arbitrary byte length (specified) ******
	
	// measure bytes needed to fint integer
	static int              getIntSize64(int64_t i);
	static int              getIntSize32(int32_t i);
	template<class T> 
	static int				getIntSize(const T &i);

	static int              getIntUSize64(uint64_t i);
	static int              getIntUSize32(uint32_t i);
	template<class T> 
	static int              getIntUSize(const T &i);

	static unsigned char *  addIntBytes32(unsigned char *pout, int32_t val, int size);
	static unsigned char *  addIntBytes64(unsigned char *pout, int64_t val, int size);
	template<class T> 
	static unsigned char *  addIntBytes(unsigned char *pout, const T &val, int size);
	template<class T> 
	static unsigned char *  addIntBytes(unsigned char *pout, const T &val);
	
	
	static int32_t          extractInt(const void *pin, int size);
	static int64_t          extractLong(const void *pin, int size);
	
	static unsigned char *  addIntUBytes(unsigned char *pout, uint64_t val, int size);
	template<class T> 
	static unsigned char *  addIntUBytes(unsigned char *pout, T val);

	static uint32_t         extractIntU(const void *pin, int size);
	static uint64_t         extractLongU(const void *pin, int size);
	
	// these nicely optimize out to not do the compare and call the 
	// appropriate function directly
	template<class T> 
	inline static const uint8_t*	extractIntU(const uint8_t *p, T &iu, int size)
	{
		if(sizeof(T) <= sizeof(uint32_t)) {
			iu = extractIntU(p,size);
			return(p + size);
		} else {
			iu = (T)extractLongU(p,size);
			return(p + size);
		}
	}

	template<class T> 
	inline static const uint8_t*	extractInt(const uint8_t *p, T &iu, int size)
	{
		// this should optimize out to not do the compare 
		if(sizeof(T) <= sizeof(uint32_t)) {
			iu = extractInt(p,size);
			return(p + size);
		} else {
			iu = (T)extractLong(p,size);
			return(p + size);
		}
	}

	// Variable Length Integer types using "continuation bit"
	// flag in bit 7 of each byte to indicate the next byte
	// is inclded in the current value.

	static int              getVIntUSize64(uint64_t i);
	static int              getVIntUSize32(uint32_t i);
	template<class T> 
	static int getVIntUSize(const T &i);

	static int              getVIntSize64(int64_t i);
	static int              getVIntSize32(int32_t i);    
	template<class T> 
	static int getVIntSize(const T &i);

	static unsigned char *  addVIntBytes32(unsigned char *pout, int val);
	static unsigned char *  addVIntBytes64(unsigned char *pout, int64_t val);
	template<class T> 
	static unsigned char *  addVIntBytes(unsigned char *pout, T val);
	
	/**
	 *  pin - input bytes
	 *  ppin - set to next byte after extraction if non-null
	 */
	static int64_t          extractVInt(const void *pin, const uint8_t **ppin);
	static int32_t          extractVInt32(const void *pin, const uint8_t **ppin);
	
	static unsigned char *  addVIntUBytes32(unsigned char *pout, uint32_t val);
	static unsigned char *  addVIntUBytes64(unsigned char *pout, uint64_t val);
	template<class T> 
	static unsigned char *  addVIntUBytes(unsigned char *pout, T val);
  
	/**
	 *  pin - input bytes
	 *  ppin - set to next byte after extraction if non-null
	 */
	static uint64_t         extractVIntU(const void *pin, const uint8_t **ppin);

	/**
	 * Add bytes to output representing a floating point value
	 * @param pout - where to put the bytes
	 * @param ieee64 - input floating point value
	 * @param outsize - if outsize == 0 then test mantissa to write 
	 *                  the most compact representation<br>
	 *                  if outsize > 0 then write a fixed size representation
	 *                  truncating if necessary. Note: the exponent will
	 *                  always be written and the result will not be less than
	 *                  the size of the exponent.<br>
	 *                  if outsize < 0 write mantissa as a VUint in
	 *                  7 bit increments. (unimplemented)
	 *                  
	 * @return - index of next available byte in out[]
	 */
	static unsigned char *  addFloatBytes(unsigned char *pout, double ieee64, int outsize=0);
	static double           extractFloat(const void *pin, const uint8_t **ppin, int size);

	/** utility functions for counting bits */
	static inline int		numberOfLeadingZeros(int anint) {
		return(Integer::numberOfLeadingZeros(anint)); 
	}
	
	static inline int		numberOfLeadingZeros(int64_t anint)
	{
		return(Long::numberOfLeadingZeros(anint));
	}

	static inline int		numberOfLeadingOnes(int anint)
		{ return(Long::numberOfLeadingOnes(anint)); }

protected:

	// floating point formats
	enum {
		FPV_ZERO = 0,
		FPV_SIGN = 1,
		FPV_NEGZERO = (FPV_ZERO | FPV_SIGN), // 1
		FPV_NAN = 2,
		FPV_NEGNAN = (FPV_NAN | FPV_SIGN), // 3
		FPV_INFINITY = 4,
		FPV_NEGINFINITY = (FPV_INFINITY | FPV_SIGN), // 5
	};

};

// **** getVIntSize **** //
template<>
inline int
PrimFormatter::getIntSize(const int64_t &i)
{
	return(getIntSize64(i));
}

template<>
inline int
PrimFormatter::getIntSize(const long &i)
{
	return(getIntSize64(i));
}

template<>
inline int
PrimFormatter::getIntSize(const unsigned long &i)
{
	return(getIntSize64(i));
}

template<>
inline int
PrimFormatter::getIntSize(const unsigned int &i)
{
	return(getIntSize32(i));
}

template<>
inline int
PrimFormatter::getIntSize(const int &i)
{
	return(getIntSize32(i));
}

template<>
inline int
PrimFormatter::getIntSize(const unsigned short &i)
{
	return(getIntSize32(i));
}

template<>
inline int
PrimFormatter::getIntSize(const short &i)
{
	return(getIntSize32(i));
}


// **** getIntUSize **** //
template<>
inline int
PrimFormatter::getIntUSize(const uint64_t &i)
{
	return(getIntUSize64(i));
}

template<>
inline int
PrimFormatter::getIntUSize(const unsigned long &i)
{
	return(getIntUSize64(i));
}

template<>
inline int
PrimFormatter::getIntUSize(const long &i)
{
	return(getIntUSize64(i));
}

template<> 
inline int 
PrimFormatter::getIntUSize(const int64_t &i) 
{ 
	return(getIntUSize64(i)); 
}

template<>
inline int
PrimFormatter::getIntUSize(const unsigned int &i)
{
	return(getIntUSize32(i));
}

template<>
inline int
PrimFormatter::getIntUSize(const int &i)
{
	return(getIntUSize32(i));
}

template<>
inline int
PrimFormatter::getIntUSize(const unsigned short &i)
{
	return(getIntUSize32(i));
}

template<>
inline int
PrimFormatter::getIntUSize(const short &i)
{
	return(getIntUSize32((int)i & 0x00FFFF));
}


// **** getVIntSize **** //
template<> 
inline int 
PrimFormatter::getVIntSize(const int64_t &i) 
{ 
	return(getVIntSize64(i)); 
}

template<> 
inline int
PrimFormatter::getVIntSize(const unsigned int &i) 
{ 
	return(getVIntSize32(i)); 
}

template<> 
inline int 
PrimFormatter::getVIntSize(const int &i) 
{ 
	return(getVIntSize32(i)); 
}

// **** getVIntUSize **** //
template<> 
inline int 
PrimFormatter::getVIntUSize(const uint64_t &i) 
{ 
	return(getVIntUSize64(i)); 
}

template<> 
inline int 
PrimFormatter::getVIntUSize(const int64_t &i) 
{ 
	return(getVIntUSize64(i)); 
}

template<>
inline int
PrimFormatter::getVIntUSize(const unsigned int &i)
{
	return(getVIntUSize32(i));
}

template<>
inline int
PrimFormatter::getVIntUSize(const int &i)
{
	return(getVIntUSize32(i));
}

template<>
inline int
PrimFormatter::getVIntUSize(const unsigned short &i)
{
	return(getVIntUSize32(i));
}

template<>
inline int
PrimFormatter::getVIntUSize(const short &i) {
	return(getVIntUSize32((int)i & 0x00FFFF));
}


/***** addIntBytes *****/
template<> 
inline unsigned char *  
PrimFormatter::addIntBytes(unsigned char *pout, const int &val, int size) {
	return(addIntBytes32(pout,val,size));
}
template<> 
inline unsigned char *  
PrimFormatter::addIntBytes(unsigned char *pout,const unsigned int &val, int size) {
	return(addIntBytes32(pout,val,size));
}
template<>
inline unsigned char *
PrimFormatter::addIntBytes(unsigned char *pout, const int64_t &val, int size) {
	return(addIntBytes64(pout, val, size));
}
template<>
inline unsigned char *
PrimFormatter::addIntBytes(unsigned char *pout, const uint64_t &val, int size) {
	return(addIntBytes64(pout, val, size));
}

template<>
inline unsigned char *
PrimFormatter::addIntBytes(unsigned char *pout, const long &val, int size) {
	return(addIntBytes64(pout, val, size));
}
template<>
inline unsigned char *
PrimFormatter::addIntBytes(unsigned char *pout, const unsigned long &val, int size) {
	return(addIntBytes64(pout, val, size));
}


template<class T> 
inline unsigned char *  
PrimFormatter::addIntBytes(unsigned char *pout, const T &val) {
	return(addIntBytes<T>(pout,val,getIntSize<T>(val)));
}
template<> 
inline unsigned char *  
PrimFormatter::addIntBytes(unsigned char *pout,const unsigned short &val, int size) {
	return(addIntBytes32(pout,val,size));
}
template<> 
inline unsigned char *  
PrimFormatter::addIntBytes(unsigned char *pout,const short &val, int size) {
	return(addIntBytes32(pout,val,size));
}


/***** addVIntBytes *****/

template<> 
inline unsigned char * 
PrimFormatter::addVIntBytes(unsigned char *pout, unsigned int val) {
	return(addVIntBytes32(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntBytes(unsigned char *pout, int val) {
	return(addVIntBytes32(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntBytes(unsigned char *pout, uint64_t val) {
	return(addVIntBytes64(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntBytes(unsigned char *pout, int64_t val) {
	return(addVIntBytes64(pout,val));
}

/***** addVIntUBytes *****/

template<> 
inline unsigned char * 
PrimFormatter::addVIntUBytes(unsigned char *pout, unsigned int val) {
	return(addVIntUBytes32(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntUBytes(unsigned char *pout, int val) {
	return(addVIntUBytes32(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntUBytes(unsigned char *pout, uint64_t val) {
	return(addVIntUBytes64(pout,val));
}

template<> 
inline unsigned char * 
PrimFormatter::addVIntUBytes(unsigned char *pout, int64_t val) {
	return(addVIntUBytes64(pout,val));
}

template<class T> 
inline unsigned char *  
PrimFormatter::addIntUBytes(unsigned char *pout, T val) {
	return(addIntUBytes(pout,val,getIntUSize<T>(val)));
}

ARTD_END

#endif // __artd_PrimFormatter_h
