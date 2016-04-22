#include "sha256_mb.h"

void bswap_hash(uint32_t hash[8]) {
	hash[0] = bswap_32(hash[0]);
	hash[1] = bswap_32(hash[1]);
	hash[2] = bswap_32(hash[2]);
	hash[3] = bswap_32(hash[3]);
	hash[4] = bswap_32(hash[4]);
	hash[5] = bswap_32(hash[5]);
	hash[6] = bswap_32(hash[6]);
	hash[7] = bswap_32(hash[7]);
}

inline void sha256_parallel_hash_extract_bswap(uint32_t* ret_hash,
		uint32_t* hashs, int idx, int step) {
	ret_hash[0] = bswap_32(hashs[idx + 0 * step]);
	ret_hash[1] = bswap_32(hashs[idx + 1 * step]);
	ret_hash[2] = bswap_32(hashs[idx + 2 * step]);
	ret_hash[3] = bswap_32(hashs[idx + 3 * step]);
	ret_hash[4] = bswap_32(hashs[idx + 4 * step]);
	ret_hash[5] = bswap_32(hashs[idx + 5 * step]);
	ret_hash[6] = bswap_32(hashs[idx + 6 * step]);
	ret_hash[7] = bswap_32(hashs[idx + 7 * step]);
}

void sha256_simultaneous_init(uint32_t hs[8 * N]) {
#ifdef AVX3
	__m512i *h_ptr = (__m512i*)hs;
	h_ptr[0] = _mm512_set1_epi32(0x6a09e667);
	h_ptr[1] = _mm512_set1_epi32(0xbb67ae85);
	h_ptr[2] = _mm512_set1_epi32(0x3c6ef372);
	h_ptr[3] = _mm512_set1_epi32(0xa54ff53a);
	h_ptr[4] = _mm512_set1_epi32(0x510e527f);
	h_ptr[5] = _mm512_set1_epi32(0x9b05688c);
	h_ptr[6] = _mm512_set1_epi32(0x1f83d9ab);
	h_ptr[7] = _mm512_set1_epi32(0x5be0cd19);
#elif defined AVX2
	__m256i *h_ptr = (__m256i*)hs;
	h_ptr[0] = _mm256_set1_epi32(0x6a09e667);
	h_ptr[1] = _mm256_set1_epi32(0xbb67ae85);
	h_ptr[2] = _mm256_set1_epi32(0x3c6ef372);
	h_ptr[3] = _mm256_set1_epi32(0xa54ff53a);
	h_ptr[4] = _mm256_set1_epi32(0x510e527f);
	h_ptr[5] = _mm256_set1_epi32(0x9b05688c);
	h_ptr[6] = _mm256_set1_epi32(0x1f83d9ab);
	h_ptr[7] = _mm256_set1_epi32(0x5be0cd19);
#else
	__m128i *h_ptr = (__m128i *) hs;
	h_ptr[0] = _mm_set_epi32(0x6a09e667, 0x6a09e667, 0x6a09e667, 0x6a09e667);
	h_ptr[1] = _mm_set_epi32(0xbb67ae85, 0xbb67ae85, 0xbb67ae85, 0xbb67ae85);
	h_ptr[2] = _mm_set_epi32(0x3c6ef372, 0x3c6ef372, 0x3c6ef372, 0x3c6ef372);
	h_ptr[3] = _mm_set_epi32(0xa54ff53a, 0xa54ff53a, 0xa54ff53a, 0xa54ff53a);
	h_ptr[4] = _mm_set_epi32(0x510e527f, 0x510e527f, 0x510e527f, 0x510e527f);
	h_ptr[5] = _mm_set_epi32(0x9b05688c, 0x9b05688c, 0x9b05688c, 0x9b05688c);
	h_ptr[6] = _mm_set_epi32(0x1f83d9ab, 0x1f83d9ab, 0x1f83d9ab, 0x1f83d9ab);
	h_ptr[7] = _mm_set_epi32(0x5be0cd19, 0x5be0cd19, 0x5be0cd19, 0x5be0cd19);
#endif
}


uint64_t hash_keys_Simultaneous2(uint32_t *hashlist, uint32_t *buffers, int n) {
int i;
uint64_t total_bytes_processed = 0;

//   uint8_t last_block[64];
ALIGN32 uint32_t hs[8 * N] = { 0 };
ALIGN32 uint32_t buffsPadded[2 * N * 8] = { 0 };
uint8_t* buffs[N];
SHA256_hashs hashs = { 0 };

//   sha256_prepare_last_block(last_block, NULL, PAGE_SIZE);
uint64_t pad[4] = { 0 };
pad[0] = 0x00000080;
pad[3] = bswap_64(256);
for (i = 0; i < N; i++) {
	buffs[i] = (uint8_t*)&buffsPadded[2 * i * 8];
	memcpy(buffs[i] + 32, pad, 32);
}
//   sha256_prepare_last_blocks(buffers, PAGE_SIZE/2);

for (i = 0; i < (n & ~(N - 1)); i += N)
{

	hashs.h1 = &hashlist[i * 8];
	hashs.h2 = &hashlist[(i + 1) * 8];
	hashs.h3 = &hashlist[(i + 2) * 8];
	hashs.h4 = &hashlist[(i + 3) * 8];
#if (defined AVX2) || (defined AVX3)
	hashs.h5 = &hashlist[(i+4) * 8];
	hashs.h6 = &hashlist[(i+5) * 8];
	hashs.h7 = &hashlist[(i+6) * 8];
	hashs.h8 = &hashlist[(i+7) * 8];
#endif
#ifdef AVX3
	hashs.h9 = &hashlist[(i+8) * 8];
	hashs.h10 = &hashlist[(i+9) * 8];
	hashs.h11 = &hashlist[(i+10) * 8];
	hashs.h12 = &hashlist[(i+11) * 8];
	hashs.h13 = &hashlist[(i+12) * 8];
	hashs.h14 = &hashlist[(i+13) * 8];
	hashs.h15 = &hashlist[(i+14) * 8];
	hashs.h16 = &hashlist[(i+15) * 8];
#endif

	buffs[0] = (uint8_t*)&buffsPadded[2 * 0 * 8];
	buffs[1] = (uint8_t*)&buffsPadded[2 * 1 * 8];
	buffs[2] = (uint8_t*)&buffsPadded[2 * 2 * 8];
	buffs[3] = (uint8_t*)&buffsPadded[2 * 3 * 8];
#if (defined AVX2) || (defined AVX3)
	buffs[4]=(uint8_t*)&buffsPadded[2*4*8];
	buffs[5]=(uint8_t*)&buffsPadded[2*5*8];
	buffs[6]=(uint8_t*)&buffsPadded[2*6*8];
	buffs[7]=(uint8_t*)&buffsPadded[2*7*8];
#endif
#ifdef AVX3
	buffs[8]=(uint8_t*)&buffsPadded[2*8*8];
	buffs[9]=(uint8_t*)&buffsPadded[2*9*8];
	buffs[10]=(uint8_t*)&buffsPadded[2*10*8];
	buffs[11]=(uint8_t*)&buffsPadded[2*11*8];
	buffs[12]=(uint8_t*)&buffsPadded[2*12*8];
	buffs[13]=(uint8_t*)&buffsPadded[2*13*8];
	buffs[14]=(uint8_t*)&buffsPadded[2*14*8];
	buffs[15]=(uint8_t*)&buffsPadded[2*15*8];
#endif

	memcpy(buffs[0], &buffers[i * 8], 32);
	memcpy(buffs[1], &buffers[(i + 1) * 8], 32);
	memcpy(buffs[2], &buffers[(i + 2) * 8], 32);
	memcpy(buffs[3], &buffers[(i + 3) * 8], 32);
#if (defined AVX2) || (defined AVX3)
	memcpy(buffs[4],&buffers[(i+4)*8],32);
	memcpy(buffs[5],&buffers[(i+5)*8],32);
	memcpy(buffs[6],&buffers[(i+6)*8],32);
	memcpy(buffs[7],&buffers[(i+7)*8],32);
#endif
#ifdef AVX3
	memcpy(buffs[8],&buffers[(i+8)*8],32);
	memcpy(buffs[9],&buffers[(i+9)*8],32);
	memcpy(buffs[10],&buffers[(i+10)*8],32);
	memcpy(buffs[11],&buffers[(i+11)*8],32);
	memcpy(buffs[12],&buffers[(i+12)*8],32);
	memcpy(buffs[13],&buffers[(i+13)*8],32);
	memcpy(buffs[14],&buffers[(i+14)*8],32);
	memcpy(buffs[15],&buffers[(i+15)*8],32);
#endif

//      buffs[0] = buffers[i];
//      buffs[1] = buffers[i+1];
//      buffs[2] = buffers[i+2];
//      buffs[3] = buffers[i+3];
//#if (defined AVX2) || (defined AVX3)
//      buffs[4] = buffers[i+4];
//      buffs[5] = buffers[i+5];
//      buffs[6] = buffers[i+6];
//      buffs[7] = buffers[i+7];
//#endif
//#ifdef AVX3
//      buffs[8] = buffers[i+8];
//      buffs[9] = buffers[i+9];
//      buffs[10] = buffers[i+10];
//      buffs[11] = buffers[i+11];
//      buffs[12] = buffers[i+12];
//      buffs[13] = buffers[i+13];
//      buffs[14] = buffers[i+14];
//      buffs[15] = buffers[i+15];000000000000
//#endif

	sha256_simultaneous_init(hs);

//      print_hash(buffs[0]);

	SHA256_simultaneous_update(hs, buffs, PAGE_SIZE);

//      print_hash(buffs[0]);
//      print_hash(buffs[0]+32);
//      print_hash(hs);

//      SHA256_simultaneous_update(hs, last_ptr, 64);

	sha256_parallel_hash_extract_bswap(hashs.h1, hs, 0, N); // Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h2, hs, 1, N); // Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h3, hs, 2, N); // Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h4, hs, 3, N); // Extract the completed hash for that block
#if (defined AVX2) || (defined AVX3)
			sha256_parallel_hash_extract_bswap(hashs.h5, hs, 4, N); // Extract the completed hash for that block
			sha256_parallel_hash_extract_bswap(hashs.h6, hs, 5, N);// Extract the completed hash for that block
			sha256_parallel_hash_extract_bswap(hashs.h7, hs, 6, N);// Extract the completed hash for that block
			sha256_parallel_hash_extract_bswap(hashs.h8, hs, 7, N);// Extract the completed hash for that block
#endif
#ifdef AVX3
	sha256_parallel_hash_extract_bswap(hashs.h9, hs, 8, N); // Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h10, hs, 9, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h11, hs, 10, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h12, hs, 11, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h13, hs, 12, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h14, hs, 13, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h15, hs, 14, N);// Extract the completed hash for that block
	sha256_parallel_hash_extract_bswap(hashs.h16, hs, 15, N);// Extract the completed hash for that block
#endif

	total_bytes_processed += PAGE_SIZE / 2 * N;
}

return total_bytes_processed;
}