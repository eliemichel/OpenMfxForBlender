/* this is taken from alShaders/Cryptomatte/MurmurHash3.h:
 *
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code.
 *
 */
#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
	return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
	return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32_t getblock32 ( const uint32_t * p, int i )
{
	return p[i];
}

FORCE_INLINE uint64_t getblock64 ( const uint64_t * p, int i )
{
	return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	
	return h;
}

//----------

FORCE_INLINE uint64_t fmix64 ( uint64_t k )
{
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;
	
	return k;
}

//-----------------------------------------------------------------------------

static void MurmurHash3_x86_32 ( const void * key, int len,
						 uint32_t seed, void * out )
{
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 4;
	
	uint32_t h1 = seed;
	
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	
	//----------
	// body
	
	const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);
	
	for(int i = -nblocks; i; i++)
	{
		uint32_t k1 = getblock32(blocks,i);
		
		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;
		
		h1 ^= k1;
		h1 = ROTL32(h1,13);
		h1 = h1*5+0xe6546b64;
	}
	
	//----------
	// tail
	
	const uint8_t * tail = (const uint8_t*)(data + nblocks*4);
	
	uint32_t k1 = 0;
	
	switch(len & 3)
	{
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
			k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};
	
	//----------
	// finalization
	
	h1 ^= len;
	
	h1 = fmix32(h1);
	
	*(uint32_t*)out = h1;
}

/* this is taken from the cryptomatte specification 1.0 */

static inline float hash_to_float(uint32_t hash) {
	uint32_t mantissa = hash & (( 1 << 23) - 1);
	uint32_t exponent = (hash >> 23) & ((1 << 8) - 1);
	exponent = std::max(exponent, (uint32_t) 1);
	exponent = std::min(exponent, (uint32_t) 254);
	exponent = exponent << 23;
	uint32_t sign = (hash >> 31);
	sign = sign << 31;
	uint32_t float_bits = sign | exponent | mantissa;
	float f;
	std::memcpy(&f, &float_bits, 4);
	return f;
}
