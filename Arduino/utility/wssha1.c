
//XXX todo license: check this one from "public domain", need to dig to original source and statement
//XXX https://github.com/spotify/linux/blob/master/lib/sha1.c

// GPLv2 https://www.oryx-embedded.com/doc/sha1_8c_source.html/
/**
  * @file sha1.c
  * @brief SHA-1 (Secure Hash Algorithm 1)
  *
  * @section License
  *
  * Copyright (C) 2010-2018 Oryx Embedded SARL. All rights reserved.
  *
  * This file is part of CycloneCrypto Open.
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
  * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  *
  * @section Description
  *
  * SHA-1 is a secure hash algorithm for computing a condensed representation
  * of an electronic message. Refer to FIPS 180-4 for more details
  *
  * @author Oryx Embedded SARL (www.oryx-embedded.com)
  * @version 1.8.2
  **/

#include <stdint.h>
#include <stdlib.h>	// malloc free
#include <string.h>
#include <arpa/inet.h>	// ntohl htonl

#include "wssha1.h"

#define MIN(a,b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_b < _a)? _b: _a; })

// Macro to access the workspace as a circular buffer
#define W(t) w[(t) & 0x0F]

// SHA-1 auxiliary functions
#define CH(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define PARITY(x, y, z) ((x) ^ (y) ^ (z))
#define MAJ(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define ROL32(a, n) (((a) << (n)) | ((a) >> (32 - (n))))

// SHA-1 constants

#define K0 0x5A827999
#define K1 0x6ED9EBA1
#define K2 0x8F1BBCDC
#define K3 0xCA62C1D6

/**
 * @brief Digest a message using SHA-1
 * @param[in] data Pointer to the message being hashed
 * @param[in] length Length of the message
 * @param[out] digest Pointer to the calculated digest
 * @return Error code
 **/

// SHA-1 block size
#define SHA1_BLOCK_SIZE 64

typedef struct
{
	union
	{
		uint32_t h[SHA1LENGTH / 4];
		uint8_t digest[SHA1LENGTH];
	};
	union
	{
		uint32_t w[SHA1_BLOCK_SIZE / 4];
		uint8_t buffer[SHA1_BLOCK_SIZE];
	};
	size_t size;
	uint64_t totalSize;
} Sha1Context;

static Sha1Context* context = NULL;


/**
 * @brief Process message in 16-word blocks
 * @param[in] context Pointer to the SHA-1 context
 **/

static void sha1ProcessBlock(Sha1Context* context)
{
	unsigned int t;
	uint32_t temp;

	// Initialize the 5 working registers
	uint32_t a = context->h[0];
	uint32_t b = context->h[1];
	uint32_t c = context->h[2];
	uint32_t d = context->h[3];
	uint32_t e = context->h[4];

	// Process message in 16-word blocks
	uint32_t* w = context->w;

	// Convert from big-endian byte order to host byte order
	for (t = 0; t < 16; t++)
		w[t] = ntohl(w[t]);

	// SHA-1 hash computation (alternate method)
	for (t = 0; t < 80; t++)
	{
		// Prepare the message schedule
		if (t >= 16)
			W(t) = ROL32(W(t + 13) ^ W(t + 8) ^ W(t + 2) ^ W(t), 1);

		// Calculate T
		if (t < 20)
			temp = ROL32(a, 5) + CH(b, c, d) + e + W(t) + K0;
		else if (t < 40)
			temp = ROL32(a, 5) + PARITY(b, c, d) + e + W(t) + K1;
		else if (t < 60)
			temp = ROL32(a, 5) + MAJ(b, c, d) + e + W(t) + K2;
		else
			temp = ROL32(a, 5) + PARITY(b, c, d) + e + W(t) + K3;

		// Update the working registers
		e = d;
		d = c;
		c = ROL32(b, 30);
		b = a;
		a = temp;
	}

	// Update the hash value
	context->h[0] += a;
	context->h[1] += b;
	context->h[2] += c;
	context->h[3] += d;
	context->h[4] += e;
}

int sha1init(void)
{
	// Allocate a memory buffer to hold the SHA-1 context
	context = (Sha1Context*) malloc(sizeof(Sha1Context));
	// Failed to allocate memory?
	if (!context)
		return 0;

	// Set initial hash value
	context->h[0] = 0x67452301;
	context->h[1] = 0xEFCDAB89;
	context->h[2] = 0x98BADCFE;
	context->h[3] = 0x10325476;
	context->h[4] = 0xC3D2E1F0;

	// Number of bytes in the buffer
	context->size = 0;
	// Total length of the message
	context->totalSize = 0;

	return 1;
}


/**
 * @brief Update the SHA-1 context with a portion of the message being hashed
 * @param[in] context Pointer to the SHA-1 context
 * @param[in] data Pointer to the buffer being hashed
 * @param[in] length Length of the buffer
 **/

void sha1update(const void* data, size_t length)
{
	size_t n;

	// Process the incoming data
	while (length > 0)
	{
		// The buffer can hold at most 64 bytes
		n = MIN(length, 64 - context->size);

		// Copy the data to the buffer
		memcpy(context->buffer + context->size, data, n);

		// Update the SHA-1 context
		context->size += n;
		context->totalSize += n;
		// Advance the data pointer
		data = (uint8_t*) data + n;
		// Remaining bytes to process
		length -= n;

		// Process message in 16-word blocks
		if (context->size == 64)
		{
			// Transform the 16-word block
			sha1ProcessBlock(context);
			// Empty the buffer
			context->size = 0;
		}
	}
}

/**
 * @brief Finish the SHA-1 message digest
 * @param[in] context Pointer to the SHA-1 context
 * @param[out] digest Calculated digest (optional parameter)
 **/

void* sha1result()
{
	unsigned int i;
	size_t paddingSize;
	uint64_t totalSize;

	// Length of the original message (before padding)
	totalSize = context->totalSize * 8;

	// Pad the message so that its length is congruent to 56 modulo 64
	if (context->size < 56)
		paddingSize = 56 - context->size;
	else
		paddingSize = 64 + 56 - context->size;

	// Append padding
#ifdef ARDUINO
	// preserve stack
	if (paddingSize)
	{
		uint8_t c = 0x80;
		sha1update(&c, 1);
		c = 0;
		while (--paddingSize)
			sha1update(&c, 1);
	}
#else
	if (paddingSize)
	{
		uint8_t padding[paddingSize];
		memset(padding, 0, paddingSize);
		padding[0] = 0x80;
		sha1update(padding, paddingSize);
	}
#endif

	// Append the length of the original message
	context->w[14] = htonl((uint32_t)(totalSize >> 32));
	context->w[15] = htonl((uint32_t) totalSize);

	// Calculate the message digest
	sha1ProcessBlock(context);

	// Convert from host byte order to big-endian byte order
	for (i = 0; i < 5; i++)
		context->h[i] = htonl(context->h[i]);

	// Return the resulting digest
	return context->digest;
}


void sha1deinit(void)
{
	if (context)
	{
		free(context);
		context = NULL;
	}
}
