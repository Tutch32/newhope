#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wmmintrin.h>

#include <stdlib.h>
#include <byteswap.h>
#include <immintrin.h>

#ifdef AVX3
#define N 16
#elif defined AVX2
#define N 8
#else
#define N 4
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 64
#endif


#ifndef ALIGNMENT_BYTES
#define ALIGNMENT_BYTES 16
#endif


#if !defined (ALIGN32)
# if defined (__GNUC__)
#  define ALIGN32  __attribute__  ( (aligned (64)))
# else
#  define ALIGN32 __declspec (align (64))
# endif
#endif

#if !defined (ALIGN64)
# if defined (__GNUC__)
#  define ALIGN64  __attribute__  ( (aligned (64)))
# else
#  define ALIGN64 __declspec (align (64))
# endif
#endif

typedef struct
{
   uint8_t *buff1;
   uint8_t *buff2;
   uint8_t *buff3;
   uint8_t *buff4;
} SHA_buffers;

typedef struct
{
   int len1;
   int len2;
   int len3;
   int len4;
} SHA_lengths;


typedef struct
{
   uint32_t *h1;
   uint32_t *h2;
   uint32_t *h3;
   uint32_t *h4;
#if (defined AVX2) || (defined AVX3)
   uint32_t *h5;
   uint32_t *h6;
   uint32_t *h7;
   uint32_t *h8;
#endif
#ifdef AVX3
   uint32_t *h9;
   uint32_t *h10;
   uint32_t *h11;
   uint32_t *h12;
   uint32_t *h13;
   uint32_t *h14;
   uint32_t *h15;
   uint32_t *h16;
#endif
} SHA256_hashs;

uint64_t hash_keys_Simultaneous2(uint32_t *hashlist, uint32_t *buffers, int n);
void SHA256_simultaneous_update(uint32_t *hashlist, uint8_t **buffers, int n);