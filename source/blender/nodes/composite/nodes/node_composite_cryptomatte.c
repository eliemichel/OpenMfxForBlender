/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2016 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Lukas Stockner, Stefan Werner
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/nodes/composite/nodes/node_composite_cryptomatte.c
 *  \ingroup cmpnodes
 */

#include "node_composite_util.h"
#include "BLI_dynstr.h"

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

/* Other compilers */

#else	/* defined(_MSC_VER) */

#define	FORCE_INLINE inline __attribute__((always_inline))

static inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
	return (x << r) | (x >> (32 - r));
}

static inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
	return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif /* !defined(_MSC_VER) */

/* Block read - if your platform needs to do endian-swapping or can only
 * handle aligned reads, do the conversion here
 */

FORCE_INLINE uint32_t getblock32 ( const uint32_t * p, int i )
{
	return p[i];
}

FORCE_INLINE uint64_t getblock64 ( const uint64_t * p, int i )
{
	return p[i];
}

/* Finalization mix - force all bits of a hash block to avalanche */

FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	
	return h;
}

FORCE_INLINE uint64_t fmix64 ( uint64_t k )
{
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;
	
	return k;
}

static void MurmurHash3_x86_32 ( const void * key, int len,
						 uint32_t seed, void * out )
{
	const uint8_t * data = (const uint8_t*)key;
	const int nblocks = len / 4;
	
	uint32_t h1 = seed;
	
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	
	/* body */
	
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
	
	/* tail */
	
	const uint8_t * tail = (const uint8_t*)(data + nblocks*4);
	
	uint32_t k1 = 0;
	
	switch(len & 3)
	{
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
			k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};
	
	/* finalization */
	
	h1 ^= len;
	
	h1 = fmix32(h1);
	
	*(uint32_t*)out = h1;
}

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* this is taken from the cryptomatte specification 1.0 */

static /*inline*/ float hash_to_float(uint32_t hash) {
	uint32_t mantissa = hash & (( 1 << 23) - 1);
	uint32_t exponent = (hash >> 23) & ((1 << 8) - 1);
	exponent = max(exponent, (uint32_t) 1);
	exponent = min(exponent, (uint32_t) 254);
	exponent = exponent << 23;
	uint32_t sign = (hash >> 31);
	sign = sign << 31;
	uint32_t float_bits = sign | exponent | mantissa;
	float f;
	memcpy(&f, &float_bits, 4);
	return f;
}

static void cryptomatte_add(NodeCryptomatte* n, float f)
{
	/* Turn the number into a string. */
	static char number[32];
	BLI_snprintf(number, sizeof(number), "<%.9g>", f);

	if(BLI_strnlen(n->matte_id, sizeof(n->matte_id)) == 0)
	{
		BLI_snprintf(n->matte_id, sizeof(n->matte_id), "%s", number);
		return;
	}

	/* Search if we already have the number. */
	size_t start = 0;
	const size_t end = strlen(n->matte_id);
	size_t token_len = 0;
	while(start < end) {
		/* Ignore leading whitespace. */
		while (start < end && n->matte_id[start] == ' ') {
			++start;
		}

		/* Find the next seprator. */
		char* token_end = strchr(n->matte_id+start, ',');
		if (token_end == NULL || token_end == n->matte_id+start) {
			token_end = n->matte_id+end;
		}
		/* Be aware that token_len still contains any trailing white space. */
		token_len = token_end - (n->matte_id + start);

		/* If this has a leading bracket, assume a raw floating point number and look for the closing bracket. */
		if (n->matte_id[start] == '<') {
			if (strncmp(n->matte_id+start, number, strlen(number)) == 0) {
				/* This number is already there, so continue. */
				return;
			}
		}
		else {
			/* Remove trailing white space */
			size_t name_len = token_len;
			while (n->matte_id[start+name_len] == ' ' && name_len > 0) {
				name_len--;
			}
			/* Calculate the hash of the token and compare. */
			uint32_t hash = 0;
			MurmurHash3_x86_32(n->matte_id+start, name_len, 0, &hash);
			if (f == hash_to_float(hash)) {
				return;
			}
		}
		start += token_len+1;
	}
	char *temp_str;
	temp_str = BLI_strdup(n->matte_id);
	BLI_snprintf(n->matte_id, sizeof(n->matte_id), "%s,%s", temp_str, number);
	MEM_freeN(temp_str);
}

static void cryptomatte_remove(NodeCryptomatte*n, float f)
{
	if(strnlen(n->matte_id, sizeof(n->matte_id)) == 0)
	{
		/* Empty string, nothing to remove. */
		return;
	}

	/* This will be the new srting without the removed key. */
	DynStr *new_matte = BLI_dynstr_new();
	if (!new_matte) {
		return;
	}

	/* Turn the number into a string. */
	static char number[32];
	BLI_snprintf(number, sizeof(number), "<%.9g>", f);

	/* Search if we already have the number. */
	size_t start = 0;
	const size_t end = strlen(n->matte_id);
	size_t token_len = 0;
	while (start < end) {
		bool skip = false;
		/* Ignore leading whitespace. */
		while(start < end && n->matte_id[start] == ' ') {
			++start;
		}

		/* Find the next seprator. */
		char* token_end = strchr(n->matte_id+start, ',');
		if (token_end == NULL || token_end == n->matte_id+start) {
			token_end = n->matte_id+end;
		}
		/* Be aware that token_len still contains any trailing white space. */
		token_len = token_end - (n->matte_id + start);

		/* If this has a leading bracket, assume a raw floating point number and look for the closing bracket. */
		if (n->matte_id[start] == '<') {
			if(strncmp(n->matte_id+start, number, strlen(number)) == 0) {
				/* This number is already there, so skip it. */
				skip = true;
			}
		}
		else {
			/* Remove trailing white space */
			size_t name_len = token_len;
			while (n->matte_id[start+name_len] == ' ' && name_len > 0) {
				name_len--;
			}
			/* Calculate the hash of the token and compare. */
			uint32_t hash = 0;
			MurmurHash3_x86_32(n->matte_id+start, name_len, 0, &hash);
			if (f == hash_to_float(hash)) {
				skip = true;
			}
		}
		if (!skip) {
			BLI_dynstr_nappend(new_matte, n->matte_id+start, token_len+1);
		}
		start += token_len+1;
	}
	
	BLI_strncpy(n->matte_id, BLI_dynstr_get_cstring(new_matte), sizeof(n->matte_id));
	BLI_dynstr_free(new_matte);
}

static bNodeSocketTemplate outputs[] = {
	{	SOCK_RGBA,	0, N_("Image")},
	{	SOCK_FLOAT, 0, N_("Matte")},
	{	SOCK_RGBA,	0, N_("CryptoPick")},
	{	-1, 0, ""	}
};

void ntreeCompositCryptomatteSyncFromAdd(bNodeTree *UNUSED(ntree), bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if(n->add[0] != 0.0f) {
		cryptomatte_add(n, n->add[0]);
		n->add[0] = 0.0f;
		n->add[1] = 0.0f;
		n->add[2] = 0.0f;
	}
}

void ntreeCompositCryptomatteSyncFromRemove(bNodeTree *UNUSED(ntree), bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if(n->remove[0] != 0.0f) {
		cryptomatte_remove(n, n->remove[0]);
		n->remove[0] = 0.0f;
		n->remove[1] = 0.0f;
		n->remove[2] = 0.0f;
	}
}

bNodeSocket *ntreeCompositCryptomatteAddSocket(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *n = node->storage;
	char sockname[32];
	n->num_inputs++;
	BLI_snprintf(sockname, sizeof(sockname), "Pass %d", n->num_inputs);
	bNodeSocket *sock = nodeAddStaticSocket(ntree, node, SOCK_IN, SOCK_RGBA, PROP_NONE, NULL, sockname);
	return sock;
}

int ntreeCompositCryptomatteRemoveSocket(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if (n->num_inputs < 2) {
		return 0;
	}
	bNodeSocket *sock = node->inputs.last;
	nodeRemoveSocket(ntree, node, sock);
	n->num_inputs--;
	return 1;
}

static void init(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *user = MEM_callocN(sizeof(NodeCryptomatte), "cryptomatte user");
	node->storage = user;


	nodeAddStaticSocket(ntree, node, SOCK_IN, SOCK_RGBA, PROP_NONE, "image", "Image");

	/* add two inputs by default */
	ntreeCompositCryptomatteAddSocket(ntree, node);
	ntreeCompositCryptomatteAddSocket(ntree, node);
}

void register_node_type_cmp_cryptomatte(void)
{
	static bNodeType ntype;

	cmp_node_type_base(&ntype, CMP_NODE_CRYPTOMATTE, "Cryptomatte", NODE_CLASS_CONVERTOR, 0);
	node_type_socket_templates(&ntype, NULL, outputs);
	node_type_init(&ntype, init);
	node_type_storage(&ntype, "NodeCryptomatte", node_free_standard_storage, node_copy_standard_storage);
	nodeRegisterType(&ntype);
}
