
#if 1
#include <stdio.h>  // removeme
#include <assert.h> // removeme
#endif

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h> // htons

#include "wsb64enc.h"
#include "wssha1.h"
#include "wsc.h"

#ifndef ARDUINO
#define memcpy_P	memcpy
#define strncmp_P	strncmp
#define F(x)		x
#endif

#define KEYFIELD  F("Sec-WebSocket-Key: ")
#define RESPONSE  F("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ")
#define MAGICGUID F("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")
#define TRAILER   F("\r\n\r\n")
#define EMPTYLINE F("\r\n")

#if WS_PROTO_HEADER_MAX_INPUT != 12 || WS_PROTO_HEADER_MAX_OUTPUT != 8
#error invalid WS protocol constants
#endif

static char* handshake_response = NULL;

const char* ws_handshake (const char* line)
{
	if (strncmp_P(F(EMPTYLINE), line, sizeof EMPTYLINE - 1) == 0 && handshake_response)
	{
		// header terminated, response ready
		return handshake_response;
	}

	else if (strncmp_P(F(KEYFIELD), line, sizeof KEYFIELD - 1) == 0)
	{
		// build string to sha1+b64-encode
		const char* pline = (line += sizeof(KEYFIELD) - 1);
		while (*pline != '\r' && *pline != '\n' && *pline)
			pline++;
		size_t keylen = pline - line;
		char key [keylen + sizeof MAGICGUID];
		memcpy_P(key, line, keylen);
		memcpy_P(key + keylen, F(MAGICGUID), sizeof(MAGICGUID));

		// encode it
		if (!sha1init())
			return NULL;
		sha1update(key, strlen(key));
		char* hash = sha1result();
		size_t b64len = base64_enc_len(SHA1LENGTH);
		handshake_response = (char*)malloc(sizeof(RESPONSE) + b64len + sizeof(TRAILER));
		if (!handshake_response)
			return NULL;
		memcpy_P(handshake_response, F(RESPONSE), sizeof(RESPONSE) - 1);
		base64_encode(handshake_response + sizeof(RESPONSE) - 1, hash, SHA1LENGTH);
		memcpy_P(handshake_response + sizeof(RESPONSE) - 1 + b64len, F(TRAILER), sizeof(TRAILER));
		sha1deinit();

		return handshake_response;
	}

	return NULL;
}

static size_t ws_decode_userlen (const char* in, size_t in_size, size_t* protoskip)
{
	if (in_size >= 2)
	{
		char smalllen = in[1] & 0x7f;
		if (smalllen == 126)
		{
			if (in_size >= 4)
			{
				*protoskip = 4;
				return htons(*(uint16_t*)&in[2]);
			}
		}			
		else if (smalllen == 127)
		{
			if (in_size >= 8)
			{
				*protoskip = 8;
				return htonl(*(uint32_t*)&in[4]);
			}
		}
		else
		{
			*protoskip = 2;
			return smalllen;
		}
	}
	
	// not enough input
	*protoskip = 0;
	return 0;
}

size_t ws_decode_header (char* raw, size_t raw_size, char** real_data, char mask[WS_MASK_SIZE])
{
	size_t protoskip;
	size_t userlen = ws_decode_userlen(raw, raw_size, &protoskip);
	if (userlen == 0)
	{
		// not enough data, come back later
		return 0;
	}
	char final = raw[0] & 0x80;
	char masked = raw[1] & 0x80;

	if ((raw[0] & 0x0f) != 0x01 || !final)
	{
		// not a TXT frame, or !final, invalid
		*real_data = NULL;
		return 0;
	}

	if (raw_size < protoskip + (masked? WS_MASK_SIZE: 0))
	{
		// not enough data, come back later
		return 0;
	}

	if (masked)
	{
		memcpy(mask, &raw[protoskip], WS_MASK_SIZE);
		protoskip += WS_MASK_SIZE;
	}
	else
		memset(mask, 0, WS_MASK_SIZE);

	*real_data = raw + protoskip;

	return userlen;
}

void ws_unmask (char* masked, size_t len, size_t offs, const char mask[WS_MASK_SIZE])
{
	if (*(uint32_t*)mask)
		for (size_t i = 0; i < len; i++)
			masked[i] ^= mask[(i + offs) & 3];
}

size_t ws_decode_full_frame (char** raw, size_t raw_size)
{
	char* real_data;
	char mask[WS_MASK_SIZE];
	size_t user_len = ws_decode_header(*raw, raw_size, &real_data, mask);
	if (user_len == 0 || (user_len > raw_size - (real_data - *raw)))
		// not enough data, come back later with a grown buffer
		return 0;
	ws_unmask(real_data, 0, user_len, mask);
	*raw = real_data;
	return user_len;
}

char ws_out_preamble [WS_PROTO_HEADER_MAX_OUTPUT];

size_t ws_set_output_preamble (size_t outsize, char output_preamble[WS_PROTO_HEADER_MAX_OUTPUT])
{
	output_preamble[0] = 0x81; // final txt
	if (outsize <= 125)
	{
		output_preamble[1] = outsize;
		return 2;
	}
	if (outsize <= 65535)
	{
		output_preamble[1] = 126;
		*(uint16_t*)&output_preamble[2] = htons(outsize);
		return 4;
	}
	output_preamble[1] = 127;
	*(uint32_t*)&output_preamble[4] = htonl(outsize);
	return 8;
}

size_t ws_output_preamble_length (size_t outsize)
{
	return ws_set_output_preamble(outsize, ws_out_preamble);
}
