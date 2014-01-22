//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_crypt.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FDAT_CRYPT_H
#define FDAT_CRYPT_H

#ifndef FDAT_IMAGE_H
	#include "fdat_image.h"	// for FDAT_IMAGE_MAGIC
#endif

#ifndef FDAT_CIPHER_H
	#include "fdat_cipher.h"	// for FDC_METHOD typedef
#endif

// plaintext data in block0 used to guess crypto method
#define	FDAT_IMAGE_BLOCK0_IDENT_MAGIC1		FDAT_IMAGE_MAGIC
#define	FDAT_IMAGE_BLOCK0_IDENT_MAGIC1_LEN	FDAT_IMAGE_MAGIC_LEN
#define	FDAT_IMAGE_BLOCK0_IDENT_MAGIC1_OFFSET	(sizeof(FDAT_ENC_BLOCK_HDR)+FDAT_IMAGE_MAGIC_OFS)


extern	int	fdat_decrypt_buffer(unsigned char *p_fdat_encrypted, size_t sz_fdat_encrypted, 
			unsigned char **pp_fdat_decrypted, size_t *psz_fdat_decrypted, FDC_METHOD *p_fdc_method);

extern	int	fdat_decrypt_file(const char *fdat_in_fname, const char *fdat_out_fname, FDC_METHOD *p_fdc_method);
extern	int	fdat_encrypt_file(const char *fdat_in_fname, const char *fdat_out_fname, const char *fdat_check_fname, FDC_METHOD *p_fdc_method);	// added by kenan


#endif // FDAT_CRYPT_H
