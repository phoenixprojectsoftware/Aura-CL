/*********************************************************************
* Filename:   sha256.h
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding SHA1 implementation.
*********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SHA256_H
#define SHA256_H

	/*************************** HEADER FILES ***************************/
#include <stddef.h>

/****************************** MACROS ******************************/
#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
#include <stdint.h>

	typedef uint8_t  SHA256_BYTE;
	typedef uint32_t SHA256_WORD;

	typedef struct
	{
		SHA256_BYTE data[64];
		SHA256_WORD datalen;
		uint64_t bitlen;
		SHA256_WORD state[8];
	} SHA256_CTX;

	/*********************** FUNCTION DECLARATIONS **********************/
	void sha256_init(SHA256_CTX* ctx);
	void sha256_update(
		SHA256_CTX* ctx,
		const SHA256_BYTE data[],
		size_t len
	);

	void sha256_final(
		SHA256_CTX* ctx,
		SHA256_BYTE hash[]
	);

#ifdef __cplusplus
}
#endif

#endif   // SHA256_H
