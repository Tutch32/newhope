#include <string.h>

#ifndef AES_H
#define AES_H

#if !defined (ALIGN16)
#if defined (__GNUC__)
#  define ALIGN16  __attribute__  ( (aligned (16)))
# else
#  define ALIGN16 __declspec (align (16))
# endif
#endif

typedef struct KEY_SCHEDULE
{
   ALIGN16 unsigned char KEY[16*15];
   unsigned int nr;
} AES_KEY; 



int AES_set_encrypt_key (const unsigned char *userKey, 
                         const int bits, AES_KEY *key);
int AES_set_decrypt_key (const unsigned char *userKey, 
                         const int bits, AES_KEY *key);

void AES_ECB_encrypt(const unsigned char *in, 
                     unsigned char *out,
                     unsigned long length,
                     unsigned char *key,
                     int nr);
void AES_ECB_decrypt(const unsigned char *in, 
                     unsigned char *out,
                     unsigned long length,
                     unsigned char *key,
                     int nr);

#endif

