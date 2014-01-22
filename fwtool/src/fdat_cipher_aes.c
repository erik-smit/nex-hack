//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher_aes.c" is part of fwtool (http://www.nex-hack.info)
//
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "config.h"

#include "fdat_cipher.h"	// this includes "fdat_cipher_aes.h"

#ifndef	AES_BLOCK_LEN
#define	AES_BLOCK_LEN	16
#endif


int
fdc_aes_init(FDC_AES *p_ctx, const unsigned char *p_key, int keylen, int is_encrypt)
{
	memset((void *)p_ctx, 0, sizeof(*p_ctx));

	if (!p_key || (keylen < 0) || (keylen > FDC_AES_MAX_KEYLEN)) {
		return 1;
	}
	memcpy((void *)p_ctx->key, p_key, keylen);
	p_ctx->keylen = keylen;

	p_ctx->is_encrypt = is_encrypt;

	return fdc_aes_reinit(p_ctx);
}


int
fdc_aes_reinit(FDC_AES *p_ctx)
{
	int	ret;

	if (p_ctx->is_encrypt) {
		ret = aes_setkey_enc(&p_ctx->aes_ctx, p_ctx->key, (p_ctx->keylen*8));
	} else {
		ret = aes_setkey_dec(&p_ctx->aes_ctx, p_ctx->key, (p_ctx->keylen*8));
	}
	if (ret) {
		return ret;
	}
	p_ctx->valid_ctx = 1;

	return 0;
}


int
fdc_aes_cipher_bytes(FDC_AES *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes)
{
	unsigned char	l_aes_buf_in[AES_BLOCK_LEN];
	unsigned char	l_aes_buf_out[AES_BLOCK_LEN];
	int	this_nbytes;
	unsigned char	*p_in, *p_out;
	int	ret;

	while(nbytes > 0) {
		if (nbytes < AES_BLOCK_LEN) {
			this_nbytes = nbytes;
			p_in = l_aes_buf_in;
			p_out = l_aes_buf_out;
		} else {
			this_nbytes = AES_BLOCK_LEN;
			p_in = p_inbuf;
			p_out = p_outbuf;
		}
		ret = aes_crypt_ecb(&p_ctx->aes_ctx, (p_ctx->is_encrypt ? AES_ENCRYPT : AES_DECRYPT), p_in, p_out);
		if (ret) {
			return ret;
		}

		if (this_nbytes < AES_BLOCK_LEN) {
			memcpy(p_outbuf, p_out, this_nbytes);
		}

		nbytes -= this_nbytes;
		p_inbuf += this_nbytes;
		p_outbuf += this_nbytes;
	}
	return 0;
}


int
fdc_aes_cipher_blocks(FDC_AES *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks)
{
	return fdc_aes_cipher_bytes(p_ctx, p_outbuf, p_inbuf, (nblocks * FDC_AES_BLOCKLEN));
}
