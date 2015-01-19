#ifndef _BASE64_H_
#define _BASE64_H_

unsigned char *base64_encode(const unsigned char *str, int length,unsigned char *result,int *ret_length);
unsigned char *base64_decode(const unsigned char *str, int length,unsigned char *result,int *ret_length);
#endif
