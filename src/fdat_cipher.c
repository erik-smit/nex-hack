//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher.c" is part of fwtool (http://www.nex-hack.info)
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

#include "fdat_cipher.h"


static const unsigned char	fdc_sha1_default_key[] = {
	0x8D, 0xE5, 0xA8, 0x56, 0xD2, 0xEE, 0x76, 0xE0, 
	0x6C, 0x45, 0xDD, 0x9F, 0x57, 0x12, 0xC6, 0x3A, 
	0x0A, 0xDB, 0x05, 0xC1, 0xAF, 0x80, 0x8F, 0xC3, 
	0x97, 0x7B, 0x21, 0x87, 0x75, 0x22, 0x69, 0xDE, 
	0x83, 0xCC, 0xA6, 0xC6, 0x12, 0xF0, 0xDC, 0x49, 
};

static const unsigned char	fdc_aes_default_key[] = {
	0xE3, 0xB0, 0xC4, 0x42, 0x98, 0xFC, 0x1C, 0x14, 
	0x9A, 0xFB, 0xF4, 0xC8, 0x99, 0x6F, 0xB9, 0x24, 
};


int
fdc_init(FDC *p_ctx, int is_encrypt, FDC_METHOD fdc_method)
{
	memset((void *)p_ctx, 0, sizeof(*p_ctx));

	fdc_sha1_init(&p_ctx->fdc_sha1_ctx, fdc_sha1_default_key, sizeof(fdc_sha1_default_key), is_encrypt);
	fdc_aes_init(&p_ctx->fdc_aes_ctx, fdc_aes_default_key, sizeof(fdc_aes_default_key), is_encrypt);

	p_ctx->fdc_method = fdc_method;

	return 0;
}


int
fdc_reinit(FDC *p_ctx)
{
	fdc_sha1_reinit(&p_ctx->fdc_sha1_ctx);
	fdc_aes_reinit(&p_ctx->fdc_aes_ctx);

	return 0;
}


int
fdc_set_crypto_method(FDC *p_ctx, FDC_METHOD fdc_method, int re_init)
{
	p_ctx->fdc_method = fdc_method;

	if (re_init) {
		return fdc_reinit(p_ctx);
	} else {
		return 0;
	}
}


FDC_METHOD
fdc_crypto_method(FDC *p_ctx)
{
	return p_ctx->fdc_method;
}


static size_t
_fdc_block_len_for_method(FDC_METHOD fdc_method)
{
	switch(fdc_method) {
	case FDCM_UNKNOWN:
	default:
		return 0;
	case FDCM_SHA1:
		return FDC_SHA1_BLOCKLEN;
	case FDCM_AES:
		return FDC_AES_BLOCKLEN;
	}
}


size_t
fdc_block_len(FDC *p_ctx)
{
	return _fdc_block_len_for_method(p_ctx->fdc_method);
}


size_t
fdc_max_block_len(FDC *p_ctx)
{
	size_t	maxlen = 0;
	FDC_METHOD	i;
	size_t	this_len;

	for (i = 0; i < NUM_FDC_METHOD; i++) {
		this_len = _fdc_block_len_for_method(i);
		if (this_len > maxlen) maxlen = this_len;
	}

	return maxlen;
}


int
fdc_cipher_bytes(FDC *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes)
{
	switch(p_ctx->fdc_method) {
	case FDCM_UNKNOWN:
	default:
		return 1;
	case FDCM_SHA1:
		return fdc_sha1_cipher_bytes(&p_ctx->fdc_sha1_ctx, p_outbuf, p_inbuf, nbytes);
	case FDCM_AES:
		return fdc_aes_cipher_bytes(&p_ctx->fdc_aes_ctx, p_outbuf, p_inbuf, nbytes);
	}
}


int
fdc_cipher_blocks(FDC *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks)
{
	switch(p_ctx->fdc_method) {
	case FDCM_UNKNOWN:
	default:
		return 1;
	case FDCM_SHA1:
		return fdc_sha1_cipher_blocks(&p_ctx->fdc_sha1_ctx, p_outbuf, p_inbuf, nblocks);
	case FDCM_AES:
		return fdc_aes_cipher_blocks(&p_ctx->fdc_aes_ctx, p_outbuf, p_inbuf, nblocks);
	}
}
