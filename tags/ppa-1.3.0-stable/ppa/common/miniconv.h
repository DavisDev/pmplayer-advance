#ifndef __MINICONV_H__
#define __MINICONV_H__

typedef char* (*utf8_convert_function)(const unsigned char*);

#ifdef __cplusplus
extern "C" {
#endif

char* utf16le_to_utf8(const unsigned short* utf16le);
char* utf16be_to_utf8(const unsigned short* utf16be);
char* gbk_to_utf8(const unsigned char* gbk);
char* big5_to_utf8(const unsigned char* big5);
char* sjis_to_utf8(const unsigned char* sjis);
char* euc_kr_to_utf8(const unsigned char* euc_kr);
char* ms_ee_to_utf8(const unsigned char* ms_ee);

#ifdef __cplusplus
}
#endif

#endif
