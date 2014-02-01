//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_cipher_sha1.c" is part of fwtool (http://www.nex-hack.info)
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

#include "fdat_cipher.h"	// this includes "fdat_cipher_sha1.h"




int
fdc_sha1_init(FDC_SHA1 *p_ctx, const unsigned char *p_key, int keylen, int is_encrypt)
{
	memset((void *)p_ctx, 0, sizeof(*p_ctx));

	if (!p_key || (keylen != sizeof(p_ctx->key))) {
		return 1;
	}
	memcpy((void *)p_ctx->key, p_key, keylen);

	p_ctx->is_encrypt = is_encrypt;

	return fdc_sha1_reinit(p_ctx);
}

int
fdc_sha1_reinit(FDC_SHA1 *p_ctx)
{
	sha1_starts(&p_ctx->sha1_ctx);
	sha1_update(&p_ctx->sha1_ctx, &p_ctx->key[0], SHA1_DIGEST_LEN);
	sha1_update(&p_ctx->sha1_ctx, &p_ctx->key[SHA1_DIGEST_LEN], SHA1_DIGEST_LEN);

	p_ctx->valid_ctx = 1;

	return 0;
}

int
fdc_sha1_cipher_bytes(FDC_SHA1 *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nbytes)
{
	unsigned char	sha_digest[SHA1_DIGEST_LEN];
	int	this_nbytes;
	int	i;

	while(nbytes > 0) {
		if (nbytes < sizeof(sha_digest)) {
			this_nbytes = nbytes;
		} else {
			this_nbytes = sizeof(sha_digest);
		}

		// get the latest sha-1 digest (result used to xor data buffer)
		sha1_finish(&p_ctx->sha1_ctx, sha_digest);

		// use the newly calculated digest, along with "Decrypt_sha1_seed_2"
		// to start a new sha-1 calculation (resulting digest will be used
		// next time through this loop)
		sha1_starts(&p_ctx->sha1_ctx);
		sha1_update(&p_ctx->sha1_ctx, sha_digest, sizeof(sha_digest));
		sha1_update(&p_ctx->sha1_ctx, &p_ctx->key[SHA1_DIGEST_LEN], SHA1_DIGEST_LEN);

		// now take the sha-1 digest (from above) and xor that into
		// the next 20 bytes of the data buffer
		for (i = 0; i < this_nbytes; i++) {
			*p_outbuf++ = (*p_inbuf++ ^ sha_digest[i]);
		}
		nbytes -= this_nbytes;
	}

	return 0;
}

int
fdc_sha1_cipher_blocks(FDC_SHA1 *p_ctx, unsigned char *p_outbuf, unsigned char *p_inbuf, int nblocks)
{
	return fdc_sha1_cipher_bytes(p_ctx, p_outbuf, p_inbuf, (nblocks * FDC_SHA1_BLOCKLEN));
}

