// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
// copy from u-boot-2022.07/include/u-boot/md5.h

/*
 * This file was transplanted with slight modifications from Linux sources
 * (fs/cifs/md5.h) into U-Boot by Bartlomiej Sieka <tur@semihalf.com>.
 */

#ifndef __ZC_MD5_H__
#define __ZC_MD5_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MD5_SUM_LEN	16

struct ZC_MD5Context {
	unsigned int buf[4];
	unsigned int  bits[2];
	union {
		unsigned char in[64];
		unsigned int  in32[16];
	};
};

void ZC_MD5Init(struct ZC_MD5Context *ctx);
void ZC_MD5Update(struct ZC_MD5Context *ctx, unsigned char const *buf, unsigned len);
void ZC_MD5Final(unsigned char digest[16], struct ZC_MD5Context *ctx);

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at
 * 'input'. 'output' must have enough space to hold 16 bytes.
 */
void zc_md5(unsigned char *input, int len, unsigned char output[16]);

/*
 * Calculate and store in 'output' the MD5 digest of 'len' bytes at 'input'.
 * 'output' must have enough space to hold 16 bytes. If 'chunk' Trigger the
 * watchdog every 'chunk_sz' bytes of input processed.
 */
void zc_md5_wd(const unsigned char *input, unsigned int len,
	     unsigned char output[16], unsigned int chunk_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __ZC_MD5_H__ */
