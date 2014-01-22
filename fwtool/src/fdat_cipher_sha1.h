//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher_sha1.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FDAT_CIPHER_SHA1_H
#define FDAT_CIPHER_SHA1_H

#include "sha1.h"

#ifndef SHA1_DIGEST_LEN
	#define SHA1_DIGEST_LEN		20
#endif

#define	FDC_SHA1_BLOCKLEN	1000

typedef struct tagFDC_SHA1 {
	int	valid_ctx;
	sha1_context	sha1_ctx;
	unsigned char	key[SHA1_DIGEST_LEN*2];
	int	is_encrypt;
} FDC_SHA1;

extern	int	fdc_sha1_init(FDC_SHA1 *p_ctx, const unsigned char *p_key, int keylen, int is_encrypt);
extern	int	fdc_sha1_reinit(FDC_SHA1 *p_ctx);
extern	int	fdc_sha1_cipher_bytes(FDC_SHA1 *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes);
extern	int	fdc_sha1_cipher_blocks(FDC_SHA1 *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks);

#endif // FDAT_CIPHER_SHA1_H

