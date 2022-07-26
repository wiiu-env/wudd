#pragma once
extern "C" {

void aes_set_key(unsigned char *key);
void aes_decrypt(unsigned char *iv, unsigned char *inbuf, unsigned char *outbuf, unsigned long long len);
void aes_encrypt(unsigned char *iv, unsigned char *inbuf, unsigned char *outbuf, unsigned long long len);
};