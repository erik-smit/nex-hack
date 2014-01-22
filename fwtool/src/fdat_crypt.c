//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_crypt.c" is part of fwtool (http://www.nex-hack.info)
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
#include "fwt_util.h"

#include "endian.h"
#include "csum.h"
#include "fdat_cipher.h"

#include "fdat_crypt.h"


FDC_METHOD
fdat_image_guess_crypto_method(FDC *p_ctx, unsigned char *p_databuf, int nbytes)
{
	FDC_METHOD	fdcm;
	FDC	tmp_ctx;
	unsigned char	*p_blockbuf = NULL;
	int	block_len;

	p_ctx->fdc_method = FDCM_UNKNOWN;

	for (fdcm = 0; fdcm < NUM_FDC_METHOD; fdcm++) {
		if (fdcm == FDCM_UNKNOWN) continue;
		tmp_ctx = *p_ctx;
		tmp_ctx.fdc_method = fdcm;
		block_len = fdc_block_len(&tmp_ctx);
		if (block_len <= nbytes) {
			if (!(p_blockbuf = malloc(block_len))) {
				fprintf(stderr, "fdat_image_guess_crypto_method(): no memory for block buffer!\n");
				goto exit_err;
			}
			memcpy(p_blockbuf, p_databuf, block_len);
			if (fdc_cipher_blocks(&tmp_ctx, p_blockbuf, p_blockbuf, 1) == 0) {
				if (!memcmp(p_blockbuf+FDAT_IMAGE_BLOCK0_IDENT_MAGIC1_OFFSET,
					FDAT_IMAGE_BLOCK0_IDENT_MAGIC1, FDAT_IMAGE_BLOCK0_IDENT_MAGIC1_LEN)) {
					p_ctx->fdc_method = fdcm;
					goto exit_ok;
				}
			}
			free((void *)p_blockbuf); p_blockbuf = NULL;
		}
	}
	// fallthru to exit_err
exit_err:
	goto exit_common;

exit_ok:
	// fallthru to exit_common

exit_common:
	if (p_blockbuf) {
		free(p_blockbuf);
	}
	return p_ctx->fdc_method;
}


int
fdat_decrypt_buffer(unsigned char *p_fdat_encrypted, size_t sz_fdat_encrypted, unsigned char **pp_fdat_decrypted, size_t *psz_fdat_decrypted, FDC_METHOD *p_fdc_method)
{
	FDC	fdc;
	FDC_METHOD	fdc_method;
	size_t	block_len, block_len_decrypted;
	int	nblocks;
	unsigned char	*p_fdat_decrypted = NULL;
	size_t	sz_fdat_decrypted;
	int	retval;
	unsigned char	*p_crypt_io, *p_dec_out;
	int	block;
	FDAT_ENC_BLOCK_HDR	*p_blk_hdr;
	unsigned short	block_hdr_csum, block_hdr_len_and_flags;
	unsigned short	calc_csum;
	int	last_block;
	size_t	sz_decrypt_actual;
	size_t	this_len_decrypted;


	*pp_fdat_decrypted = NULL;
	*psz_fdat_decrypted = 0;
	if (p_fdc_method) *p_fdc_method = FDCM_UNKNOWN;

	//
	// the FDAT region is made up of one or more 1000-byte chunks
	// of data.
	//
	// we'll read the data, 1000 bytes at a time, then decrypt
	// the 1000 bytes.
	//
	// the decrypted data will then look like:
	//
	//	- header value: 16-bit checksum (stored little-endian) of all remaining 16-bit values following
	//	- header value: 16-bit 'actual length' (stored little-endian) of the data that follows
	//	  (if less than a full block)
	//	  if the high-bit of this value is set, then it's the last chunk of the file
	//	- up to 996 (1000-2-2) bytes of data ('actual length' may say less)
	//
	// we perform a checksum over all 16-bit values *except* for the first one (which
	// holds the expected checksum), then compare to the expected checksum.  since
	// a full chunk is 1000 bytes, then the number of 16-bit values added together
	// is: ((1000-2)/2) = 499 'words'.
	//
	// NOTE: we always calculate the checksum over the entire 499-word region
	// (even if the 'actual length' says the size is less) - this may or may not
	// be correct (could need tweaking later if a different firmware image
	// fails to checksum correctly)
	//
	// if the checksum matches, then we'll write 'actual length' bytes of data (starting
	// after the two 'header' values) to the output file.
	//
	fdc_init(&fdc, 0, FDCM_UNKNOWN);
	if ((fdc_method = fdat_image_guess_crypto_method(&fdc, p_fdat_encrypted, sz_fdat_encrypted)) == FDCM_UNKNOWN) {
		fprintf(stderr, "Could not determine FDAT crypto method!\n");
		goto exit_err;
	}
	fdc_set_crypto_method(&fdc, fdc_method, 1);

	block_len = fdc_block_len(&fdc);

	//
	// if the overall FDAT record length isn't an even multiple of block_len,
	// return an error
	//
	if (sz_fdat_encrypted % block_len) {
		fprintf(stderr,
			"decrypt_fdat_image(): FDAT record length (0x%x) not a multiple of decrypt block length (%u)!\n",
			sz_fdat_encrypted, block_len);
		goto exit_err;
	}
	nblocks = (sz_fdat_encrypted / block_len);

	block_len_decrypted = (block_len - sizeof(FDAT_ENC_BLOCK_HDR));
	sz_fdat_decrypted = (nblocks * block_len_decrypted);
	if (!(p_fdat_decrypted = malloc(sz_fdat_decrypted))) {
		goto exit_err;
	}
	memset((void *)p_fdat_decrypted, 0, sz_fdat_decrypted);

	// we decrypt blocks (with headers) in-place
	p_crypt_io = p_fdat_encrypted;
	if (fdc_cipher_blocks(&fdc, p_crypt_io, p_crypt_io, nblocks)) {
		fprintf(stderr, "fdc_cipher_blocks() returned error!\n");
		goto exit_err;
	}

	p_dec_out = p_fdat_decrypted;
	sz_decrypt_actual = 0;
	for (block = 0; block < nblocks; block++) {
		p_blk_hdr = (FDAT_ENC_BLOCK_HDR *)p_crypt_io;

		// ((block_hdr[3] << 8) | block_hdr[2]) is the # of bytes to write
		// high bit of block_hdr[3] indicates last block?
		block_hdr_csum = readLE16((u8 *)&p_blk_hdr->hdr_csum);

		block_hdr_len_and_flags = readLE16((u8 *)&p_blk_hdr->hdr_len_and_flags);
		last_block = ((block_hdr_len_and_flags & 0x8000) != 0);

		this_len_decrypted = (block_hdr_len_and_flags & 0xfff);
		if (this_len_decrypted > block_len_decrypted) {
			fprintf(stderr,
				"decrypt_fdat_image(blocknum=%u): Bad block write length (%u)!\n",
				block, this_len_decrypted);
			goto exit_err;
		}
// fprintf(plog_global, "block: %04x, hdr_csum: %04x, len_and_flags: %04x\n", block, block_hdr_csum, block_hdr_len_and_flags); log_it(plog_global);
		// calculate checksum, over everything but the first word of the
		// block header (which is the checksum).  confirm it matches the
		// value stored in the header
		calc_csum = calc_csum_16bitLE_words((u16 *)(&p_crypt_io[2]), ((block_len-2)>>1));

		if (calc_csum != block_hdr_csum) {
			fprintf(stderr,
				"decrypt_fdat_image(blocknum=%u): block checksum mismatch (expected: %04x, calculated: %04x)!\n",
				block, block_hdr_csum, calc_csum);
			goto exit_err;
		}

		memcpy((void *)p_dec_out, (void *)(&p_blk_hdr[1]), this_len_decrypted);

		p_crypt_io += block_len;
		p_dec_out += this_len_decrypted;
		sz_decrypt_actual += this_len_decrypted;
		if (last_block) {
			break;
		}
	}
	goto exit_ok;

exit_err:
	if (p_fdat_decrypted) {
		free((void *)p_fdat_decrypted);
		p_fdat_decrypted = NULL;
	}
	retval = 1;
	goto exit_common;

exit_ok:
	*pp_fdat_decrypted = p_fdat_decrypted;
	*psz_fdat_decrypted = sz_decrypt_actual;
	if (p_fdc_method) *p_fdc_method = fdc_method;
	retval = 0;

exit_common:
	return retval;
}


int
fdat_decrypt_file(const char *fdat_in_fname, const char *fdat_out_fname, FDC_METHOD *p_fdc_method)
{
	int	retval;
	FDC	fdc;
	FDC_METHOD	fdc_method;
	FDAT_ENC_BLOCK_HDR	*p_blk_hdr;
	FILE	*fh_in = NULL, *fh_out = NULL;
	size_t	max_block_len, block_len, filesize, block_len_decrypted, this_len_decrypted;
	int	nblocks, block;
	unsigned char	*p_block_buf = NULL;
	unsigned short	block_hdr_csum, block_hdr_len_and_flags, calc_csum;
	// int	last_block;

	if (p_fdc_method) *p_fdc_method = FDCM_UNKNOWN;

	sprintf(plog_global, "Reading FDAT input file: '%s'\n", fdat_in_fname); log_it(plog_global);

	if (!(fh_in = fopen(fdat_in_fname, "rb"))) {
		fprintf(stderr, "decrypt_fdat_file(): Error opening input file '%s'!\n", fdat_in_fname);
		goto exit_err;
	}

	fdc_init(&fdc, 0, FDCM_UNKNOWN);

	// first, we read a block of data (of the max possible size for a crypto block), then
	// 'guess' the  crypto method using that buffer. we go back and decrypt the file.
	max_block_len = fdc_max_block_len(&fdc);
	if (!(p_block_buf = malloc(max_block_len))) {
		fprintf(stderr, "decrypt_fdat_file(): Failed to allocate block buffer!\n");
		goto exit_err;
	}

	// get total input file size
	fseek(fh_in, 0L, SEEK_END);
	filesize = ftell(fh_in);
	fseek(fh_in, 0L, SEEK_SET);

	if (max_block_len > filesize) max_block_len = filesize;

	if (fread(p_block_buf, 1, max_block_len, fh_in) != max_block_len) {
		fprintf(stderr, "decrypt_fdat_file(): Failed to read initial block!\n");
		goto exit_err;
	}

	if ((fdc_method = fdat_image_guess_crypto_method(&fdc, p_block_buf, max_block_len)) == FDCM_UNKNOWN) {
		fprintf(stderr, "decrypt_fdat_file(): Could not determine FDAT crypto method!\n");
		goto exit_err;
	}
	fdc_set_crypto_method(&fdc, fdc_method, 1);

	// go back to the start of the file
	fseek(fh_in, 0L, SEEK_SET);

	block_len = fdc_block_len(&fdc);
	block_len_decrypted = (block_len - sizeof(FDAT_ENC_BLOCK_HDR));

	sprintf(plog_global, "Using cipher method = %d, block length = %d\n", fdc_method, block_len); log_it(plog_global);

	// if the overall FDAT record length isn't an even multiple of block_len, return an error
	if (filesize % block_len) {
		fprintf(stderr,
			"decrypt_fdat_file(): FDAT file length (0x%x) not a multiple of decrypt block length (%u)!\n",
			filesize, block_len);
		goto exit_err;
	}

	sprintf(plog_global, "Writing FDAT output to '%s' ... ", fdat_out_fname); log_it(plog_global);

	if (!(fh_out = fopen(fdat_out_fname, "wb"))) {
		fprintf(stderr, "decrypt_fdat_file(): Error creating output file '%s'!\n", fdat_out_fname);
		goto exit_err;
	}

	nblocks = (filesize / block_len);
	for (block = 0; block < nblocks; block++) {
		p_blk_hdr = (FDAT_ENC_BLOCK_HDR *)p_block_buf;

		if (fread(p_block_buf, 1, block_len, fh_in) != block_len) {
			fprintf(stderr, "decrypt_fdat_file(): Failed to read input block #%d!\n", block);
			goto exit_err;
		}
//debug
//if(block > nblocks-3) {for(int j=0;j<1024;j=j+32) {sprintf(plog_global, "\n%04x:", j);log_it(plog_global);for(int k=0;k<32;k++) {sprintf(plog_global, " %02x",p_block_buf[j+k]);log_it(plog_global);}}}
//debug

		// we decrypt blocks (with headers) in-place
		if (fdc_cipher_blocks(&fdc, p_block_buf, p_block_buf, 1)) {
			fprintf(stderr, "fdc_cipher_blocks() returned error!\n");
			goto exit_err;
		}
//debug
//if(block > nblocks-3) {for(int j=0;j<1024;j=j+32) {sprintf(plog_global, "\n%04x:", j);log_it(plog_global);for(int k=0;k<32;k++) {sprintf(plog_global, " %02x",p_block_buf[j+k]);log_it(plog_global);}}}
//debug

		// ((block_hdr[3] << 8) | block_hdr[2]) is the # of bytes to write
		// high bit of block_hdr[3] indicates last block?
		block_hdr_csum = readLE16((u8 *)&p_blk_hdr->hdr_csum);

		block_hdr_len_and_flags = readLE16((u8 *)&p_blk_hdr->hdr_len_and_flags);
		// last_block = ((block_hdr_len_and_flags & 0x8000) != 0);

		this_len_decrypted = (block_hdr_len_and_flags & 0xfff);
		if (this_len_decrypted > block_len_decrypted) {
			fprintf(stderr,
				"decrypt_fdat_image(blocknum=%u): Bad block write length (%u)!\n",
				block, this_len_decrypted);
			goto exit_err;
		}
//debug
//sprintf(plog_global, "\nblock: %04x, hdr_csum: %04x, len_and_flags: %04x\n", block, block_hdr_csum, block_hdr_len_and_flags); log_it(plog_global);
//debug
		// calculate checksum, over everything but the first word of the
		// block header (which is the checksum).  confirm it matches the
		// value stored in the header
		calc_csum = calc_csum_16bitLE_words((u16 *)(&p_block_buf[2]), ((block_len-2)>>1));

		if (calc_csum != block_hdr_csum) {
			fprintf(stderr,
				"decrypt_fdat_image(blocknum=%u): block checksum mismatch (expected: %04x, calculated: %04x)!\n",
				block, block_hdr_csum, calc_csum);
			goto exit_err;
		}

		if (fwrite((void *)(&p_blk_hdr[1]), 1, this_len_decrypted, fh_out) != this_len_decrypted) {
			fprintf(stderr, "decrypt_fdat_file(): Failed to write output block #%d!\n", block);
			goto exit_err;
		}
	}
	fclose(fh_in);
	fclose(fh_out);
	goto exit_ok;

exit_err:
	if (fh_in) fclose(fh_in);
	if (fh_out) fclose(fh_out);
	unlink(fdat_out_fname);
	retval = 1;
	goto exit_common;

exit_ok:
	if (p_fdc_method) *p_fdc_method = fdc_method;
	retval = 0;
	sprintf(plog_global,"Done\n\n"); log_it(plog_global);
	// fallthru

exit_common:
	if (p_block_buf) {
		free((void *)p_block_buf);
	}
	return retval;
}


// added by kenan based on oz_paulb fdat_decrypt_file()
int
fdat_encrypt_file(const char *fdat_in_fname, const char *fdat_out_fname, const char *fdat_check_fname, FDC_METHOD *p_fdc_method)
{
	int	retval=0;
	FDC	fdc;
	FDC_METHOD	fdc_method = FDCM_UNKNOWN;
	// FDAT_ENC_BLOCK_HDR	*p_blk_hdr;
	FILE	*fh_check = NULL, *fh_in = NULL, *fh_out = NULL;
	size_t	max_block_len=0, block_len=0, filesize=0, block_len_decrypted=0, this_len_decrypted=0, this_len_encrypted=0;
	int nblocks=0, block=0, last_block=0, pad=0;
	unsigned char	*p_block_buf = NULL;
	unsigned short	calc_csum=0;
	u16 highb=0;

	if (p_fdc_method) *p_fdc_method = FDCM_UNKNOWN;

	// autotdetect fdc_method for encrypt
	// we read a block of data from encrypted FDAT.bin to 'guess' the crypto method using that buffer.
	// TODO find better method using content of FDAT.dec file
	if (!(fh_check = fopen(fdat_check_fname, "rb"))) {
		fprintf(stderr, "decrypt_fdat_file(): Error opening FDAT bin file '%s'!\n", fdat_check_fname);
		goto exit_err;
	}
	fdc_init(&fdc, 0, FDCM_UNKNOWN);
	max_block_len = fdc_max_block_len(&fdc);
	if (!(p_block_buf = malloc(max_block_len))) {
		fprintf(stderr, "encrypt_fdat_file(): Failed to allocate block buffer for guess!\n");
		goto exit_err;
	}
	fseek(fh_check, 0L, SEEK_END);
	filesize = ftell(fh_check);
	fseek(fh_check, 0L, SEEK_SET);
	if (max_block_len > filesize) max_block_len = filesize;
	if (fread(p_block_buf, 1, max_block_len, fh_check) != max_block_len) {
		fprintf(stderr, "encrypt_fdat_file(): Failed to read initial block for guess!\n");
		goto exit_err;
	}
	if ((fdc_method = fdat_image_guess_crypto_method(&fdc, p_block_buf, max_block_len)) == FDCM_UNKNOWN) {
		fprintf(stderr, "encrypt_fdat_file(): Could not determine FDAT bin crypto method!\n");
		goto exit_err;
	}
	block_len = fdc_block_len(&fdc);	// doubled later, here for output only
	sprintf(plog_global, "cipher method for encrypt = %d, block length = %d\n", fdc_method, block_len); log_it(plog_global);
	fclose(fh_check);
	free((void *)p_block_buf);
	fdc_init(&fdc, 1, fdc_method);

	//
	// begin encrypt *.dec file with fdc_method determined before
	//
	sprintf(plog_global, "Reading FDAT dec input file: '%s' ...\n", fdat_in_fname); log_it(plog_global);

	if (!(fh_in = fopen(fdat_in_fname, "rb"))) {
		fprintf(stderr, "encrypt_fdat_file(): Error opening input file '%s'!\n", fdat_in_fname);
		goto exit_err;
	}

	max_block_len = fdc_max_block_len(&fdc);
	if (!(p_block_buf = malloc(max_block_len))) {
		fprintf(stderr, "encrypt_fdat_file(): Failed to allocate block buffer!\n");
		goto exit_err;
	}

	// get total input file size
	fseek(fh_in, 0L, SEEK_END);
	filesize = ftell(fh_in);
	fseek(fh_in, 0L, SEEK_SET);

	if (max_block_len > filesize) max_block_len = filesize;

	block_len = fdc_block_len(&fdc);	// 1.gen SHA1=1000, 2.gen AES=1024
	block_len_decrypted = (block_len - sizeof(FDAT_ENC_BLOCK_HDR));	// 1.gen SHA1=996, 2.gen AES=1020
	//sprintf(plog_global, "cipher method = %d, block length = %d\n", fdc_method, block_len); log_it(plog_global);

	sprintf(plog_global, "Writing FDAT enc output to '%s' ... ", fdat_out_fname); log_it(plog_global);
	if (!(fh_out = fopen(fdat_out_fname, "wb"))) {
		fprintf(stderr, "encrypt_fdat_file(): Error creating output file '%s'!\n", fdat_out_fname);
		goto exit_err;
	}

	// if the overall FDAT dec record length isn't an even multiple of block_len_decrypted,
	// then the last block has to be padded with 0xff to form a full enc block. (kenan)
	//
	last_block = filesize % block_len_decrypted;
	nblocks = (filesize / block_len_decrypted);
	if (last_block) nblocks++;

	for (block = 0; block < nblocks; block++) {
		// header clear (UGLY!)
		p_block_buf[0] = '\0';p_block_buf[1] = '\0';p_block_buf[2] = '\0';p_block_buf[3] = '\0';
		// needed?
		// p_blk_hdr = (FDAT_ENC_BLOCK_HDR *)p_block_buf;

		if (block == nblocks-1) {
			if ((this_len_decrypted = fread(p_block_buf+sizeof(FDAT_ENC_BLOCK_HDR), 1, block_len_decrypted, fh_in)) != last_block) {
				fprintf(stderr, "encrypt_fdat_file(): Failed to read last input block #%d!\n", block);
				goto exit_err;
			}
		}
		else {
			if ((this_len_decrypted = fread(p_block_buf+sizeof(FDAT_ENC_BLOCK_HDR), 1, block_len_decrypted, fh_in)) != block_len_decrypted) {
				fprintf(stderr, "encrypt_fdat_file(): Failed to read input block #%d!\n", block);
				goto exit_err;
			}
		}

//debug
//if(block > nblocks-3) {for(int j=0;j<1024;j=j+32) {sprintf(plog_global, "\n%04x:", j);log_it(plog_global);for(int k=0;k<32;k++) {sprintf(plog_global, " %02x",p_block_buf[j+k]);log_it(plog_global);}}}
//debug

		// calculate encrypted block length
		this_len_encrypted = this_len_decrypted + sizeof(FDAT_ENC_BLOCK_HDR);

//debug
//sprintf(plog_global, "\nthis_len_enc %04x, block_len %04x\n",this_len_encrypted, block_len); log_it(plog_global);
//debug

		if (this_len_encrypted > block_len) {
			fprintf(stderr,
				"encrypt_fdat_image(blocknum=%u): Bad block write length (%u)!\n",
				block, this_len_decrypted);
			goto exit_err;
		}
		// store len in buffer LE16 (UGLY!)
		highb = this_len_decrypted >> 8;
		if (block == nblocks-1) {
			highb = (highb | 0x80);	// last block has high bit set as flag
			for (pad=last_block+sizeof(FDAT_ENC_BLOCK_HDR);pad<block_len;pad++) p_block_buf[pad] = 0xff;	// pad last block to block len
		}
		p_block_buf[2] = (u8) this_len_decrypted;
		p_block_buf[3] = (u8) highb;
		// calculate checksum, over everything but the first word of the
		// block header (which is the checksum).  write into buffer.
		calc_csum = calc_csum_16bitLE_words((u16 *)(&p_block_buf[2]), ((block_len-2)>>1));
		highb = calc_csum >> 8;
		p_block_buf[0] = (u8) calc_csum;
		p_block_buf[1] = (u8) highb;

//debug
//if(block > nblocks-3) {for(int j=0;j<1024;j=j+32) {sprintf(plog_global, "\n%04x:", j);log_it(plog_global);for(int k=0;k<32;k++) {sprintf(plog_global, " %02x",p_block_buf[j+k]);log_it(plog_global);}}}
//if(block > nblocks-3) {sprintf(plog_global, "\nblock: %04x, hdr_csum: %04x, len_and_flags: %04x\n", block, calc_csum, this_len_decrypted); log_it(plog_global);}
//debug

		// we encrypt blocks (with headers) in-place
		if (fdc_cipher_blocks(&fdc, p_block_buf, p_block_buf, 1)) {
			fprintf(stderr, "fdc_cipher_blocks() returned error!\n");
			goto exit_err;
		}

//debug
//if(block > nblocks-3) {for(int j=0;j<1024;j=j+32) {sprintf(plog_global, "\n%04x:", j);log_it(plog_global);for(int k=0;k<32;k++) {sprintf(plog_global, " %02x",p_block_buf[j+k]);log_it(plog_global);}}}
//debug

		// write complete block_len, last block padded
		if (fwrite(p_block_buf, 1, block_len, fh_out) != block_len) {
			fprintf(stderr, "encrypt_fdat_file(): Failed to write output block #%d!\n", block);
			goto exit_err;
		}
	}
	fclose(fh_in);
	fclose(fh_out);
	goto exit_ok;

exit_err:
	if (fh_check) fclose(fh_check);
	if (fh_in) fclose(fh_in);
	if (fh_out) fclose(fh_out);
	unlink(fdat_out_fname);
	retval = 1;
	goto exit_common;

exit_ok:
	if (p_fdc_method) *p_fdc_method = fdc_method;
	retval = 0;
	sprintf(plog_global, "Done\n"); log_it(plog_global);
	// fallthru

exit_common:
	if (p_block_buf) {
		free((void *)p_block_buf);
	}
	return retval;
}
