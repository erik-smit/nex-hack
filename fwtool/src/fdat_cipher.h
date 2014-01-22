//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FDAT_CIPHER_H
#define FDAT_CIPHER_H

#include "fdat_cipher_sha1.h"
#include "fdat_cipher_aes.h"


typedef enum tagFDC_METHOD {
	FDCM_UNKNOWN = 0,
	FDCM_SHA1,
	FDCM_AES,

	NUM_FDC_METHOD
} FDC_METHOD;

typedef struct tagFDC {
	FDC_METHOD	fdc_method;
	size_t	fdc_block_len;

	FDC_SHA1	fdc_sha1_ctx;
	FDC_AES		fdc_aes_ctx;
} FDC;


extern	int	fdc_init(FDC *p_ctx, int is_encrypt, FDC_METHOD fdc_method);
extern	int	fdc_reinit(FDC *p_ctx);
extern	int	fdc_set_crypto_method(FDC *p_ctx, FDC_METHOD fdc_method, int re_init);
extern	FDC_METHOD	fdc_crypto_method(FDC *p_ctx);
extern	size_t	fdc_block_len(FDC *p_ctx);
extern	size_t	fdc_max_block_len(FDC *p_ctx);
extern	int	fdc_cipher_bytes(FDC *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes);
extern	int	fdc_cipher_blocks(FDC *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks);

#endif // FDAT_CIPHER_H
