/** 
 * @author peterk
 *
 * $Id: PrimFormatter.cpp 3766 2009-01-26 18:18:09Z peterk $
 */
#include "artd/PrimFormatter.h"
#include <float.h>

ARTD_BEGIN

#ifdef __EMSCRIPTEN__
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif

static inline 
void reverseBytes(uint8_t *start,uint8_t *end)
{
	// makes assumption that "end" is placed one beyond last byte
	while((--end) > start)
	{
		uint8_t swapper = *start;
		*start++ = *end;
		*end = swapper;
	}
}
//int
//PrimFormatter::numberOfLeadingOnes(const void *start, int size)
//{
//    // little endian only
//    const uint8_t *p = ((const uint8_t *)start) + size;
//    uint8_t c;
//    int bits = 0;
//    while(p-- > start) {
//        c = *p;
//        if(c == 0xFF) {
//            bits += 8;
//            continue;
//        }
//        while(c & 0x80) {
//            ++bits;
//            c <<= 1;
//        }
//        break;
//    }
//    return(bits);
//}
//int 
//PrimFormatter::numberOfLeadingZeros(const void *start, int size)
//{
//    // little endian only
//    const uint8_t *p = ((const uint8_t *)start) + size;
//    uint8_t c;
//    int bits = 0;
//    while(p-- > start) {
//        c = *p;
//        if(c == 0) {
//            bits += 8;
//            continue;
//        }
//        c = ~c;
//        while(c & 0x80) {
//            ++bits;
//            c <<= 1;
//        }
//        break;
//    }
//    return(bits);
//}
int 
PrimFormatter::getIntSize64(int64_t i)
{
    if(i < 0) 
        return((8 + (((int)sizeof(i)) * 8) - Long::numberOfLeadingOnes(i)) / 8);
    else
		return((8+(((int)sizeof(i))*8)-Long::numberOfLeadingZeros(i)) / 8);
}
int 
PrimFormatter::getIntSize32(int32_t i)
{
    if(i < 0) 
		return((8+(((int)sizeof(i))*8)-Integer::numberOfLeadingOnes(i)) / 8);
    else
		return((8+(((int)sizeof(i))*8)-Integer::numberOfLeadingZeros(i)) / 8);
}

/*
    int size = 1;
    if(i < 0)
    {
        for(;;)
        {
            int64_t newi = i >> 8;
            if((newi == ~0) && (i & 0x80))
                break;
            i = newi;
            ++size;
        }
    }
    else
    {
        for(;;)
        {
            uint64_t newi = i >> 8;
            if((newi == 0) && (!(i & 0x80))) {
                break;
            }
            i = newi;
            ++size;
        }
    }
    return(size);
}
*/

unsigned char *
PrimFormatter::addIntBytes32(unsigned char *pout, int32_t val, int size)
{
    int ix = size;
    while(ix > 0)
    {
        pout[--ix] = (unsigned char)val;
        val >>= 8;
    }
    return(pout + size);

}

unsigned char*
PrimFormatter::addIntBytes64(unsigned char* pout, int64_t val, int size)
{
    if(std::endian::native == std::endian::little)
     {
        const uint8_t *pin = ((const uint8_t *)&val) + size;
        while(size > 0) {
            *pout++ = *--pin;
            --size;
        }
    } else {
        *((int64_t*)pout) = val;
        pout += sizeof(int64_t); // TODO: a;wats 8 ? assert here ?
    }
    return(pout);
}

unsigned char* PrimFormatter::add_int16(unsigned char* pout, int16_t val) {
    pout[1] = (unsigned char)val;
    val >>= 8;
    pout[0] = (unsigned char)val;
    return(&pout[2]);
}

unsigned char* PrimFormatter::add_int32(unsigned char* pout, int32_t val) {
    pout[3] = (unsigned char)val;
    val >>= 8;
    pout[2] = (unsigned char)val;
    return(&pout[2]);
    pout[1] = (unsigned char)val;
    val >>= 8;
    pout[0] = (unsigned char)val;
    return(&pout[4]);
}

unsigned char* PrimFormatter::add_int64(unsigned char* pout, int64_t val) {
    return(addIntBytes64(pout, val, sizeof(int64_t)));
}

int32_t 
PrimFormatter::extractInt(const void *pin, int size)
{
    const uint8_t *bytes = (const uint8_t *)pin;
    int32_t val = *(int8_t *)bytes;
    while(--size > 0) {
        val <<= 8;
		val |= *(++bytes);
	}
	return(val);
}
int64_t 
PrimFormatter::extractLong(const void* pin, int size)
{
    const uint8_t* bytes = (const uint8_t*)pin;

    int32_t valLOW = *(int8_t*)bytes;
    int32_t valHI = valLOW >> 31;

    while (--size > 0) {
        valHI <<= 8;
        valHI |= ((uint32_t)valLOW) >> 24;
        valLOW <<= 8;
        valLOW |= *(++bytes);
    }

    int32_t val[2];

    // TODO: template or have it choose to optimize out 
    if (std::endian::native == std::endian::little) {
        val[1] = valHI;
        val[0] = valLOW;
    } else {
        val[0] = valHI;
        val[1] = valLOW;
    }
	return(*(int64_t *)&val);
}
int 
PrimFormatter::getIntUSize64(uint64_t i)
{
    // note this was broken out because on PocketPC there is a sign extension
    // bug where it takes the sizeof() * 8 - 1 as an unsigned
    int difbits = (((int)(sizeof(i)*8))-1)-Number::numberOfLeadingZeros(i);
    difbits /= 8;
    difbits += 1;
    return(difbits);
}
int 
PrimFormatter::getIntUSize32(uint32_t i)
{
    int size = 1;
    for(;;)
    {
        if(!(i >>= 8))
            break;
        ++size;
    }
    return(size);
}
unsigned char *
PrimFormatter::addIntUBytes(unsigned char *pout, uint64_t val, int size)
{
    // little endian only!
    const uint8_t *pin = ((const uint8_t *)&val) + size;
    while(size > 0) {
        *pout++ = *--pin;
        --size;
    }
    return(pout);
}
uint32_t 
PrimFormatter::extractIntU(const void *pin, int size)
{
    const uint8_t *bytes = (const uint8_t *)pin;
    uint32_t val = *bytes;
    while(--size > 0) {
        val <<= 8;
		val |= *(++bytes);
	}
	return(val);
}
uint64_t 
PrimFormatter::extractLongU(const void *pin, int size)
{
    const uint8_t *bytes = (const uint8_t *)pin;
    uint64_t val = *bytes;
    while(--size > 0) {
        val <<= 8;
		val |= *(++bytes);
	}
	return(val);
}
int 
PrimFormatter::getVIntSize64(int64_t i)
{
    if(i < 0) 
        return((7+(((int)sizeof(i))*8)-Number::numberOfLeadingOnes(i)) / 7);
    else
        return((7+(((int)sizeof(i))*8)-Number::numberOfLeadingZeros(i)) / 7);
}
int 
PrimFormatter::getVIntSize32(int32_t i)
{
    int size = 1;
    if(i < 0)
    {
        for(;;)
        {
            int nexti = i >> 7;
            if((i & 0x40) && (nexti == ~0)) {
                break;
            }
            i = nexti;
            ++size;
        }
    }
    else
    {
        for(;;)
        {
            int nexti = i >> 7;
            if( (!(i & 0x40)) && (nexti == 0)) {
                break;
            }
            i = nexti;
            ++size;
        }
    }
    return(size);
}
int 
PrimFormatter::getVIntUSize64(uint64_t i)
{
    int ones = ( ((((int)(sizeof(i)) * 8)-1)-Number::numberOfLeadingZeros(i)) );
    return((ones/7)+1);
}
int 
PrimFormatter::getVIntUSize32(uint32_t i)
{
    int size = 1;
    for(;;)
    {
        if(!(i >>= 7))
            break;
        ++size;
    }
    return(size);
}
unsigned char *
PrimFormatter::addVIntBytes32(unsigned char *pout, int val)
{
	unsigned char *p = pout;
    const int done = val >> ((sizeof(val)*8)-1);
    
    for(;;)
    {
        *p++ = ((uint8_t)val) | 0x80;
        val >>= 7;
        if(val == done) // high bits left are all the same
        { 
            if((val & 0x40) == (p[-1] & 0x40)) { // if last high bit is same as sign bit
                break;
            }
        }
    }
    *pout &= ~0x080; // clear continuation bit on "last" (curently first) byte   
    reverseBytes(pout,p);
    return(p);
}
unsigned char *
PrimFormatter::addVIntBytes64(unsigned char *pout, int64_t val)
{
	unsigned char *p = pout;
    const int64_t done = val >> ((sizeof(val)*8)-1);
    
    for(;;)
    {
        *p++ = ((uint8_t)val) | 0x80;
        val >>= 7;
        if(val == done) // high bits left are all the same
        { 
            if((val & 0x40) == (p[-1] & 0x40)) { // if last high bit is same as sign bit
                break;
            }
        }
    }
    *pout &= ~0x080; // clear continuation bit on "last" (curently first) byte   
    reverseBytes(pout,p);
    return(p);
}
unsigned char *
PrimFormatter::addVIntUBytes64(unsigned char *pout, uint64_t val)
{
	unsigned char *p = pout;
   
    uint32_t hi = ((uint32_t *)&val)[1];
    uint32_t lo = ((uint32_t *)&val)[0];

    for(;;)
    {
        *p++ = (uint8_t)(lo | 0x80);
        lo >>= 7;
        if(hi) {
            lo |= ((hi & 0x7f) << 25);
            hi >>= 7;
            continue;
        }
        if(lo == 0) {
            break;
        }
    }
    *pout &= ~0x80; // clear continuation bit on "last" (curently first) byte
    reverseBytes(pout,p);
    return(p);
}
unsigned char *
PrimFormatter::addVIntUBytes32(unsigned char *pout, uint32_t val)
{
    unsigned char *p = pout;
    for(;;)
    {
        *p++ = (uint8_t)(val | 0x80);
        val >>= 7;
        if(val == 0) { // no high bits left
            break;
        }
    }
    *pout &= ~0x80; // clear continuation bit on "last" (curently first) byte
    reverseBytes(pout,p);
    return(p);
}
int64_t 
PrimFormatter::extractVInt(const void *pin_, const uint8_t **ppin)
{
    const unsigned char *pin = (const unsigned char *)pin_;
    // do first byte
	
    uint8_t b = *pin++;
    uint32_t hi = 0;
    if(b & 0x040)
        hi = ~0; // extend sign bit
    uint32_t lo = hi;
    for(;;)
    {
        lo <<= 7;  // here in case sign was extended
        lo |= (b & 0x7f);
        if(!(b & 0x80))
            break;
        hi <<= 7;
        hi |= (lo >> 25);
        b = *pin++;        
    }
    if(ppin)
        *ppin = pin;
    int32_t out[2];
    out[0] = lo;
    out[1] = hi;
    return(*(int64_t *)&out);
}
int32_t 
PrimFormatter::extractVInt32(const void *pin_, const uint8_t **ppin)
{
    const unsigned char *pin = (const unsigned char *)pin_;
    // do first byte
	uint8_t b;
    b = *pin++;
    int32_t out = 0;
    if(b & (uint8_t)0x040) {
        out = ((~0) << 7); // extend sign bit
	}
	for(;;) // continuation bit is on
    {
        out |= (b & (uint8_t)0x7f);
        if(!(b & (uint8_t)0x80))
            break;
        out <<= 7;
        b = *pin++;
    }
    if(ppin)
        *ppin = pin;
    return(out);
}
uint64_t 
PrimFormatter::extractVIntU(const void *pin_, const uint8_t **ppin)
{
    const unsigned char *pin = (const unsigned char *)pin_;
    uint32_t hi = 0;
    uint32_t lo = 0;
    for(;;)
    {
        uint8_t b = *pin++;        
        lo |= (b & 0x7f);
        if(!(b & 0x80))
            break;
        hi <<= 7;
        hi |= (lo >> 25);
        lo <<= 7;
    }
    if(ppin)
        *ppin = pin;
    int32_t out[2];
    out[0] = lo;
    out[1] = hi;
    return(*(uint64_t *)&out);
/*
    uint64_t out = 0;
	for(;;) // continuation bit is on
    {
        uint8_t b = *pin++;
        out |= (b & 0x7f);
        if(!(b & 0x80))
            break;
        out <<= 7;
    }
    return(out);
*/
}
// floating point types
unsigned char *
PrimFormatter::addFloatBytes(unsigned char *pout, double ieee64, int outsize)
{
    int32_t ieeeHigh32 = ((int32_t*)&ieee64)[1]; 
    int32_t ieeeLow32 = ((int32_t*)&ieee64)[0]; 

    uint32_t mant = (((ieeeHigh32 & 0x000FFFFF) << 12) | ((ieeeLow32 >> 20) & 0x00000FFF)); // & mantMaskHigh_;
    uint32_t mantLow = ((ieeeLow32 & 0x000FFFFF) << 12); //  & mantMaskLow_;

    // handle ieee64 special cases
    int exp = ((ieeeHigh32 & 0x7FF00000) >> 20); // still biased to unsigned value;
    int sign = (ieeeHigh32 >> 31);

    if(exp == 0x07FF) // special numbers
    {
        if(mant != 0 || mantLow != 0)
            exp = 1|(FPV_NAN<<1); // NAN
        else 
            exp = 1|(FPV_INFINITY<<1); // INFINITY
        exp |= (sign & (FPV_SIGN<<1));
        goto singleByte;
    }
    else if(exp == 0 && mant == 0 && mantLow == 0)
    {
        // zero FPV_ZERO is 0
        exp = 1 | (sign & (FPV_SIGN<<1)); 
        goto singleByte;
    } else {
	// if it is a low integer, put it in as a one byte integer
	{
		int iv = (int)ieee64;
		if(iv == ieee64 && iv <= 24 && iv >= -23)
		{
			exp = ((iv + 39) << 1) | 1;
		    	goto singleByte;
		}
		goto multiByte;
	}
   singleByte:
    	*pout++ = exp;
		// we are asking for a fixed length output bigger than one
		// fill remainder with zeros
		if(outsize > 1)
	{
		int size = outsize;
		while(--size > 0)
			*(pout++) = 0;
	}
	return(pout);
   }

multiByte:

	uint8_t *p = pout;

    {
        // regular float - unbias exponent, shift and add sign flag
        exp = ((exp-1023) << 2) | (sign & 2); // composite with sign        

        // if exponent is two or more bytes and we can fit 4 more bits into 
        // it without increasing it's size do so and flag "special"
        int size = getVIntSize32(exp);
        if((size > 1) && (size == getVIntSize32(exp << 4))) 
        {
          //  dbg("%08x + 4\n", asint32 );
            // shift over 4 clear old flags insert 4 mantissa bits
            // and re-insert flags
            exp = ((exp << 4) & (~0x03F)) | (1|(sign&2));                
            exp |= ((((uint32_t)(mant & 0xF0000000)) >> 26) & 0x03c);
            mant <<= 4;
            mant |= (((mantLow & 0xf0000000) >> 26) & 0x0f);
            mantLow <<= 4;
        }
        // add composite exponent word to output
        p = addVIntBytes(p,exp);
    }

    if(outsize >= 0) 
    {
    	// VARIABLE_FLOAT8 == 0
        // put non-zero mantissa bits in buffer starting from top

    	if(outsize > 0) // subtract exponent size
    		outsize -= (int)(p - pout);

        for(;;)
        {
            *p++ = ((uint32_t)(mant & 0xff000000)) >> 24; // & 0x0ff;
            mant <<= 8;
            if(mantLow) 
            {
                mant |= ((uint32_t)(mantLow & 0xff000000)) >> 24; // & 0xff;  
                mantLow <<= 8;
                if(outsize == 0) { // variable length
                	continue;
                }
            } 
            else if(outsize == 0) // variable length
            {
	            if(mant == 0) {
	                break;
                }
	            continue;	                
            } 
	        // fixed length, continue until length is filled.
            if(--outsize == 0) {
        		break;
            }
        }
    }
    else  // VARIABLE_FLOAT7 (< 0) currently unimplemented
    	return(pout); 

    /*        
    for(;;)
    {
        *p++ = (((uint32_t)(mant & 0xfe000000)) >> 25); // & 0x07F;
        mant <<= 7;
        if(mantLow) {
            mant |= (((uint32_t)(mantLow & 0xfe000000)) >> 25); // & 0x7f);  
            mantLow <<= 7;
            continue;
        }
        if(!mant)
            break;
        p[-1] |= 0x80; // continuation flag
    }
    */
    
    return(p);
}

#ifdef _MSC_VER
    #pragma warning(disable : 4056 4756) // float overflow in constant (for infinities)
#endif

static const int32_t positiveNAN = 0x7FFFFFFF; // positive NAN
static const int32_t negativeNAN = 0xFFFFFFFF; // negative NAN

double 
PrimFormatter::extractFloat(const void *pin_, const uint8_t **ppin, int size)
{    
    const uint8_t *bytes;
    int exp = extractVInt32(pin_,&bytes);
    int len = (int)(bytes - (uint8_t *)pin_);

    if(len == 1) // one byte exponent
    {
        if(exp & 1) // "special" value flag
        {
            exp = (exp >> 1) & 0x3f; // now a 6 bit unsigned value
            switch(exp)
            {                
                case FPV_ZERO:
                    return(0.0f);
                case FPV_NEGZERO:
                    return(-0.0f); 
                case FPV_NAN:  // nan
                    return(*(float *)&positiveNAN);
                case FPV_NEGNAN:  // -nan
                    return(*(float *)&negativeNAN);
                case FPV_INFINITY:  // infinity
                    return(FLT_MAX+FLT_MAX);
                case FPV_NEGINFINITY:  // -infinity
                    return(-(FLT_MAX+FLT_MAX));
                // 6 - 15 unused
                default: // low (biased) integer 
                    if(exp >= 0x10) {
                        //dbg("low integer %d->%d\n", exp, exp-39 );
                        return((float)(exp - 39));
                    }
                    return(0);
            }
        }
    }        

    {
        int shifter;
        uint32_t ieee64[2];
        
        #define ieeeHIGH32 (ieee64[1])
        #define ieeeLOW32 (ieee64[0])

        if(exp & 1) { // 4 mant bits in exponent
            shifter = 8;
            ieeeHIGH32 = ((exp & 2) << 30) | (((exp+(1023<<6))<<(18-4)) & 0x7fff0000); 
        } else {
            shifter = 12;
            ieeeHIGH32 = ((exp & 2) << 30) | (((exp+(1023<<2))<<18) & 0x7ff00000); 
        }

        ieeeLOW32 = 0; // clear low word
        size -= len;

        uint32_t *pmant = &ieeeHIGH32;
        
        while(size > 0)
        {
            uint8_t c = *bytes++;
            if(shifter < 0)
            {
                *pmant |= ((c & 0xff) >> -shifter);
                if(pmant == &ieeeLOW32)
                    break;
                pmant = &ieeeLOW32;
                shifter += 32;
            }
            *pmant |= c << shifter;
            shifter -= 8;
            --size;
        }

        #undef ieeeHIGH32
        #undef ieeeLOW32

        return(*(double *)ieee64);
    }
}   

    
    /*
        for(;;)
        {
            if(shifter >= 0)
            {
                *pmant |= ((c&0x7f) << shifter);
            } 
            else if(shifter != -99)
            {
                *pmant |= ((c & 0x7f) >> -shifter);
                if(pmant == &ieeeHIGH32)
                {                
                    pmant = &ieeeLOW32;
                    shifter += 32;
                    *pmant |= ((c & 0x7f) << shifter);
                } else {
                    shifter = -99;
                    goto checkContinuation;
                }
            }
            shifter -= 7;
        
        checkContinuation:
        
            if(!(c & 0x80))
                break;
        }
        

        #undef ieeeHIGH32
        #undef ieeeLOW32

        if(ppin)
            *ppin = pin;
        return(*(double *)ieee64);
    }
}   
*/
    /*
    int addFloatBytes(unsigned char *pout, float ieee32)
    {
        int32_t asint32 = *(int32_t*)&ieee32;
        
        int mant = ((asint32 & 0x007FFFFF) << 9);
        int exp = ((asint32 & 0x7F800000) >> 23); // currently biased
        int sign = (asint32 >> 31);

        if(exp == 0x0FF)   // ieee float special numbers
        {
            if(mant != 0)
                exp = 1|(FPV_NAN<<1); // NAN
            else 
                exp = 1|(FPV_INFINITY<<1); // INFINITY
            exp |= (sign & (FPV_SIGN<<1));
            goto singleByte;
        }
        if(exp == 0 && mant == 0)
        {
            // zero FPV_ZERO is 0
            exp = 1 | (sign & (FPV_SIGN<<1)); 
            goto singleByte;
        } 
        // if it is a low integer, put it in as a one byte integer
        int iv = (int)ieee32;
        if(iv == ieee32 && iv <= 24 && iv >= -23)
        {
            exp = ((iv + 39) << 1) | 1;
    singleByte:
            *pout = exp;
            return(1);
        }

        uint8_t *p;

        {
            // regular float - unbias exponent, shift and add sign flag
            exp = ((exp-127) << 2) | (sign & 2); // composite with sign        

            // if exponent is two or more bytes and we can fit 4 more bits into 
            // it without increasing it's size do so and flag "special"
            int size = PrimWriter::getVIntSize(exp);
            if((size > 1) && (size == PrimWriter::getVIntSize(exp << 4))) 
            {
              //  dbg("%08x + 4\n", asint32 );
                // shift over 4 clear old flags insert 4 mantissa bits
                // and re-insert flags
                exp = ((exp << 4) & (~0x03F)) | (1|(sign&2));                
                exp |= ((((uint32_t)(mant & 0xF0000000)) >> 26) & 0x03c);
                mant <<= 4;
            }
            // add composite exponent word to output
            p = PrimWriter::addVIntBytes(pout,exp);
        }

        // remaining add non-zero mantissa bits to output
        for(;;)
        {
            *p++ = (((unsigned int)(mant & 0xFF000000)) >> 24) & 0x0FF;
            mant <<= 8;
            if(!mant)  // no more bits left
                break;
        }
        return((int)(p - pout));
    }
    //*/
    /*  
    float 
    extractFloat(const void *pin_,int size)
    {
        static const int32_t positiveNAN = 0x7FFFFFFF; // positive NAN
        static const int32_t negativeNAN = 0xFFFFFFFF; // negative NAN
     
        if(size < 1) {
            return(*(float *)&positiveNAN);    
        }

        uint8_t *bytes = (uint8_t *)pin_;

        int len;
        int exp = PrimReader::extractVInt(bytes,&len);

        if(len == 1) // one byte exponent
        {
            if(exp & 1) // "special" value flag
            {
                exp = (exp >> 1) & 0x3f; // now a 6 bit unsigned value
                switch(exp)
                {                
                    case FPV_ZERO:
                        return(0.0f);
                    case FPV_NEGZERO:
                        return(-0.0f); 
                    case FPV_NAN:  // nan
                        return(*(float *)&positiveNAN);
                    case FPV_NEGNAN:  // -nan
                        return(*(float *)&negativeNAN);
                    case FPV_INFINITY:  // infinity
                        return(FLT_MAX+FLT_MAX);
                    case FPV_NEGINFINITY:  // -infinity
                        return(-(FLT_MAX+FLT_MAX));
                    // 6 - 15 unused
                    default: // low (biased) integer 
                        if(exp >= 0x10) {
                            //dbg("low integer %d->%d\n", exp, exp-39 );
                            return((float)(exp - 39));
                        }
                        return(0);
                }
            }
        }        

        uint32_t ieee32;
        int shifter;

        if(exp & 1) { // 4 mant bits in exponent
            shifter = 11;
            ieee32 = ((exp & 2) << 30) | (((exp+(127<<6))<<(21-4)) & 0x7ff80000); 
        } else {
            shifter = 15;
            ieee32 = ((exp & 2) << 30) | (((exp+(127<<2))<<21) & 0x7f800000); 
        }

        size -= len;
        bytes += len;

        while(size-- > 0)
        {
            uint8_t c = *bytes++;
            if(shifter >= 0) {
                ieee32 |= c << shifter;
            } else {
                ieee32 |= c >> -shifter;
                break;
            }
            shifter -= 8;        
        }

        return(*(float *)&ieee32);
    }   
//*/



ARTD_END
