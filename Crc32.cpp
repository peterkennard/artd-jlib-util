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
#include "artd/Crc.h"
#include "CrcImpl.h"
#include "artd/pointer_math.h"

ARTD_BEGIN

#define INL ARTD_ALWAYS_INLINE

class Crc32::Impl
	: public CRCImpl<uint32_t, Crc32::Impl>
{
public:

	static const char *getName() { return("CRC-32"); }
	enum { 
		POLYNOMIAL			= 0x04C11DB7,
		INITIAL_REMAINDER	= 0xFFFFFFFF,
		FINAL_XOR_VALUE		= 0xFFFFFFFF,
		REFLECT_DATA		= true,
		REFLECT_REMAINDER	= true,
		CHECK_VALUE			= 0xCBF43926
	};
};

INL Crc32::Impl &Crc32::impl()
{
	return(*(reinterpret_cast<Impl*>(this)));
}



Crc32::Crc32()
{
	new(this) Crc32::Impl();
}
void 
Crc32::reset()
{
	impl().reset();
}
void
Crc32::addByte(unsigned char b)
{
	impl().addByte(b);
}
void
Crc32::addBytes(const void *data, int len)
{
	impl().addBytes(data,len);
}
uint32_t
Crc32::getCrc()
{
	return(impl().getCrc());    
}
void 
Crc32::test()
{
	class Errck {
		protected: char c[(sizeof(Impl) != sizeof(Crc32)) ? -1 : 1];
	};
	impl().test();
}

ARTD_END
