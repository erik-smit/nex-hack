//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher_aes.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FDAT_CIPHER_AES_H
#define FDAT_CIPHER_AES_H

#include "aes.h"

#define	FDC_AES_BLOCKLEN	1024

#define	FDC_AES_MAX_KEYLEN	(256/8)

typedef struct tagFDC_AES {
	int	valid_ctx;
	aes_context	aes_ctx;
	unsigned char	key[FDC_AES_MAX_KEYLEN];
	int	keylen;
	int	is_encrypt;
} FDC_AES;

extern	int	fdc_aes_init(FDC_AES *p_ctx, const unsigned char *p_key, int keylen, int is_encrypt);
extern	int	fdc_aes_reinit(FDC_AES *p_ctx);
extern	int	fdc_aes_cipher_bytes(FDC_AES *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes);
extern	int	fdc_aes_cipher_blocks(FDC_AES *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks);

#endif // FDAT_CIPHER_AES_H

