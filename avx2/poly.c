#include "poly.h"
#include "ntt.h"
#include "randombytes.h"
#include "fips202.h"
#include "crypto_stream.h"
#include "immintrin.h"
#include "sha/sha256_mb.h"

static const unsigned char nonce[8] = {0};

// 8 KB lookup table for permutations
const __m256i idx[256] = {
{0x800000008,0x800000008,0x800000008,0x800000008},
{0x800000000,0x800000008,0x800000008,0x800000008},
{0x800000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000008,0x800000008,0x800000008},
{0x800000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000008,0x800000008,0x800000008},
{0x200000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000002,0x800000008,0x800000008},
{0x800000003,0x800000008,0x800000008,0x800000008},
{0x300000000,0x800000008,0x800000008,0x800000008},
{0x300000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000003,0x800000008,0x800000008},
{0x300000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000003,0x800000008,0x800000008},
{0x200000001,0x800000003,0x800000008,0x800000008},
{0x100000000,0x300000002,0x800000008,0x800000008},
{0x800000004,0x800000008,0x800000008,0x800000008},
{0x400000000,0x800000008,0x800000008,0x800000008},
{0x400000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000004,0x800000008,0x800000008},
{0x400000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000004,0x800000008,0x800000008},
{0x200000001,0x800000004,0x800000008,0x800000008},
{0x100000000,0x400000002,0x800000008,0x800000008},
{0x400000003,0x800000008,0x800000008,0x800000008},
{0x300000000,0x800000004,0x800000008,0x800000008},
{0x300000001,0x800000004,0x800000008,0x800000008},
{0x100000000,0x400000003,0x800000008,0x800000008},
{0x300000002,0x800000004,0x800000008,0x800000008},
{0x200000000,0x400000003,0x800000008,0x800000008},
{0x200000001,0x400000003,0x800000008,0x800000008},
{0x100000000,0x300000002,0x800000004,0x800000008},
{0x800000005,0x800000008,0x800000008,0x800000008},
{0x500000000,0x800000008,0x800000008,0x800000008},
{0x500000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000005,0x800000008,0x800000008},
{0x500000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000005,0x800000008,0x800000008},
{0x200000001,0x800000005,0x800000008,0x800000008},
{0x100000000,0x500000002,0x800000008,0x800000008},
{0x500000003,0x800000008,0x800000008,0x800000008},
{0x300000000,0x800000005,0x800000008,0x800000008},
{0x300000001,0x800000005,0x800000008,0x800000008},
{0x100000000,0x500000003,0x800000008,0x800000008},
{0x300000002,0x800000005,0x800000008,0x800000008},
{0x200000000,0x500000003,0x800000008,0x800000008},
{0x200000001,0x500000003,0x800000008,0x800000008},
{0x100000000,0x300000002,0x800000005,0x800000008},
{0x500000004,0x800000008,0x800000008,0x800000008},
{0x400000000,0x800000005,0x800000008,0x800000008},
{0x400000001,0x800000005,0x800000008,0x800000008},
{0x100000000,0x500000004,0x800000008,0x800000008},
{0x400000002,0x800000005,0x800000008,0x800000008},
{0x200000000,0x500000004,0x800000008,0x800000008},
{0x200000001,0x500000004,0x800000008,0x800000008},
{0x100000000,0x400000002,0x800000005,0x800000008},
{0x400000003,0x800000005,0x800000008,0x800000008},
{0x300000000,0x500000004,0x800000008,0x800000008},
{0x300000001,0x500000004,0x800000008,0x800000008},
{0x100000000,0x400000003,0x800000005,0x800000008},
{0x300000002,0x500000004,0x800000008,0x800000008},
{0x200000000,0x400000003,0x800000005,0x800000008},
{0x200000001,0x400000003,0x800000005,0x800000008},
{0x100000000,0x300000002,0x500000004,0x800000008},
{0x800000006,0x800000008,0x800000008,0x800000008},
{0x600000000,0x800000008,0x800000008,0x800000008},
{0x600000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000006,0x800000008,0x800000008},
{0x600000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000006,0x800000008,0x800000008},
{0x200000001,0x800000006,0x800000008,0x800000008},
{0x100000000,0x600000002,0x800000008,0x800000008},
{0x600000003,0x800000008,0x800000008,0x800000008},
{0x300000000,0x800000006,0x800000008,0x800000008},
{0x300000001,0x800000006,0x800000008,0x800000008},
{0x100000000,0x600000003,0x800000008,0x800000008},
{0x300000002,0x800000006,0x800000008,0x800000008},
{0x200000000,0x600000003,0x800000008,0x800000008},
{0x200000001,0x600000003,0x800000008,0x800000008},
{0x100000000,0x300000002,0x800000006,0x800000008},
{0x600000004,0x800000008,0x800000008,0x800000008},
{0x400000000,0x800000006,0x800000008,0x800000008},
{0x400000001,0x800000006,0x800000008,0x800000008},
{0x100000000,0x600000004,0x800000008,0x800000008},
{0x400000002,0x800000006,0x800000008,0x800000008},
{0x200000000,0x600000004,0x800000008,0x800000008},
{0x200000001,0x600000004,0x800000008,0x800000008},
{0x100000000,0x400000002,0x800000006,0x800000008},
{0x400000003,0x800000006,0x800000008,0x800000008},
{0x300000000,0x600000004,0x800000008,0x800000008},
{0x300000001,0x600000004,0x800000008,0x800000008},
{0x100000000,0x400000003,0x800000006,0x800000008},
{0x300000002,0x600000004,0x800000008,0x800000008},
{0x200000000,0x400000003,0x800000006,0x800000008},
{0x200000001,0x400000003,0x800000006,0x800000008},
{0x100000000,0x300000002,0x600000004,0x800000008},
{0x600000005,0x800000008,0x800000008,0x800000008},
{0x500000000,0x800000006,0x800000008,0x800000008},
{0x500000001,0x800000006,0x800000008,0x800000008},
{0x100000000,0x600000005,0x800000008,0x800000008},
{0x500000002,0x800000006,0x800000008,0x800000008},
{0x200000000,0x600000005,0x800000008,0x800000008},
{0x200000001,0x600000005,0x800000008,0x800000008},
{0x100000000,0x500000002,0x800000006,0x800000008},
{0x500000003,0x800000006,0x800000008,0x800000008},
{0x300000000,0x600000005,0x800000008,0x800000008},
{0x300000001,0x600000005,0x800000008,0x800000008},
{0x100000000,0x500000003,0x800000006,0x800000008},
{0x300000002,0x600000005,0x800000008,0x800000008},
{0x200000000,0x500000003,0x800000006,0x800000008},
{0x200000001,0x500000003,0x800000006,0x800000008},
{0x100000000,0x300000002,0x600000005,0x800000008},
{0x500000004,0x800000006,0x800000008,0x800000008},
{0x400000000,0x600000005,0x800000008,0x800000008},
{0x400000001,0x600000005,0x800000008,0x800000008},
{0x100000000,0x500000004,0x800000006,0x800000008},
{0x400000002,0x600000005,0x800000008,0x800000008},
{0x200000000,0x500000004,0x800000006,0x800000008},
{0x200000001,0x500000004,0x800000006,0x800000008},
{0x100000000,0x400000002,0x600000005,0x800000008},
{0x400000003,0x600000005,0x800000008,0x800000008},
{0x300000000,0x500000004,0x800000006,0x800000008},
{0x300000001,0x500000004,0x800000006,0x800000008},
{0x100000000,0x400000003,0x600000005,0x800000008},
{0x300000002,0x500000004,0x800000006,0x800000008},
{0x200000000,0x400000003,0x600000005,0x800000008},
{0x200000001,0x400000003,0x600000005,0x800000008},
{0x100000000,0x300000002,0x500000004,0x800000006},
{0x800000007,0x800000008,0x800000008,0x800000008},
{0x700000000,0x800000008,0x800000008,0x800000008},
{0x700000001,0x800000008,0x800000008,0x800000008},
{0x100000000,0x800000007,0x800000008,0x800000008},
{0x700000002,0x800000008,0x800000008,0x800000008},
{0x200000000,0x800000007,0x800000008,0x800000008},
{0x200000001,0x800000007,0x800000008,0x800000008},
{0x100000000,0x700000002,0x800000008,0x800000008},
{0x700000003,0x800000008,0x800000008,0x800000008},
{0x300000000,0x800000007,0x800000008,0x800000008},
{0x300000001,0x800000007,0x800000008,0x800000008},
{0x100000000,0x700000003,0x800000008,0x800000008},
{0x300000002,0x800000007,0x800000008,0x800000008},
{0x200000000,0x700000003,0x800000008,0x800000008},
{0x200000001,0x700000003,0x800000008,0x800000008},
{0x100000000,0x300000002,0x800000007,0x800000008},
{0x700000004,0x800000008,0x800000008,0x800000008},
{0x400000000,0x800000007,0x800000008,0x800000008},
{0x400000001,0x800000007,0x800000008,0x800000008},
{0x100000000,0x700000004,0x800000008,0x800000008},
{0x400000002,0x800000007,0x800000008,0x800000008},
{0x200000000,0x700000004,0x800000008,0x800000008},
{0x200000001,0x700000004,0x800000008,0x800000008},
{0x100000000,0x400000002,0x800000007,0x800000008},
{0x400000003,0x800000007,0x800000008,0x800000008},
{0x300000000,0x700000004,0x800000008,0x800000008},
{0x300000001,0x700000004,0x800000008,0x800000008},
{0x100000000,0x400000003,0x800000007,0x800000008},
{0x300000002,0x700000004,0x800000008,0x800000008},
{0x200000000,0x400000003,0x800000007,0x800000008},
{0x200000001,0x400000003,0x800000007,0x800000008},
{0x100000000,0x300000002,0x700000004,0x800000008},
{0x700000005,0x800000008,0x800000008,0x800000008},
{0x500000000,0x800000007,0x800000008,0x800000008},
{0x500000001,0x800000007,0x800000008,0x800000008},
{0x100000000,0x700000005,0x800000008,0x800000008},
{0x500000002,0x800000007,0x800000008,0x800000008},
{0x200000000,0x700000005,0x800000008,0x800000008},
{0x200000001,0x700000005,0x800000008,0x800000008},
{0x100000000,0x500000002,0x800000007,0x800000008},
{0x500000003,0x800000007,0x800000008,0x800000008},
{0x300000000,0x700000005,0x800000008,0x800000008},
{0x300000001,0x700000005,0x800000008,0x800000008},
{0x100000000,0x500000003,0x800000007,0x800000008},
{0x300000002,0x700000005,0x800000008,0x800000008},
{0x200000000,0x500000003,0x800000007,0x800000008},
{0x200000001,0x500000003,0x800000007,0x800000008},
{0x100000000,0x300000002,0x700000005,0x800000008},
{0x500000004,0x800000007,0x800000008,0x800000008},
{0x400000000,0x700000005,0x800000008,0x800000008},
{0x400000001,0x700000005,0x800000008,0x800000008},
{0x100000000,0x500000004,0x800000007,0x800000008},
{0x400000002,0x700000005,0x800000008,0x800000008},
{0x200000000,0x500000004,0x800000007,0x800000008},
{0x200000001,0x500000004,0x800000007,0x800000008},
{0x100000000,0x400000002,0x700000005,0x800000008},
{0x400000003,0x700000005,0x800000008,0x800000008},
{0x300000000,0x500000004,0x800000007,0x800000008},
{0x300000001,0x500000004,0x800000007,0x800000008},
{0x100000000,0x400000003,0x700000005,0x800000008},
{0x300000002,0x500000004,0x800000007,0x800000008},
{0x200000000,0x400000003,0x700000005,0x800000008},
{0x200000001,0x400000003,0x700000005,0x800000008},
{0x100000000,0x300000002,0x500000004,0x800000007},
{0x700000006,0x800000008,0x800000008,0x800000008},
{0x600000000,0x800000007,0x800000008,0x800000008},
{0x600000001,0x800000007,0x800000008,0x800000008},
{0x100000000,0x700000006,0x800000008,0x800000008},
{0x600000002,0x800000007,0x800000008,0x800000008},
{0x200000000,0x700000006,0x800000008,0x800000008},
{0x200000001,0x700000006,0x800000008,0x800000008},
{0x100000000,0x600000002,0x800000007,0x800000008},
{0x600000003,0x800000007,0x800000008,0x800000008},
{0x300000000,0x700000006,0x800000008,0x800000008},
{0x300000001,0x700000006,0x800000008,0x800000008},
{0x100000000,0x600000003,0x800000007,0x800000008},
{0x300000002,0x700000006,0x800000008,0x800000008},
{0x200000000,0x600000003,0x800000007,0x800000008},
{0x200000001,0x600000003,0x800000007,0x800000008},
{0x100000000,0x300000002,0x700000006,0x800000008},
{0x600000004,0x800000007,0x800000008,0x800000008},
{0x400000000,0x700000006,0x800000008,0x800000008},
{0x400000001,0x700000006,0x800000008,0x800000008},
{0x100000000,0x600000004,0x800000007,0x800000008},
{0x400000002,0x700000006,0x800000008,0x800000008},
{0x200000000,0x600000004,0x800000007,0x800000008},
{0x200000001,0x600000004,0x800000007,0x800000008},
{0x100000000,0x400000002,0x700000006,0x800000008},
{0x400000003,0x700000006,0x800000008,0x800000008},
{0x300000000,0x600000004,0x800000007,0x800000008},
{0x300000001,0x600000004,0x800000007,0x800000008},
{0x100000000,0x400000003,0x700000006,0x800000008},
{0x300000002,0x600000004,0x800000007,0x800000008},
{0x200000000,0x400000003,0x700000006,0x800000008},
{0x200000001,0x400000003,0x700000006,0x800000008},
{0x100000000,0x300000002,0x600000004,0x800000007},
{0x600000005,0x800000007,0x800000008,0x800000008},
{0x500000000,0x700000006,0x800000008,0x800000008},
{0x500000001,0x700000006,0x800000008,0x800000008},
{0x100000000,0x600000005,0x800000007,0x800000008},
{0x500000002,0x700000006,0x800000008,0x800000008},
{0x200000000,0x600000005,0x800000007,0x800000008},
{0x200000001,0x600000005,0x800000007,0x800000008},
{0x100000000,0x500000002,0x700000006,0x800000008},
{0x500000003,0x700000006,0x800000008,0x800000008},
{0x300000000,0x600000005,0x800000007,0x800000008},
{0x300000001,0x600000005,0x800000007,0x800000008},
{0x100000000,0x500000003,0x700000006,0x800000008},
{0x300000002,0x600000005,0x800000007,0x800000008},
{0x200000000,0x500000003,0x700000006,0x800000008},
{0x200000001,0x500000003,0x700000006,0x800000008},
{0x100000000,0x300000002,0x600000005,0x800000007},
{0x500000004,0x700000006,0x800000008,0x800000008},
{0x400000000,0x600000005,0x800000007,0x800000008},
{0x400000001,0x600000005,0x800000007,0x800000008},
{0x100000000,0x500000004,0x700000006,0x800000008},
{0x400000002,0x600000005,0x800000007,0x800000008},
{0x200000000,0x500000004,0x700000006,0x800000008},
{0x200000001,0x500000004,0x700000006,0x800000008},
{0x100000000,0x400000002,0x600000005,0x800000007},
{0x400000003,0x600000005,0x800000007,0x800000008},
{0x300000000,0x500000004,0x700000006,0x800000008},
{0x300000001,0x500000004,0x700000006,0x800000008},
{0x100000000,0x400000003,0x600000005,0x800000007},
{0x300000002,0x500000004,0x700000006,0x800000008},
{0x200000000,0x400000003,0x600000005,0x800000007},
{0x200000001,0x400000003,0x600000005,0x800000007},
{0x100000000,0x300000002,0x500000004,0x700000006},

};


static uint16_t barrett_reduce(uint16_t a)
{
  uint32_t u;

  u = ((uint32_t) a * 5) >> 16;
  u *= PARAM_Q;
  a -= u;
  return a;
}

void poly_frombytes(poly *r, const unsigned char *a)
{
  int i;
  for(i=0;i<PARAM_N/4;i++)
  {
    r->coeffs[4*i+0] =                               a[7*i+0]        | (((uint16_t)a[7*i+1] & 0x3f) << 8);
    r->coeffs[4*i+1] = (a[7*i+1] >> 6) | (((uint16_t)a[7*i+2]) << 2) | (((uint16_t)a[7*i+3] & 0x0f) << 10);
    r->coeffs[4*i+2] = (a[7*i+3] >> 4) | (((uint16_t)a[7*i+4]) << 4) | (((uint16_t)a[7*i+5] & 0x03) << 12);
    r->coeffs[4*i+3] = (a[7*i+5] >> 2) | (((uint16_t)a[7*i+6]) << 6); 
  }
}

void poly_tobytes(unsigned char *r, const poly *p)
{
  int i;
  uint16_t t0,t1,t2,t3,m;
  int16_t c;
  for(i=0;i<PARAM_N/4;i++)
  {
    t0 = barrett_reduce(p->coeffs[4*i+0]); //Make sure that coefficients have only 14 bits
    t1 = barrett_reduce(p->coeffs[4*i+1]);
    t2 = barrett_reduce(p->coeffs[4*i+2]);
    t3 = barrett_reduce(p->coeffs[4*i+3]);

    m = t0 - PARAM_Q;
    c = m;
    c >>= 15;
    t0 = m ^ ((t0^m)&c); // <Make sure that coefficients are in [0,q]

    m = t1 - PARAM_Q;
    c = m;
    c >>= 15;
    t1 = m ^ ((t1^m)&c); // <Make sure that coefficients are in [0,q]

    m = t2 - PARAM_Q;
    c = m;
    c >>= 15;
    t2 = m ^ ((t2^m)&c); // <Make sure that coefficients are in [0,q]

    m = t3 - PARAM_Q;
    c = m;
    c >>= 15;
    t3 = m ^ ((t3^m)&c); // <Make sure that coefficients are in [0,q]

    r[7*i+0] =  t0 & 0xff;
    r[7*i+1] = (t0 >> 8) | (t1 << 6);
    r[7*i+2] = (t1 >> 2);
    r[7*i+3] = (t1 >> 10) | (t2 << 4);
    r[7*i+4] = (t2 >> 4);
    r[7*i+5] = (t2 >> 12) | (t3 << 2);
    r[7*i+6] = (t3 >> 6);
  }
}

void poly_uniform(poly *a, const unsigned char *seed)
{
  uint16_t moduli[5] = {0,PARAM_Q,2*PARAM_Q,3*PARAM_Q,4*PARAM_Q}; // multiples of q for lookup
  unsigned int pos=0, ctr=0;
  uint16_t val, r;

  uint32_t counter = 0;

  uint32_t nblocks=9; // 9 * 8 * 32 = 2304 > 2184 bytes that are needed on average
  uint32_t numStreams = 8;

  uint32_t SHA256_SIZE = 32;
  uint32_t SHA256_INP_SIZE = SHA256_SIZE + sizeof(counter);

  unsigned int SHA256_RATE = SHA256_SIZE * numStreams;
  uint8_t buf[SHA256_RATE*nblocks];

  uint8_t *input_buf;
  input_buf = (unsigned char*) _mm_malloc(SHA256_INP_SIZE*numStreams, ALIGNMENT_BYTES);

  uint16_t i, j = 0;
  for (i = 0; i < nblocks; i++) {
    for (j = 0; j < numStreams; j++, counter++) {
      memcpy(input_buf+(j*SHA256_INP_SIZE), seed, 32);
      memcpy(input_buf+(j*SHA256_INP_SIZE+SHA256_SIZE), &counter, sizeof(counter));
    }

    hash_keys_Simultaneous2((uint32_t *)(buf+i*SHA256_RATE), (uint32_t *)input_buf, numStreams);
  }

  const __m256i zero = _mm256_setzero_si256();
  const __m256i modulus8 = _mm256_set1_epi32(PARAM_Q);
  const __m256i modulus16 = _mm256_set1_epi16(PARAM_Q);


  uint32_t good = 0;
  uint32_t offset = 0;
  while(ctr < PARAM_N-16)
  {
    __m256i tmp0, tmp1, tmp2;

    tmp0 = _mm256_loadu_si256((__m256i *)&buf[pos]);

    // shift candidates into [0,q) range by
    // subtracting q up to four times
    tmp1 = _mm256_min_epu16(tmp0,modulus16);
    tmp1 = _mm256_cmpeq_epi16(tmp1,modulus16);
    tmp2 = _mm256_and_si256(tmp1, modulus16);
    tmp0 = _mm256_sub_epi16(tmp0, tmp2);
    tmp1 = _mm256_min_epu16(tmp0,modulus16);
    tmp1 = _mm256_cmpeq_epi16(tmp1,modulus16);
    tmp2 = _mm256_and_si256(tmp1, modulus16);
    tmp0 = _mm256_sub_epi16(tmp0, tmp2);
    tmp1 = _mm256_min_epu16(tmp0,modulus16);
    tmp1 = _mm256_cmpeq_epi16(tmp1,modulus16);
    tmp2 = _mm256_and_si256(tmp1, modulus16);
    tmp0 = _mm256_sub_epi16(tmp0, tmp2);
    tmp1 = _mm256_min_epu16(tmp0,modulus16);
    tmp1 = _mm256_cmpeq_epi16(tmp1,modulus16);
    tmp2 = _mm256_and_si256(tmp1, modulus16);
    tmp0 = _mm256_sub_epi16(tmp0, tmp2);


    tmp1 = _mm256_unpacklo_epi16(tmp0, zero); // transition to epi32
    tmp2 = _mm256_cmpgt_epi32(modulus8, tmp1); // compare to modulus
    good = _mm256_movemask_ps((__m256)tmp2);

    offset = __builtin_popcount(good); // we get this many good (< modulus) values

    // permute good values to lower bits using lookup table,
    // bad values are "skipped" by the previously computed mask
    // we don't care which values are at positions > offset,
    // because they are overwritten in the next iteration
    tmp2 = _mm256_permutevar8x32_epi32(tmp1, idx[good]);

    // ctr includes offset, possible bad values are overwritten
    _mm256_storeu_si256((__m256i *)&a->coeffs[ctr], tmp2);
    ctr += offset;


    // the very same thing as above, only with unpackhi
    tmp1 = _mm256_unpackhi_epi16(tmp0, zero); // transition to epi32
    tmp2 = _mm256_cmpgt_epi32(modulus8, tmp1); // compare to modulus
    good = _mm256_movemask_ps((__m256)tmp2);

    offset = __builtin_popcount(good); // we get this many good (< modulus) values

    // permute good values to lower bits using lookup table,
    // bad values are "skipped" by the previously computed mask
    // we don't care which values are at positions > offset,
    // because they are overwritten in the next iteration
    tmp2 = _mm256_permutevar8x32_epi32(tmp1, idx[good]);

    // ctr includes offset, possible bad values are overwritten
    _mm256_storeu_si256((__m256i *)&a->coeffs[ctr], tmp2);
    ctr += offset;

    pos += 32;

    if(pos > SHA256_RATE*nblocks-32)
    {
      nblocks=1;
      for (j = 0; j < numStreams; j++, counter++) {
        memcpy(input_buf+(j*SHA256_INP_SIZE), seed, 32);
        memcpy(input_buf+(j*SHA256_INP_SIZE+SHA256_SIZE), &counter, sizeof(counter));
      }
      hash_keys_Simultaneous2((uint32_t *)buf, (uint32_t *)input_buf, numStreams);

      pos = 0;
    }
  }

  // fill the last coefficients one-by-one
  while(ctr < PARAM_N)
  {
    val = (buf[pos] | ((uint16_t) buf[pos+1] << 8));

    pos += 2;

    r = val/PARAM_Q;
    if (r < 5) // q fits 5 times in 2^16, so accept if candidate < 5q
      a->coeffs[ctr++] = val - moduli[r]; // subtract q until in range [0,q)

    if(pos > SHA256_RATE*nblocks-2)
    {
      nblocks=1;
      for (j = 0; j < numStreams; j++, counter++) {
        memcpy(input_buf+(j*SHA256_INP_SIZE), seed, 32);
        memcpy(input_buf+(j*SHA256_INP_SIZE+SHA256_SIZE), &counter, sizeof(counter));
      }
      hash_keys_Simultaneous2((uint32_t *)buf, (uint32_t *)input_buf, numStreams);

      pos = 0;
    }
  }
}

extern void cbd(poly *r, unsigned char *b);

void poly_getnoise(poly *r, unsigned char *seed, unsigned char nonce)
{
#if PARAM_K != 16
#error "poly_getnoise in poly.c only supports k=16"
#endif
  unsigned char buf[4*PARAM_N];
  unsigned char n[CRYPTO_STREAM_NONCEBYTES];
  int i;

  for(i=1;i<CRYPTO_STREAM_NONCEBYTES;i++)
    n[i] = 0;
  n[0] = nonce;

  crypto_stream(buf,4*PARAM_N,n,seed);
  cbd(r,buf);
}

void poly_pointwise(poly *r, const poly *a, const poly *b)
{
  int i;
  for(i=0;i<PARAM_N;i++)
    r->coeffs[i] = a->coeffs[i] * b->coeffs[i] % PARAM_Q; /* XXX: Get rid of the % here! */
}

void poly_add(poly *r, const poly *a, const poly *b)
{
  int i;
  for(i=0;i<PARAM_N;i++)
    r->coeffs[i] = a->coeffs[i] + b->coeffs[i] % PARAM_Q; /* XXX: Get rid of the % here! */
}

void poly_ntt(poly *r)
{
  double temp[PARAM_N];

  pwmul_double(r->coeffs, psis_bitrev);
  ntt_double(r->coeffs,omegas_double,temp);
}

void poly_invntt(poly *r)
{
  double temp[PARAM_N];

  bitrev_vector(r->coeffs);
  ntt_double(r->coeffs, omegas_inv_double,temp);
  pwmul_double(r->coeffs, psis_inv);
}
