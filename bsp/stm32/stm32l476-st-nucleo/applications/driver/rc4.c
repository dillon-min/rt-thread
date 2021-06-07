////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  WjCryptLib_RC4
//
//  An implementation of RC4 stream cipher
//
//  This is free and unencumbered software released into the public domain - June 2013 waterjuice.org
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  IMPORTS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FW_VERSION
#include <rtthread.h>
#endif
#include <string.h>
#include "rc4.h"
#include <stdlib.h>
static uint8_t Key[256] = {0};
static uint32_t KeySize = 256;
static uint8_t map[10][10] = {
	{1, 4, 23, 87, 21, 65, 99, 124, 11, 176},
	{9, 43, 12, 230, 55, 134, 28, 7, 21, 3},
	{22, 90, 48, 51, 29, 77, 110, 83, 23, 1},
	{80, 5, 129, 210, 4, 7, 40, 97, 99, 21},
	{76, 89, 21, 109, 211, 237, 28, 86, 18, 34},
	{32, 11, 49, 51, 89, 23, 117, 221, 1, 3},
	{77, 21, 1, 0, 32, 28, 98, 110, 5, 112},
	{41, 147, 66, 53, 80, 119, 222, 9, 32, 39},
	{0, 6, 2, 1, 44, 55, 87, 199, 121, 10},
	{53, 99, 123, 45, 221, 32, 90, 11, 112, 8}
};
static const uint8_t Key_def[256] = {14, 87, 236, 178, 113, 18, 254, 80, 143, 20, 174, 58, 145, 185, 207, 186, 229, 1, 220, 51, 2, 200, 152, 141, 30, 41, 76, 248, 99, 173, 180, 233, 110, 22, 49, 129, 54, 159, 97, 12, 98, 239, 160, 126, 231, 103, 255, 202, 228, 171, 52, 57, 137, 64, 147, 211, 153, 63, 117, 7, 68, 6, 36, 140, 166, 46, 94, 142, 213, 43, 193, 245, 124, 101, 78, 37, 89, 45, 29, 190, 69, 168, 219, 151, 83, 134, 28, 192, 214, 127, 71, 21, 157, 208, 3, 144, 146, 108, 120, 187, 35, 93, 251, 47, 138, 155, 183, 34, 121, 139, 161, 100, 221, 66, 158, 205, 156, 204, 162, 149, 109, 241, 25, 114, 4, 115, 85, 253, 90, 39, 249, 201, 61, 111, 67, 122, 209, 9, 237, 10, 130, 217, 11, 31, 62, 206, 65, 235, 15, 135, 223, 163, 118, 74, 194, 203, 56, 8, 136, 119, 150, 255, 224, 112, 125, 24, 106, 197, 104, 88, 218, 226, 44, 60, 123, 91, 84, 234, 96, 131, 92, 195, 165, 32, 242, 238, 132, 232, 77, 167, 53, 33, 177, 42, 246, 212, 244, 55, 164, 81, 176, 225, 189, 79, 240, 26, 27, 13, 250, 38, 107, 215, 184, 48, 154, 230, 17, 40, 252, 210, 102, 128, 170, 179, 133, 175, 50, 82, 222, 116, 196, 16, 243, 73, 59, 172, 198, 182, 199, 72, 169, 105, 216, 19, 247, 75, 86, 95, 148, 23, 188, 5, 181, 70, 227, 191};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  INTERNAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SwapBytes( Value1, Value2 )                 \
{                                                   \
	uint8_t temp = Value1;                          \
	Value1 = Value2;                                \
	Value2 = temp;                                  \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Initialise
//
//  Initialises an RC4 cipher and discards the specified number of first bytes.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
	Rc4Initialise
(
 Rc4Context*     Context,        // [out]
 void const*     Key,            // [in]
 uint32_t        KeySize,        // [in]
 uint32_t        DropN           // [in]
 )
{
	uint32_t        i;
	uint32_t        j;
	uint32_t        n;

	// Setup key schedule
	for( i=0; i<256; i++ )
	{
		Context->S[i] = (uint8_t)i;
	}

	j = 0;
	for( i=0; i<256; i++ )
	{
		j = ( j + Context->S[i] + ((uint8_t*)Key)[i % KeySize] ) % 256;
		SwapBytes( Context->S[i], Context->S[j] );
	}

	i = 0;
	j = 0;

	// Drop first bytes (if requested)
	for( n=0; n<DropN; n++ )
	{
		i = ( i + 1 ) % 256;
		j = ( j + Context->S[i] ) % 256;
		SwapBytes( Context->S[i], Context->S[j] );
	}

	Context->i = i;
	Context->j = j;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Output
//
//  Outputs the requested number of bytes from the RC4 stream
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
	Rc4Output
(
 Rc4Context*     Context,        // [in out]
 void*           Buffer,         // [out]
 uint32_t        Size            // [in]
 )
{
	uint32_t    n;

	for( n=0; n<Size; n++ )
	{
		Context->i = ( Context->i + 1 ) % 256;
		Context->j = ( Context->j + Context->S[Context->i] ) % 256;
		SwapBytes( Context->S[Context->i], Context->S[Context->j] );

		((uint8_t*)Buffer)[n] = Context->S[ (Context->S[Context->i] + Context->S[Context->j]) % 256 ];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4Xor
//
//  XORs the RC4 stream with an input buffer and puts the results in an output buffer. This is used for encrypting
//  and decrypting data. InBuffer and OutBuffer can point to the same location for inplace encrypting/decrypting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
	Rc4Xor
(
 Rc4Context*     Context,        // [in out]
 void const*     InBuffer,       // [in]
 void*           OutBuffer,      // [out]
 uint32_t        Size            // [in]
 )
{
	uint32_t    n;

	for( n=0; n<Size; n++ )
	{
		Context->i = ( Context->i + 1 ) % 256;
		Context->j = ( Context->j + Context->S[Context->i] ) % 256;
		SwapBytes( Context->S[Context->i], Context->S[Context->j] );

		((uint8_t*)OutBuffer)[n] = ((uint8_t*)InBuffer)[n]
			^ ( Context->S[ (Context->S[Context->i] + Context->S[Context->j]) % 256 ] );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Rc4XorWithKey
//
//  This function combines Rc4Initialise and Rc4Xor. This is suitable when encrypting/decrypting data in one go with a
//  key that is not going to be reused.
//  InBuffer and OutBuffer can point to the same location for inplace encrypting/decrypting
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
	rc4
(
 void const*         InBuffer,               // [in]
 void*               OutBuffer,              // [out]
 uint32_t            BufferSize              // [in]
 )
{
	Rc4Context      context;

	Rc4Initialise( &context, Key, KeySize, 0);
	Rc4Xor( &context, InBuffer, OutBuffer, BufferSize );
}
/* msg_id 2 bytes
 * ts 8 bytes
 */
void set_rc4_key(uint8_t ofs, uint16_t msg_id, uint8_t *ts)
{
	/* ofs >= 0 && ofs <= 9 */
	uint8_t index[10] = {0x00};
	if (ofs >= 10)
		return;
	memcpy(Key, (uint8_t *)Key_def, 256);
	memcpy(index, map[ofs], 10);
	
	Key[index[0]] = (msg_id >> 8) & 0xff;
	Key[index[1]] = msg_id & 0xff;
	Key[index[2]] = ts[0];
	Key[index[3]] = ts[1];
	Key[index[4]] = ts[2];
	Key[index[5]] = ts[3];
	Key[index[6]] = ts[4];
	Key[index[7]] = ts[5];
	Key[index[8]] = ts[6];
	Key[index[9]] = ts[7];
}
